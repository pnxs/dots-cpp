#pragma once
#include <map>
#include <functional>
#include <dots/type/AnyStruct.h>
#include <DotsHeader.dots.h>
#include <DotsCloneInformation.dots.h>

namespace dots
{
    template <typename = type::Struct>
    struct Container;

    /*!
     * @class template <> Container<type::Struct> Container.h
     * <dots/Container.h>
     *
     * @brief Data structure for storing DOTS instances alongside meta
     * information.
     *
     * This class implements the DOTS container concept. It is used to hold
     * local clones of all current instances of a particular DOTS type,
     * including meta information, such as timestamps and sender data.
     *
     * The contents of a Container can be inspected in a variety of ways,
     * such as through lookup or iteration.
     *
     * Technically, a Container is an associative data structure that is
     * updated by a dots::Dispatcher based on incoming transmissions (i.e.
     * a dots::type::Struct instance annotated by a DotsHeader).
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage Container objects themselves. Instead,
     * Container references can be retrieved and used for inspection via
     * the corresponding dots::Transceiver.
     *
     * @remark This specialization is used as a base class for all DOTS
     * struct types. A typed version of the current Container can be
     * obtained via the Container::as() cast helper functions.
     *
     * @remark Container objects are usually managed by a
     * dots::ContainerPool.
     */
    template <>
    struct Container<type::Struct>
    {
        struct key_compare
        {
            using is_transparent = void;

            key_compare(const type::StructDescriptor<>& descriptor);
            bool operator () (const type::Struct& lhs, const type::Struct& rhs) const;
            bool operator () (const type::AnyStruct& lhs, const type::Struct& rhs) const;
            bool operator () (const type::Struct& lhs, const type::AnyStruct& rhs) const;
            bool operator () (const type::AnyStruct& lhs, const type::AnyStruct& rhs) const;

        private:

            type::partial_property_descriptor_container_t m_keyPropertyDescriptors;
        };

        using container_t = std::map<type::AnyStruct, DotsCloneInformation, key_compare>;
        using const_iterator_t = container_t::const_iterator;
        using value_t = container_t::value_type;
        using node_t = container_t::node_type;

        /*!
         * @brief Construct a new Container object for a given DOTS struct
         * type.
         *
         * @param descriptor The DOTS struct type of the Container.
         */
        Container(const type::StructDescriptor<>& descriptor);
        Container(const Container& other) = default;
        Container(Container&& other) = default;
        ~Container() = default;

        Container& operator = (const Container& rhs) = default;
        Container& operator = (Container&& rhs) = default;

        /*!
         * @brief Get the DOTS struct type of the Container.
         *
         * Note that this is the same descriptor that was given in Container().
         *
         * @return const type::StructDescriptor<>& A reference to the DOTS
         * struct type of the container.
         */
        const type::StructDescriptor<>& descriptor() const &;
        const type::StructDescriptor<>& descriptor() && = delete;

        /*!
         * @brief Get a constant iterator to the beginning of the Container.
         *
         * @return const_iterator_t A constant iterator to the beginning of the
         * Container.
         */
        const_iterator_t begin() const &;
        const_iterator_t begin() && = delete;

        /*!
         * @brief Get a constant iterator to the end of the Container.
         *
         * @return const_iterator_t A constant iterator to the end of the
         * Container.
         */
        const_iterator_t end() const &;
        const_iterator_t end() && = delete;

        /*!
         * @brief Get a constant iterator to the beginning of the Container.
         *
         * @return const_iterator_t A constant iterator to the beginning of the
         * Container.
         */
        const_iterator_t cbegin() const &;
        const_iterator_t cbegin() && = delete;

        /*!
         * @brief Get a constant iterator to the end of the Container.
         *
         * @return const_iterator_t A constant iterator to the end of the
         * Container.
         */
        const_iterator_t cend() const &;
        const_iterator_t cend() && = delete;

        /*!
         * @brief Check whether the Container is empty (i.e. contains no
         * clones).
         *
         * @return true If the Container has no instances.
         * @return false If the Container has at least one instance.
         */
        bool empty() const &;
        bool empty() && = delete;

        /*!
         * @brief Get the number of instances in the Container.
         *
         * @return size_t The number of instances in the Container.
         */
        size_t size() const &;
        size_t size() && = delete;

        /*!
         * @brief Try to find the clone of a specific instance.
         *
         * The lookup is performed by searching for an instance that is equal
         * to a given instance in its key properties.
         *
         * @warning The Container does not check whether the given instance is
         * of the same type as the container. Users are advised to use the
         * typed specialization of a Container to ensure that instances are
         * compatible.
         *
         * @attention Non-key properties of the given instance are ignored and
         * may not be equal to the corresponding properties of the clone.
         *
         * @remark This function should only be used when the entire clone
         * information including meta data is needed. When the instance itself
         * is sufficient, find() is usually a better choice.
         *
         * @param instance The key instance whose clone to find (non-key
         * properties are ignored).
         *
         * @return const value_t* A pointer to the clone. Will be nullptr if no
         * clone was found.
         */
        const value_t* findClone(const type::Struct& instance) const &;
        const value_t* findClone(const type::Struct& instance) && = delete;

        /*!
         * @brief Get the clone of a specific instance.
         *
         * This function is similar to findClone() but throws an exception if
         * the clone could not be found.
         *
         * @warning The Container does not check whether the given instance is
         * of the same type as the container. Users are advised to use the
         * typed specialization of a Container to ensure that instances are
         * compatible.
         *
         * @attention Non-key properties of the given instance are ignored and
         * may not be equal to the corresponding properties of the clone.
         *
         * @remark This function should only be used when the entire clone
         * information including meta data is needed. When the instance itself
         * is sufficient, get() is usually a better choice.
         *
         * @param instance The key instance whose clone to find (non-key
         * properties are ignored).
         *
         * @return const value_t& A reference to the clone.
         *
         * @exception std::logic_error Thrown if no clone was found.
         */
        const value_t& getClone(const type::Struct& instance) const &;
        const value_t& getClone(const type::Struct& instance) && = delete;

        /*!
         * @brief Try to find the clone of a specific instance.
         *
         * The lookup is performed by searching for an instance that is equal
         * to a given instance in its key properties.
         *
         * @warning The Container does not check whether the given instance is
         * of the same type as the container. Users are advised to use the
         * typed specialization of a Container to ensure that instances are
         * compatible.
         *
         * @attention Non-key properties of the given instance are ignored and
         * may not be equal to the corresponding properties of the clone.
         *
         * @remark This function only returns the actual instance of the clone.
         * When the entire clone information is needed, it can be obtained by
         * using findClone() instead.
         *
         * @param instance The key instance whose clone instance to find
         * (non-key properties are ignored).
         *
         * @return const type::Struct* A pointer to the clone instance. Will be
         * nullptr if no clone was found.
         */
        const type::Struct* find(const type::Struct& instance) const &;
        const type::Struct* find(const type::Struct& instance) && = delete;

        /*!
         * @brief Get the clone of a specific instance.
         *
         * This function is similar to find() but throws an exception if the
         * clone could not be found.
         *
         * @warning The Container does not check whether the given instance is
         * of the same type as the container. Users are advised to use the
         * typed specialization of the Container to ensure that instances are
         * compatible.
         *
         * @attention Non-key properties of the given instance are ignored and
         * may not be equal to the corresponding properties of the clone.
         *
         * @remark This function only returns the actual instance of the clone.
         * When the entire clone information is needed, it can be obtained by
         * using getClone() instead.
         *
         * @param instance The key instance whose clone instance to find
         * (non-key properties are ignored).
         *
         * @return const type::Struct& A reference to the clone instance.
         *
         * @exception std::logic_error Thrown if no clone was found.
         */
        const type::Struct& get(const type::Struct& instance) const &;
        const type::Struct& get(const type::Struct& instance) && = delete;

        /*!
         * @brief Insert an instance into the Container and update clone
         * information.
         *
         * This function will insert a given instance into the Container and
         * update the clone information based on the header data and whether or
         * not the instance was known to the Container before the call.
         *
         * If the Container did not contain a coresponding clone, a new clone
         * will be created and the DotsCloneInformation::lastOperation property
         * will be set to DotsMt::create. Otherwise the existing clone will be
         * updated and the last operation set to DotsMt::update.
         *
         * @attention This function will always perform an insertion even if
         * the remove flag in the \p header is set to true.
         *
         * @param header The header data to consider.
         *
         * @param instance The instance to insert.
         *
         * @return const value_t& A reference to the inserted (i.e. created or
         * updated) clone.
         */
        const value_t& insert(const DotsHeader& header, const type::Struct& instance) &;
        const value_t& insert(const DotsHeader& header, const type::Struct& instance) && = delete;

        /*!
         * @brief Try to remove an instance from the Container and update clone
         * information.
         *
         * This function will try to remove a given instance from the
         * Container. Technically, this will result in an attempted extraction
         * of the clone node.
         *
         * If the Container contains a corresponding clone, the clone
         * information will be updated based on the header data and the
         * DotsCloneInformation::lastOperation property be set to
         * DotsMt::remove. Afterwards the extracted node will be returned.
         *
         * If the remove is ineffective (i.e. no corresponding clone could be
         * found), an empty node will be returned.
         *
         * @attention This function will always perform a remove even if the
         * remove flag in the \p header is set to false.
         *
         * @param header The header data to consider.
         *
         * @param instance The instance to insert.
         *
         * @return node_t The removed clone node. Will be an empty node if the
         * instance to remove was not found.
         */
        node_t remove(const DotsHeader& header, const type::Struct& instance) &;
        node_t remove(const DotsHeader& header, const type::Struct& instance) && = delete;

        /*!
         * @brief Clear the container.
         *
         * Calling this function will erase all clones, resulting in an empty
         * Container.
         */
        void clear() &;
        void clear() && = delete;

        /*!
         * @brief Iterate over all clones in the Container.
         *
         * This function will iterate over all clones in the Container and
         * invoke the given handler for each element.
         *
         * Note that for many uses cases it is easier to just use range-based
         * for loop to iterate over the Container:
         *
         * @code{.cpp}
         * for (const auto& [instance, cloneInfo] : dots::container<T>())
         * {
         *     const T& instance_ = instance.to<T>();
         *     ...
         * }
         * @endcode
         *
         * @remark This function should only be used when the entire clone
         * information including meta data is needed. When the instance itself
         * is sufficient, forEach() is usually a better choice.
         *
         * @param f The callback handler to invoke for each clone.
         */
        void forEachClone(const std::function<void(const value_t&)>& f) const &;
        void forEachClone(const std::function<void(const value_t&)>& f) && = delete;

        /*!
         * @brief Iterate over all instances in the Container.
         *
         * This function will iterate over all clones in the Container and
         * invoke the given handler for each clone instance.
         *
         * Note that for many uses cases it is easier to just use range-based
         * for loop to iterate over the Container:
         *
         * @code{.cpp}
         * for (const auto& [instance, cloneInfo] : dots::container<T>())
         * {
         *     const T& instance_ = instance.to<T>();
         *     ...
         * }
         * @endcode
         *
         * @param f The callback handler to invoke for each instance.
         */
        void forEach(const std::function<void(const type::Struct&)>& f) const &;
        void forEach(const std::function<void(const type::Struct&)>& f) && = delete;

        /*!
         * @brief Accumulate the total memory usage of all instances in the
         * Container.
         *
         * This function accumulates both the static and dynamic memory usage
         * of all instances in the Container.
         *
         * @attention Beware that this includes only the memory used by the
         * instances themselves. It does not take into account the memory used
         * for storing the clone meta information, as well as the overhead of
         * the data structure the clones are stored in.
         *
         * @see dots::type::Struct::_totalMemoryUsage().
         *
         * @return size_t The total memory size of all instances in the
         * Container.
         */
        size_t totalMemoryUsage() const &;
        size_t totalMemoryUsage() && = delete;

        /*!
         * @brief Safely cast the Container to the explicitly typed version.
         *
         * @tparam T The DOTS type to safely cast to.
         *
         * @return const Container<T>& A reference to the Container casted to
         * the explicitly typed version.
         *
         * @exception std::runtime_error Thrown if @p T does not match the type
         * of the Container.
         */
        template <typename T>
        const Container<T>& as() const &
        {
            static_assert(std::is_base_of_v<type::Struct, T>);

            if (&T::_Descriptor() != m_descriptor)
            {
                throw std::runtime_error{ "type mismatch: expected " + m_descriptor->name() + " but got " + T::_Descriptor().name() };
            }

            return static_cast<const Container<T>&>(*this);
        }

        /*!
         * @brief Safely cast the Container to the explicitly typed version.
         *
         * @tparam T The DOTS type to safely cast to.
         *
         * @return const Container<T>& A reference to the Container casted to
         * the explicitly typed version.
         *
         * @exception std::runtime_error Thrown if @p T does not match the type
         * of the Container.
         */
        template <typename T>
        Container<T>& as() &
        {
            return const_cast<Container<T>&>(std::as_const(*this).as<T>());
        }

        template <typename T>
        const Container<T>& as() && = delete;

    private:

        const type::StructDescriptor<>* m_descriptor;
        container_t m_instances;
    };

    /*!
     * @brief Explicitly typed version of the Container data structure.
     *
     * This is an explicitly typed version of the Container. It inherits
     * all capabilities from the dots::Container<> specialization, but also
     * offers various functions in a typed variant.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage Container objects themselves. Instead,
     * Container references can be retrieved and used for inspection via
     * the corresponding dots::Transceiver.
     *
     * @tparam T The DOTS struct type of the container.
     */
    template <typename T>
    struct Container : Container<type::Struct>
    {
        static_assert(std::is_base_of_v<type::Struct, T>);

        Container() :
            Container<type::Struct>(T::_Descriptor())
        {
            /* do nothing */
        }
        Container(const Container& other) = default;
        Container(Container&& other) = default;
        ~Container() = default;

        Container& operator = (const Container& rhs) = default;
        Container& operator = (Container&& rhs) = default;

        /*!
         * @brief Try to find the single clone instance.
         *
         * This is an explicitly typed version of Container<>::find().
         *
         * @attention This overload is only available if T has no key
         * properties.
         *
         * @tparam T_ Defaulted helper type parameter used for SFINAE. Do not
         * specify manually!
         *
         * @return const T* A pointer to the clone instance. Will be nullptr if
         * no clone was found.
         */
        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> == 0, int> = 0>
        const T* find() const &
        {
            return static_cast<const T*>(Container<>::find(T{}));
        }

        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> == 0, int> = 0>
        const T* find() && = delete;

        /*!
         * @brief Get the single clone instance.
         *
         * This function is similar to find() but throws an exception if the
         * clone could not be found.
         *
         * @attention This overload is only available if T has no key
         * properties.
         *
         * @tparam T_ Defaulted helper type parameter used for SFINAE. Do not
         * specify manually!
         *
         * @return T& A reference to the clone instance.
         *
         * @exception std::logic_error Thrown if no clone was found.
         */
        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> == 0, int> = 0>
        const T& get() const &
        {
            return static_cast<const T&>(Container<>::get(T{}));
        }

        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> == 0, int> = 0>
        const T& get() && = delete;

        /*!
         * @brief Try to find the clone of a specific instance.
         *
         * This is an explicitly typed version of Container<>::find().
         *
         * The lookup is performed by searching for an instance that is equal
         * to a given instance in its key properties.
         *
         * @attention This overload is only available if T has at least one key
         * property.
         *
         * @attention Non-key properties of the given instance are ignored and
         * may not be equal to the corresponding properties of the clone.
         *
         * @tparam T_ Defaulted helper type parameter used for SFINAE. Do not
         * specify manually!
         *
         * @param instance The key instance whose clone instance to find
         * (non-key properties are ignored).
         *
         * @return const T* A pointer to the clone instance. Will be nullptr if
         * no clone was found.
         */
        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> >= 1, int> = 0>
        const T* find(const T& instance) const &
        {
            return static_cast<const T*>(Container<>::find(instance));
        }

        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> >= 1, int> = 0>
        const T* find(const T& instance) && = delete;

        /*!
         * @brief Get the clone of a specific instance.
         *
         * This function is similar to find() but throws an exception if the
         * clone could not be found.
         *
         * @attention This overload is only available if T has at least one key
         * property.
         *
         * @attention Non-key properties of the given instance are ignored and
         * may not be equal to the corresponding properties of the clone.
         *
         * @tparam T_ Defaulted helper type parameter used for SFINAE. Do not
         * specify manually!
         *
         * @param instance The key instance whose clone instance to find
         * (non-key properties are ignored).
         *
         * @return T& A reference to the clone instance.
         *
         * @exception std::logic_error Thrown if no clone was found.
         */
        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> >= 1, int> = 0>
        const T& get(const T& instance) const &
        {
            return static_cast<const T&>(Container<>::get(instance));
        }

        template <typename T_ = T, std::enable_if_t<std::tuple_size_v<typename T_::_key_properties_t> >= 1, int> = 0>
        const T& get(const T& instance) && = delete;

        /*!
         * @brief Iterate over all instances in the Container.
         *
         * This is an explicitly typed version of Container<>::forEach().
         *
         * This function will iterate over all instances in the Container and
         * invoke the given handler for each element.
         *
         * Note that for some use cases it might be easier or even required
         * (e.g. for breaking) to use a range-based for loop to iterate over
         * the Container:
         *
         * @code{.cpp}
         * for (const auto& [instance, cloneInfo] : dots::container<T>())
         * {
         *     const T& instance_ = instance.to<T>();
         *     ...
         * }
         * @endcode
         *
         * @param f The callback handler to invoke for each instance.
         */
        void forEach(const std::function<void(const T&)>& f) const &
        {
            forEachClone([&](const value_t& value)
            {
                f(value.first.to<T>());
            });
        }

        void forEach(const std::function<void(const T&)>& f) && = delete;

    private:

        using Container<>::as;
    };
}