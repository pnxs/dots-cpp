#pragma once
#include <map>
#include <unordered_map>
#include <string_view>
#include <dots/Container.h>

namespace dots
{
    /*!
     * @class ContainerPool ContainerPool.h <dots/ContainerPool.h>
     *
     * @brief Data structure for managing dots::Container objects.
     *
     * This class is an associative data structure that stores at most one
     * Container for each type. It is usually used by a dots::Dispatcher to
     * hold Container objects.
     *
     * The contents of a ContainerPool can be inspected in a variety of
     * ways, such as through lookup or iteration.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage ContainerPool objects themselves.
     * Instead, ContainerPool references can be retrieved and used for
     * inspection via the corresponding dots::Transceiver.
     */
    struct ContainerPool
    {
        using pool_t = std::unordered_map<const type::StructDescriptor<>*, Container<>>;
        using iterator_t = pool_t::iterator;
        using const_iterator_t = pool_t::const_iterator;
        using value_t = pool_t::value_type;
        using node_t = pool_t::node_type;

        /*!
         * @brief Get a constant iterator to the beginning of the
         * ContainerPool.
         *
         * @return const_iterator_t A constant iterator to the beginning of the
         * ContainerPool.
         */
        const_iterator_t begin() const;

        /*!
         * @brief Get a constant iterator to the end of the ContainerPool.
         *
         * @return const_iterator_t A constant iterator to the end of the
         * ContainerPool.
         */
        const_iterator_t end() const;

        /*!
         * @brief Get an iterator to the beginning of the ContainerPool.
         *
         * @return const_iterator_t A iterator to the beginning of the
         * ContainerPool.
         */
        iterator_t begin();

        /*!
         * @brief Get an iterator to the end of the ContainerPool.
         *
         * @return const_iterator_t A iterator to the end of the ContainerPool.
         */
        iterator_t end();

        /*!
         * @brief Get a constant iterator to the beginning of the
         * ContainerPool.
         *
         * @return const_iterator_t A constant iterator to the beginning of the
         * ContainerPool.
         */
        const_iterator_t cbegin() const;

        /*!
         * @brief Get a constant iterator to the end of the ContainerPool.
         *
         * @return const_iterator_t A constant iterator to the end of the
         * ContainerPool.
         */
        const_iterator_t cend() const;

        /*!
         * @brief Check whether the ContainerPool is empty (i.e. contains no
         * Container objects).
         *
         * @return true If the ContainerPool manages no Container objects.
         * @return false If the ContainerPool manages at least one Container.
         */
        bool empty() const;

        /*!
         * @brief Get the number of Container objects in the ContainerPool.
         *
         * @return size_t The number of Container objects managed by the
         * ContainerPool.
         */
        size_t size() const;

        /*!
         * @brief Try to find a specific Container by type.
         *
         * @param descriptor The type descriptor of the Container to find.
         *
         * @return const Container<>* A pointer to the Container. Will be
         * nullptr if no Container for the given type was found.
         */
        const Container<>* find(const type::StructDescriptor<>& descriptor) const;

        /*!
         * @brief Get a specific container by type.
         *
         * @param descriptor The type descriptor of the Container to get.
         *
         * @param insertIfNotExist Specifies whether or not the Container
         * should be created if it does not exist.
         *
         * @return const Container<>& A reference to the Container.
         *
         * @exception std::runtime_error Thrown if no Container for @p
         * descriptor was found and @p insertIfNotExist was given as false.
         */
        const Container<>& get(const type::StructDescriptor<>& descriptor, bool insertIfNotExist = true) const;

        /*!
         * @brief Try to find a specific Container by type.
         *
         * @param descriptor The type descriptor of the Container to find.
         *
         * @return Container<>* A pointer to the Container. Will be nullptr if
         * no Container for the given type was found.
         */
        Container<>* find(const type::StructDescriptor<>& descriptor);

        /*!
         * @brief Get a specific container by type.
         *
         * @param descriptor The type descriptor of the Container to get.
         *
         * @param insertIfNotExist Specifies whether or not the Container
         * should be created if it does not exist.
         *
         * @return Container<>& A reference to the Container.
         *
         * @exception std::runtime_error Thrown if no Container for @p
         * descriptor was found and @p insertIfNotExist was given as false.
         */
        Container<>& get(const type::StructDescriptor<>& descriptor, bool insertIfNotExist = true);

        /*!
         * @brief Try to find a specific Container by type name.
         *
         * @param name The type name of the Container to find.
         *
         * @return const Container<>* A pointer to the Container. Will be
         * nullptr if no Container for the given type was found.
         */
        const Container<>* find(std::string_view name) const;

        /*!
         * @brief Get a specific container by name.
         *
         * @param name The type name of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         *
         * @exception std::runtime_error Thrown if no Container for @p a type
         * with @p name was found.
         */
        const Container<>& get(std::string_view name) const;

        /*!
         * @brief Try to find a specific Container by type name.
         *
         * @param name The type name of the Container to find.
         *
         * @return Container<>* A pointer to the Container. Will be nullptr if
         * no Container for the given type was found.
         */
        Container<>* find(std::string_view name);

        /*!
         * @brief Get a specific container by name.
         *
         * @param name The type name of the Container to get.
         *
         * @return Container<>& A reference to the Container.
         *
         * @exception std::runtime_error Thrown if no Container for @p a type
         * with @p name was found.
         */
        Container<>& get(std::string_view name);

        /*!
         * @brief Remove Container for a specific type from the ContainerPool.
         *
         * Technically this will attempt to extract and return the Container
         * node from the underlying data structure.
         *
         * @param descriptor The type descriptor of the Container to remove.
         *
         * @return node_t The removed Container node.
         *
         * @exception std::runtime_error Thrown if the ContainerPool does not
         * have a container for the given type @p descriptor.
         */
        node_t remove(const type::StructDescriptor<>& descriptor);

        /*!
         * @brief Iterate over all Container objects in the ContainerPool.
         *
         * This function will iterate over all Container objects in the
         * ContainerPool and invoke the given handler for each element.
         *
         * Note that for some use cases it might be easier or even required
         * (e.g. for breaking) to use a range-based for loop for iteration:
         *
         * @code{.cpp} for (const auto& [descriptor, container] : dots::pool())
         * {
         *     ...
         * }
         * @endcode
         *
         * @param f The callback handler to invoke for each Container.
         */
        void forEach(const std::function<void(const Container<>&)>& f) const;

        /*!
         * @brief Accumulate the total memory usage of all instances in all
         * Container objects the ContainerPool.
         *
         * @see dots::Container::totalMemoryUsage().
         *
         * @return size_t The total memory size of all instances in the
         * ContainerPool.
         */
        size_t totalMemoryUsage() const;

        /*!
         * @brief Try to find a specific Container by type.
         *
         * @tparam T The type of the Container to find.
         *
         * @return const Container<>* A pointer to the Container. Will be
         * nullptr if no Container for the given type was found.
         */
        template <typename T>
        const Container<T>* find() const
        {
            static_assert(std::is_base_of_v<type::Struct, T>);
            return static_cast<const Container<T>*>(find(T::_Descriptor()));
        }

        /*!
         * @brief Get a specific container by type.
         *
         * Note that this will implicitly create a Container for the given type
         * if it does not yet exist.
         *
         * @tparam T The type of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         */
        template <typename T>
        const Container<T>& get() const
        {
            static_assert(std::is_base_of_v<type::Struct, T>);
            return static_cast<const Container<T>&>(get(T::_Descriptor()));
        }

        /*!
         * @brief Try to find a specific Container by type.
         *
         * @tparam T The type of the Container to find.
         *
         * @return Container<>* A pointer to the Container. Will be nullptr if
         * no Container for the given type was found.
         */
        template <typename T>
        Container<T>* find()
        {
            return const_cast<Container<T>*>(static_cast<const Container<T>*>((std::as_const(*this).find<T>())));
        }

        /*!
         * @brief Get a specific container by type.
         *
         * @param insertIfNotExist Specifies whether or not the Container
         * should be created if it does not exist.
         *
         * @tparam T The type of the Container to get.
         *
         * @return Container<>& A reference to the Container.
         */
        template <typename T>
        Container<T>& get(bool insertIfNotExist = true)
        {
            static_assert(std::is_base_of_v<type::Struct, T>);
            return static_cast<Container<T>&>(get(T::_Descriptor(), insertIfNotExist));
        }

    private:

        using name_cache_t = std::map<std::string, Container<>*, std::less<>>;

        // TODO: remove mutability when utilities are fixed to no longer require non-const pool access
        mutable pool_t m_pool;
        mutable name_cache_t m_nameCache;
    };
}
