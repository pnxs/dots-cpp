#pragma once
#define DOTS_ACKNOWLEDGE_DEPRECATION_OF_DotsTransportHeader_destinationGroup
#define DOTS_ACKNOWLEDGE_DEPRECATION_OF_DotsTransportHeader_nameSpace
#define DOTS_ACKNOWLEDGE_DEPRECATION_OF_DotsTransportHeader_destinationClientId
#include <optional>
#include <boost/asio.hpp>
#include <dots/type/Registry.h>
#include <dots/io/Channel.h>
#include <dots/serialization/CborSerializer.h>
#include <dots/serialization/ExperimentalCborSerializer.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsTransportHeader.dots.h>

namespace dots::io
{
    /*!
     * @brief Specifies which format to use for serializing transmissions.
     *
     * When set to 'Legacy', transmissions will be serialized using a
     * DotsTransportHeader to be backwards compatible with legacy
     * applications.
     *
     * @remark This enum is intended to be used as an argument to
     * instantiate the dots::io::AsyncStreamChannel template.
     */
    enum struct TransmissionFormat : uint8_t
    {
        Default,
        Legacy
    };

    /*!
     * @class AsyncStreamChannel AsyncStreamChannel.h
     * <dots/AsyncStreamChannel.h>
     *
     * @brief Base class for asynchronous stream channels.
     *
     * This class can be used to implement a channel that is based on an
     * asynchronous Asio stream.
     *
     * The template can be instantiated with a specific stream type, which
     * will be used to implement the read and transmit functions of
     * dots::io::Channel.
     *
     * The resulting channel implementation will then use asynchronous
     * operations for both transmitting and receiving transmissions.
     *
     * To keep the number of system calls low, the channel features
     * buffering in both directions, which will be used to read and write
     * multiple payloads at once when the channel is under load.
     *
     * Additionally, the channel optionally can use a payload cache to
     * avoid redundant serializations when writing. This is intended to be
     * used by DOTS hosts, where the same transmission will be distributed
     * to an arbitrary number of subscribers.
     *
     * @tparam Stream The stream type to use. Must meet the requirements
     * for AsyncReadStream and AsyncWriteStream from the Asio library.
     *
     * @tparam Serializer The serializer to use.
     *
     * @tparam TransmissionFormat Specifies which format is used for
     * serializing transmissions. Note that this is not the serialization
     * encoding of instances, which is specified by the @p Serializer
     * argument.
     */
    template <typename Stream, typename Serializer = serialization::CborSerializer, TransmissionFormat TransmissionFormat = TransmissionFormat::Default>
    struct AsyncStreamChannel : Channel
    {
        using stream_t = Stream;
        using serializer_t = Serializer;

        using buffer_t = typename serializer_t::data_t;
        using payload_cache_t = std::optional<std::pair<Transmission::id_t, buffer_t>>;

        /*!
         * @brief Construct a new AsyncStreamChannel object.
         *
         * This will initialize the channel from a given Asio IO context.
         *
         * Note that the overload is only available if @p Stream is
         * constructible from boost::asio::io_context&.
         *
         * @attention Channels cannot be constructed manually and must always
         * be obtained via dots::io::make_channel().
         *
         * @tparam S Defaulted helper type parameter used for SFINAE. Do not
         * specify manually!
         *
         * @param key The key to construct the channel. This will automatically
         * be provided by the dots::io::make_channel() helper function.
         *
         * @param ioContext The IO context (i.e. event loop) to associate with
         * the channel.
         */
        template <typename S = Stream, std::enable_if_t<std::is_constructible_v<S, boost::asio::io_context&>, int> = 0>
        AsyncStreamChannel(Channel::key_t key, boost::asio::io_context& ioContext) :
            AsyncStreamChannel(key, stream_t{ ioContext }, nullptr)
        {
            /* do nothing */
        }

        /*!
         * @brief Construct a new AsyncStreamChannel object.
         *
         * This will initialize the channel from a stream object.
         *
         * @attention Channels cannot be constructed manually and must always
         * be obtained via dots::io::make_channel().
         *
         * @param key The key to construct the channel. This will automatically
         * be provided by the dots::io::make_channel() helper function.
         *
         * @param stream The stream object to use.
         *
         * @param payloadCache The payload cache to use. May be nullptr to
         * disable the payload cache (see
         * AsyncStreamChannel::serializeTransmission(Transmission::id_t, const
         * DotsHeader&, const type::Struct&)).
         */
        AsyncStreamChannel(Channel::key_t key, stream_t&& stream, payload_cache_t* payloadCache) :
            Channel(key),
            m_stream{ std::move(stream) },
            m_payloadCache(payloadCache),
            m_asyncWriting(false),
            m_readDispatching(false)
        {
            m_readBuffer.resize(ReadBufferMinSize);
        }

        AsyncStreamChannel(const AsyncStreamChannel& other) = delete;
        AsyncStreamChannel(AsyncStreamChannel&& other) = delete;
        ~AsyncStreamChannel() override = default;

        AsyncStreamChannel& operator = (const AsyncStreamChannel& rhs) = delete;
        AsyncStreamChannel& operator = (AsyncStreamChannel&& rhs) = delete;

    protected:

        /*!
         * @brief Get the underlying stream object.
         *
         * @return const stream_t& A reference to the underlying stream object.
         */
        const stream_t& stream() const
        {
            return m_stream;
        }

        /*!
         * @brief Get the underlying stream object.
         *
         * @return stream_t& A reference to the underlying stream object.
         */
        stream_t& stream()
        {
            return m_stream;
        }

        /*!
         * @brief Asynchronously receive transmissions from the channel.
         *
         * This will asynchronously read as many bytes as required from the
         * underlying stream to deserialize a transmission and call
         * dots::io::Channel::processReceive() with the result.
         *
         * When this function is called again after a transmission was
         * processed, the required amount of bytes for the next transmission
         * might already be contained in the read buffer (i.e. available as
         * input data).
         *
         * In that case, the subsequent processing of the next transmission
         * will be performed as a continuation of the call via
         * boost::asio::dispatch() and may occur synchronously before this
         * function returns.
         *
         * This process will be repeated until the read buffer has too little
         * data available, in which case an asynchronous read will again be
         * initiated on the underlying stream object.
         */
        void asyncReceiveImpl() override
        {
            if (m_readDispatching)
            {
                m_readDispatching = false;
                return;
            }

            if constexpr (TransmissionFormat == TransmissionFormat::Legacy)
            {
                auto process_transmission = [this]
                {
                    Transmission transmission = deserializeTransmission();
                    processReceive(std::move(transmission));
                };

                auto process_header = [this, process_transmission]
                {
                    if (size_t transmissionSize = deserializeTransmissionSize(); transmissionSize <= m_serializer.inputAvailable())
                    {
                        process_transmission();
                    }
                    else
                    {
                        asyncRead(transmissionSize, process_transmission);
                    }
                };

                auto process_header_size = [this, process_header]
                {
                    if (size_t headerSize = deserializeHeaderSize(); headerSize <= m_serializer.inputAvailable())
                    {
                        process_header();
                    }
                    else
                    {
                        asyncRead(headerSize, [this, process_header]
                        {
                            process_header();
                        });
                    }
                };

                asyncRead(TransmissionSizeSize, process_header_size);
            }
            else
            {
                auto process_transmission = [this]
                {
                    Transmission transmission = deserializeTransmission();
                    processReceive(std::move(transmission));
                };

                auto process_transmission_size = [this, process_transmission]
                {
                    if (size_t transmissionSize = deserializeTransmissionSize(); transmissionSize <= m_serializer.inputAvailable())
                    {
                        process_transmission();
                    }
                    else
                    {
                        asyncRead(transmissionSize, process_transmission);
                    }
                };

                asyncRead(TransmissionSizeSize, process_transmission_size);
            }

        }

        /*!
         * @brief Asynchronously transmit a transmission through the channel.
         *
         * This will serialize a given decomposed transmission and
         * asynchronously write the payload to the underlying stream.
         *
         * If the channel is already asynchronously writing data, all
         * subsequent transmits will remain in the current write buffer and
         * automatically be asynchronously written in a bulk operation when the
         * initial write has completed (see AsyncStreamChannel::asyncWrite()).
         *
         * @param header The header to use in the transmission.
         *
         * @param instance The instance to transmit.
         */
        void transmitImpl(const DotsHeader& header, const type::Struct& instance) override
        {
            serializeTransmission(header, instance);

            if (!m_asyncWriting)
            {
                asyncWrite();
            }
        }

        /*!
         * @brief Asynchronously transmit a transmission through the channel.
         *
         * This will serialize a given dots::io::Transmission and
         * asynchronously write the payload to the underlying stream.
         *
         * If the channel is already asynchronously writing data, all
         * subsequent transmits will remain in the current write buffer and
         * automatically be asynchronously written in a bulk operation when the
         * initial write has completed (see AsyncStreamChannel::asyncWrite()).
         *
         * @param transmission The transmission to transmit.
         */
        void transmitImpl(const Transmission& transmission) override
        {
            serializeTransmission(transmission);

            if (!m_asyncWriting)
            {
                asyncWrite();
            }
        }

    private:
        
        static constexpr size_t ReadBufferMinSize = 16 * 128;

        using transmission_size_t = std::conditional_t<TransmissionFormat == TransmissionFormat::Legacy, dots::uint16_t, dots::uint32_t>;
        static constexpr size_t TransmissionSizeSize = TransmissionFormat == TransmissionFormat::Legacy ? sizeof(dots::uint16_t) : sizeof(dots::uint32_t) + 1;

        using iterator_t = typename buffer_t::iterator;

        /*!
         * @brief Asynchronously read at least a specific amount of bytes.
         *
         * This will asynchronously read data until at least a given amount of
         * bytes is available in the read buffer. The operation will be
         * performed by one or multiple reads on the underlying stream object.
         *
         * If the required amount of data is already present in the read buffer
         * from a previous operation, the read will be performed as a
         * continuation of the call via boost::asio::dispatch(), which may
         * invoke the @p handler synchronously before this function returns.
         *
         * This process will be repeated until the read buffer has too little
         * data available, in which case an asynchronous read will again be
         * initiated on the underlying stream object.
         *
         * @tparam Handler The type of the handler to invoke once the read has
         * been completed.
         *
         * @param requiredBytes The minimum amount of bytes to read.
         *
         * @param handler The handler to invoke once the read has been
         * completed.
         */
        template <typename Handler>
        void asyncRead(size_t requiredBytes, Handler&& handler)
        {
            if (requiredBytes <= m_serializer.inputAvailable())
            {
                m_readDispatching = true;

                boost::asio::dispatch(m_stream.get_executor(), [this, this_{ weak_from_this() }, handler{ std::forward<Handler>(handler) }]
                {
                    try
                    {
                        if (this_.expired())
                        {
                            return;
                        }

                        handler();
                    }
                    catch (...)
                    {
                        processError(std::current_exception());
                    }
                });

                if (m_readDispatching)
                {
                    m_readDispatching = false;
                }
                else
                {
                    asyncReceiveImpl();
                }
            }
            else
            {
                size_t availableBytes = m_serializer.inputAvailable();
                requiredBytes -= availableBytes;

                std::copy(m_serializer.inputData(), m_serializer.inputDataEnd(), m_readBuffer.begin());
                m_readBuffer.resize(std::max(ReadBufferMinSize, availableBytes + requiredBytes));
                m_serializer.setInput(m_readBuffer.data(), availableBytes);

                uint8_t* readBufferBegin = m_readBuffer.data() + availableBytes;
                uint8_t* readBufferEnd = m_readBuffer.data() + m_readBuffer.size();

                m_stream.async_read_some(boost::asio::buffer(readBufferBegin, readBufferEnd - readBufferBegin), [this, this_{ weak_from_this() }, requiredBytes, handler{ std::forward<Handler>(handler)}](boost::system::error_code ec, size_t bytesRead)
                {
                    try
                    {
                        if (this_.expired())
                        {
                            return;
                        }

                        verifyErrorCode(ec);

                        m_serializer.setInput(m_serializer.inputData(), m_serializer.inputAvailable() + bytesRead);

                        if (m_serializer.inputAvailable() < requiredBytes)
                        {
                            asyncRead(requiredBytes, handler);
                        }
                        else
                        {
                            handler();
                        }
                    }
                    catch (...)
                    {
                        processError(std::current_exception());
                    }
                });
            }
        }

        /*!
         * @brief Asynchronously write all outstanding payloads.
         *
         * This will asynchronously write all data in the current write buffer
         * as a bulk operation via the underlying stream object.
         *
         * When the operation has completed and additional payloads were
         * buffered in the meantime, another asynchronous write operation will
         * be initiated automatically.
         *
         * This process will continue until the write buffer is empty.
         */
        void asyncWrite()
        {
            m_writeBuffer.swap(m_serializer.output());
            m_serializer.output().clear();

            if (m_writeBuffer.empty())
            {
                m_asyncWriting = false;
            }
            else
            {
                boost::asio::async_write(m_stream, boost::asio::buffer(m_writeBuffer.data(), m_writeBuffer.size()), [&, this_{ weak_from_this() }](boost::system::error_code ec, size_t/* numBytes*/)
                {
                    try
                    {
                        if (this_.expired())
                        {
                            return;
                        }

                        verifyErrorCode(ec);
                        asyncWrite();
                    }
                    catch (...)
                    {
                        processError(std::current_exception());
                    }
                });
                
                m_asyncWriting = true;
            }
        }

        /*!
         * @brief Deserialize the size of the DotsTransportHeader from the
         * current input data.
         *
         * Note that this function is only available if legacy transmissions
         * are used.
         *
         * @tparam TransmissionFormat_ Defaulted helper value parameter used
         * for SFINAE. Do not specify manually!
         *
         * @return size_t The deserialized header size.
         */
        template <io::TransmissionFormat TransmissionFormat_ = TransmissionFormat, std::enable_if_t<TransmissionFormat_ == TransmissionFormat::Legacy, int> = 0>
        size_t deserializeHeaderSize()
        {
            auto transmissionSize = reinterpret_cast<const transmission_size_t&>(* m_serializer.inputData());
            m_serializer.setInput(m_serializer.inputData() + TransmissionSizeSize, m_serializer.inputAvailable() - TransmissionSizeSize);

            return transmissionSize;
        }

        /*!
         * @brief Deserialize the size of the transmission from the current
         * input data.
         *
         * @return size_t The deserialized transmission size.
         */
        size_t deserializeTransmissionSize()
        {
            if constexpr (TransmissionFormat == TransmissionFormat::Legacy)
            {
                m_serializer.deserialize(m_transportHeader);

                if (!m_transportHeader.payloadSize.isValid())
                {
                    throw std::runtime_error{ "received header without payloadSize" };
                }

                return m_transportHeader.payloadSize;
            }
            else
            {
                return static_cast<size_t>(m_serializer.template deserialize<transmission_size_t>());
            }
        }

        /*!
         * @brief Deserialize a transmission from the current input data.
         *
         * @return Transmission The deserialized transmission.
         */
        Transmission deserializeTransmission()
        {
            if constexpr (TransmissionFormat == TransmissionFormat::Legacy)
            {
                type::AnyStruct instance{ registry().getStructType(*m_transportHeader.dotsHeader->typeName) };
                m_serializer.deserialize(*instance);

                return Transmission{ std::move(m_transportHeader.dotsHeader), std::move(instance) };
            }
            else
            {
                auto header = m_serializer.template deserialize<DotsHeader>();
                type::AnyStruct instance{ registry().getStructType(*header.typeName) };
                m_serializer.deserialize(*instance);

                return Transmission{ std::move(header), std::move(instance) };
            }
        }

        /*!
         * @brief Serialize a transmission into the current write buffer.
         *
         * Note that when @p TransmissionFormat is set to legacy, the
         * serialization format will consist of a DotsTransportHeader with
         * various steps of preprocessing to ensure backwards compatibility
         * with legacy DOTS applications.
         *
         * @param header The header to serialize.
         *
         * @param instance The instance to serialize.
         *
         * @return iterator_t An iterator to the begin of the area of the write
         * buffer that is used by the serialized payload.
         */
        iterator_t serializeTransmission(const DotsHeader& header, const type::Struct& instance)
        {
            if constexpr (TransmissionFormat == TransmissionFormat::Legacy)
            {
                serializer_t serializer;
                serializer.serialize(instance, header.attributes);
                std::vector<uint8_t> serializedInstance = std::move(serializer.output());

                DotsTransportHeader transportHeader{
                    DotsTransportHeader::dotsHeader_i{ header },
                    DotsTransportHeader::payloadSize_i{ static_cast<uint32_t>(serializedInstance.size()) }
                };

                // adjust header for backwards compatibility to legacy implementation
                {
                    // always set destination group
                    transportHeader.destinationGroup = transportHeader.dotsHeader->typeName;

                    // conditionally set namespace
                    if (instance._descriptor().internal() && !instance._is<DotsClient>() && !instance._is<DotsDescriptorRequest>())
                    {
                        transportHeader.nameSpace("SYS");
                    }

                    // set mandatory sent time if not valid
                    if (!transportHeader.dotsHeader->sentTime.isValid())
                    {
                        transportHeader.dotsHeader->sentTime(types::timepoint_t::Now());
                    }

                    // set mandatory sender if not valid. note that a fixed server id for the sender can be used here because
                    // in case of a client connection the id is handled on the server's side an will be overwritten anyway
                    if (!transportHeader.dotsHeader->sender.isValid())
                    {
                       transportHeader.dotsHeader->sender = 1;
                    }
                }
                
                uint16_t serializedHeaderSize = static_cast<uint16_t>(serializer.serialize(transportHeader));
                auto* serializedHeaderSizeData = reinterpret_cast<uint8_t*>(&serializedHeaderSize);
                std::vector<uint8_t> serializedHeader = std::move(serializer.output());

                buffer_t& writeBuffer = m_serializer.output();

                size_t beginIndex = writeBuffer.size();
                std::copy(serializedHeaderSizeData, serializedHeaderSizeData + sizeof(serializedHeaderSize), std::back_inserter(writeBuffer));
                writeBuffer.insert(writeBuffer.end(), serializedHeader.begin(), serializedHeader.end());
                writeBuffer.insert(writeBuffer.end(), serializedInstance.begin(), serializedInstance.end());

                return writeBuffer.begin() + beginIndex;
            }
            else
            {
                buffer_t& writeBuffer = m_serializer.output();

                // create storage area for transmission size
                size_t beginIndex = writeBuffer.size();
                writeBuffer.resize(writeBuffer.size() + TransmissionSizeSize);

                // serialize header and instance
                m_serializer.serialize(header);
                m_serializer.serialize(instance, header.attributes);

                // serialize transmission size into previously created storage
                // area. note that the transmission size is encoded as a fixed size
                // unsigned CBOR integer
                size_t payloadSize = writeBuffer.size() - beginIndex;
                auto transmissionSize = static_cast<transmission_size_t>(payloadSize - TransmissionSizeSize);
                size_t sizeIndex = beginIndex;
                writeBuffer[sizeIndex++] = static_cast<uint8_t>(0x1A);

                for (int16_t i = sizeof(transmission_size_t) - 1; i >= 0; --i)
                {
                    writeBuffer[sizeIndex++] = static_cast<uint8_t>(transmissionSize >> i * 8);
                }

                return writeBuffer.begin() + beginIndex;
            }
        }

        /*!
         * @brief Serialize a transmission into the current write buffer.
         *
         * If a payload cache was provided in
         * AsyncStreamChannel(Channel::key_t, stream_t&&, payload_cache_t*),
         * the function will attempt to retrieve the payload from the cache
         * based on the id of the given transmission.
         *
         * Otherwise the transmission will be serialized as in
         * AsyncStreamChannel::serializeTransmission(const DotsHeader&, const
         * type::Struct&).
         *
         * @param transmission The transmission to serialize.
         */
        void serializeTransmission(const Transmission& transmission)
        {
            if (m_payloadCache == nullptr)
            {
                serializeTransmission(transmission.header(), transmission.instance());
            }
            else
            {
                buffer_t& writeBuffer = m_serializer.output();

                if (*m_payloadCache != std::nullopt && transmission.id() == (*m_payloadCache)->first)
                {
                    writeBuffer.insert(writeBuffer.end(), (*m_payloadCache)->second.begin(), (*m_payloadCache)->second.end());
                }
                else
                {
                    auto begin = serializeTransmission(transmission.header(), transmission.instance());
                    (*m_payloadCache).emplace(transmission.id(), buffer_t(begin, writeBuffer.end()));
                }
            }
        }

        stream_t m_stream;

        serializer_t m_serializer;
        buffer_t m_readBuffer;
        buffer_t m_writeBuffer;
        payload_cache_t* m_payloadCache;
        bool m_asyncWriting;
        bool m_readDispatching;
        DotsTransportHeader m_transportHeader;
    };
}
