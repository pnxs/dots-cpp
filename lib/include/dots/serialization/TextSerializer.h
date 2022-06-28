#pragma once
#include <string>
#include <stack>
#include <vector>
#include <dots/serialization/Serializer.h>
#include <dots/serialization/formats/TextReader.h>
#include <dots/serialization/formats/TextWriter.h>
#include <dots/tools/type_traits.h>

namespace dots::serialization
{
    struct TextSerializerFormat
    {
        enum struct PropertySetFormat : uint8_t
        {
            DecimalValue,
            BinaryValue
        };

        enum struct TimepointFormat : uint8_t
        {
            FractionalSeconds,
            ISO8601String
        };

        enum struct EnumFormat : uint8_t
        {
            Tag,
            Value,
            Identifier,
            String
        };
    };

    template <typename Format, typename Derived>
    struct TextSerializer : Serializer<Format, Derived>
    {
        using serializer_t = Serializer<Format, Derived>;
        using format_t = typename serializer_t::format_t;

        TextSerializer(TextOptions options = {}) :
            serializer_t(options, options)
        {
            /* do nothing */
        }

        using serializer_t::reader;
        using serializer_t::writer;

    protected:

        using visitor_base_t = type::TypeVisitor<Derived>;

        friend visitor_base_t;
        friend serializer_t;

        using serializer_t::visit;

        //
        // serialization
        //

        template <typename T>
        bool visitStructBeginDerived(const T& instance, property_set_t&/* includedProperties*/)
        {
            writer().writeObjectBegin(instance._descriptor().name());
            return true;
        }

        template <typename T>
        void visitStructEndDerived(const T&/* instance*/, const property_set_t&/* includedProperties*/)
        {
            writer().writeObjectEnd();
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T& property, bool/* first*/)
        {
            if (writer().nestingLevel() > 0)
            {
                writer().writeObjectMemberName(property.descriptor().name());
            }

            if (property.isValid())
            {
                return true;
            }
            else
            {
                writer().writeNull();
                return false;
            }
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            writer().writeArrayBegin();
            return true;
        }

        template <typename T>
        void visitVectorEndDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            writer().writeArrayEnd();
        }

        template <typename T>
        void visitEnumDerived(const T& value, const type::EnumDescriptor& descriptor)
        {
            if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::Tag)
            {
                writer().write(descriptor.enumeratorFromValue(value).tag());
            }
            else if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::Value)
            {
                writer().write(type::Typeless::From(value).template to<int32_t>());
            }
            else if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::Identifier)
            {
                writer().writeIdentifierString(descriptor.name(), "::", descriptor.enumeratorFromValue(value).name());
            }
            else if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::String)
            {
                writer().writeQuotedString(descriptor.enumeratorFromValue(value).name());
            }
            else
            {
                static_assert(tools::always_false_v<T>, "unsupported enum format");
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                writer().write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                if constexpr (format_t::PropertySetFormat == TextSerializerFormat::PropertySetFormat::DecimalValue)
                {
                    writer().write(value.toValue(), 10);
                }
                else if constexpr (format_t::PropertySetFormat == TextSerializerFormat::PropertySetFormat::BinaryValue)
                {
                    writer().write(value.toValue(), 2);
                }
                else
                {
                    static_assert(tools::always_false_v<T>, "unsupported property set format");
                }
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (format_t::TimepointFormat == TextSerializerFormat::TimepointFormat::FractionalSeconds)
                {
                    writer().write(value.duration().toFractionalSeconds());
                }
                else if constexpr (format_t::TimepointFormat == TextSerializerFormat::TimepointFormat::ISO8601String)
                {
                    writer().writeQuotedString(value.toString());
                }
                else
                {
                    static_assert(tools::always_false_v<T>, "unsupported timepoint format");
                }
            }
            else if constexpr (std::is_same_v<T, steady_timepoint_t>)
            {
                writer().write(value.duration().toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, duration_t>)
            {
                writer().write(value.toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                writer().writeQuotedString(value.toString());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                writer().writeEscapedString(value);
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        //
        // deserialization
        //

        template <typename T>
        bool visitStructBeginDerived(T& instance, property_set_t& includedProperties)
        {
            const type::StructDescriptor& descriptor = instance._descriptor();
            const type::property_descriptor_container_t& propertyDescriptors = descriptor.propertyDescriptors();

            reader().readObjectBegin(descriptor.name());

            while (!reader().tryReadObjectEnd())
            {
                auto find_property = [&propertyDescriptors](std::string_view propertyName)
                {
                    return std::find_if(propertyDescriptors.begin(), propertyDescriptors.end(), [propertyName](const auto& p) { return p.name() == propertyName; });
                };

                std::string_view propertyName = reader().readObjectMemberName();

                if (visitor_base_t::template visitingLevel<false>() > 0)
                {
                    includedProperties = property_set_t::All;
                }

                if (auto it = find_property(propertyName); it != propertyDescriptors.end() && it->set() <= includedProperties)
                {
                    const type::PropertyDescriptor& propertyDescriptor = *it;
                    type::ProxyProperty<> property{ instance, propertyDescriptor };
                    visit(property);
                }
                else
                {
                    reader().skip();
                }
            }

            return false;
        }

        template <typename T>
        bool visitPropertyBeginDerived(T& property, bool/* first*/)
        {
            if (reader().tryReadNull())
            {
                property = dots::invalid;
                return false;
            }
            else
            {
                property.valueOrEmplace();
                return true;
            }
        }

        template <typename T>
        bool visitVectorBeginDerived(vector_t<T>& vector, const type::Descriptor<vector_t<T>>& descriptor)
        {
            reader().readArrayBegin();

            while (!reader().tryReadArrayEnd())
            {
                descriptor.resize(vector, vector.typelessSize() + 1);

                // TODO: avoid special case for bool
                if constexpr (std::is_same_v<T, bool_t>)
                {
                    visit(reinterpret_cast<bool_t&>(vector.back()), descriptor.valueDescriptor());
                }
                else
                {
                    if constexpr (std::is_same_v<T, type::Typeless>)
                    {
                        visit(vector.typelessAt(vector.typelessSize() - 1), descriptor.valueDescriptor());
                    }
                    else
                    {
                        visit(vector.back(), descriptor.valueDescriptor());
                    }
                }
            }

            return false;
        }

        template <typename T>
        void visitEnumDerived(T& value, const type::EnumDescriptor& descriptor)
        {
            if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::Tag)
            {
                descriptor.construct(value, descriptor.enumeratorFromTag(reader().template read<uint32_t>()).template value<T>());
            }
            else if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::Value)
            {
                auto underlyingValue = reader().template read<uint32_t>();
                descriptor.construct(value, descriptor.enumeratorFromValue(type::Typeless::From(underlyingValue)).value());
            }
            else if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::Identifier)
            {
                std::string_view identifier = reader().readIdentifierString(descriptor.name(), "::");
                descriptor.construct(value, descriptor.enumeratorFromName(identifier).value<T>());
            }
            else if constexpr (format_t::EnumFormat == TextSerializerFormat::EnumFormat::String)
            {
                std::string_view identifier = reader().readQuotedString();
                descriptor.construct(value, descriptor.enumeratorFromName(identifier).value<T>());
            }
            else
            {
                static_assert(tools::always_false_v<T>, "unsupported enum format");
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                value = reader().template read<T>();
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                if constexpr (format_t::PropertySetFormat == TextSerializerFormat::PropertySetFormat::DecimalValue || format_t::PropertySetFormat == TextSerializerFormat::PropertySetFormat::BinaryValue)
                {
                    value = property_set_t{ reader().template read<uint32_t>() };
                }
                else
                {
                    static_assert(tools::always_false_v<T>, "unsupported property set format");
                }
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (format_t::TimepointFormat == TextSerializerFormat::TimepointFormat::FractionalSeconds)
                {
                    value = T{ duration_t{ reader().template read<float64_t>() } };
                }
                else if constexpr (format_t::TimepointFormat == TextSerializerFormat::TimepointFormat::ISO8601String)
                {
                    value = timepoint_t::FromString(reader().readQuotedString());
                }
                else
                {
                    static_assert(tools::always_false_v<T>, "unsupported timepoint format");
                }
            }
            else if constexpr (std::is_same_v<T, steady_timepoint_t> || std::is_same_v<T, duration_t>)
            {
                value = T{ duration_t{ reader().template read<float64_t>() } };
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                value = uuid_t::FromString(reader().readQuotedString());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                value = reader().readEscapedString();
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }
    };
}
