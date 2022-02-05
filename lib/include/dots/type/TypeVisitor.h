#pragma once
#include <dots/type/Struct.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/EnumDescriptor.h>
#include <dots/type/StaticStruct.h>

namespace dots::type
{
    template <typename Derived = void, typename = void>
    struct TypeVisitor;

    template <typename Derived>
    struct TypeVisitor<Derived>
    {
    protected:

        template <bool Const>
        size_t visitingLevel() const
        {
            size_t visitingLevel = [this]
            {
                if constexpr (Const)
                {
                    return m_constVisitingLevel;
                }
                else
                {
                    return m_nonConstVisitingLevel;
                }
            }();

            return visitingLevel == 0 ? 0 : visitingLevel - 1;
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<Struct, T>, int> = 0>
        void visit(const T& instance, PropertySet includedProperties)
        {
            visitBegin<true>();
            visitStructInternal<true, T>(instance, includedProperties);
            visitEnd<true>();
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<Struct, T>, int> = 0>
        void visit(T& instance, PropertySet includedProperties)
        {
            visitBegin<false>();
            visitStructInternal<false, T>(instance, includedProperties);
            visitEnd<false>();
        }

        template <typename T, std::enable_if_t<is_property_v<T>, int> = 0>
        void visit(const T& property, bool first = true)
        {
            visitBegin<true>();
            visitPropertyInternal<true, T>(property, first);
            visitEnd<true>();
        }

        template <typename T, std::enable_if_t<is_property_v<T>, int> = 0>
        void visit(T& property, bool first = true)
        {
            visitBegin<false>();
            visitPropertyInternal<false, T>(property, first);
            visitEnd<false>();
        }

        template <typename T>
        void visit(const T& value, const Descriptor<T>& descriptor)
        {
            visitBegin<true>();
            visitTypeInternal<true>(value, descriptor);
            visitEnd<true>();
        }

        template <typename T>
        void visit(T& value, const Descriptor<T>& descriptor)
        {
            visitBegin<false>();
            visitTypeInternal<false>(value, descriptor);
            visitEnd<false>();
        }

        template <typename... Ts, std::enable_if_t<std::conjunction_v<std::negation<is_property<Ts>>...>, int> = 0>
        void visit(const Ts&... values)
        {
            visitBegin<true>();
            visitTypeInternal<true, Ts...>(values...);
            visitEnd<true>();
        }

        template <typename... Ts, std::enable_if_t<std::conjunction_v<std::negation<std::is_const<Ts>>..., std::negation<is_property<Ts>>...>, int> = 0>
        void visit(Ts&... values)
        {
            visitBegin<false>();
            visitTypeInternal<false, Ts...>(values...);
            visitEnd<false>();
        }

        template <bool Const>
        void visitBeginDerived()
        {
            /* do nothing */
        }

        template <bool Const>
        void visitEndDerived()
        {
            /* do nothing */
        }

        template <typename U>
        bool visitStructBeginDerived(U&/* instance*/, PropertySet&/* includedProperties*/)
        {
            return true;
        }

        template <typename U>
        void visitStructEndDerived(U&/* instance*/, PropertySet/* includedProperties*/)
        {
            /* do nothing */
        }

        template <typename U>
        bool visitPropertyBeginDerived(U&/* property*/, bool/* first*/)
        {
            return true;
        }

        template <typename U>
        void visitPropertyEndDerived(U&/* property*/, bool/* first*/)
        {
            /* do nothing */
        }

        template <typename U>
        bool visitVectorBeginDerived(U&/* vector*/, const VectorDescriptor&/* descriptor*/)
        {
            return true;
        }

        template <typename U>
        bool visitVectorValueBeginDerived(U&/* vector*/, const VectorDescriptor&/* descriptor*/, size_t/* index*/)
        {
            return true;
        }

        template <typename U>
        void visitVectorValueEndDerived(U&/* vector*/, const VectorDescriptor&/* descriptor*/, size_t/* index*/)
        {
            /* do nothing */
        }

        template <typename U>
        void visitVectorEndDerived(U&/* vector*/, const VectorDescriptor&/* descriptor*/)
        {
            /* do nothing */
        }

        template <typename U>
        void visitEnumDerived(U&/* value*/, const EnumDescriptor<std::remove_const_t<U>>&/* descriptor*/)
        {
            /* do nothing */
        }

        template <typename U>
        void visitFundamentalTypeDerived(U&/* value*/, const Descriptor<std::remove_const_t<U>>&/* descriptor*/)
        {
            /* do nothing */
        }

        template <typename... Us>
        bool visitPackBeginDerived(Us&.../* values*/)
        {
            return true;
        }

        template <typename U>
        bool visitPackElementBeginDerived(U&/* value*/, size_t/* index*/, size_t/* size*/)
        {
            return true;
        }

        template <typename U>
        void visitPackElementEndDerived(U&/* value*/, size_t/* index*/, size_t/* size*/)
        {
            /* do nothing */
        }

        template <typename... Us>
        void visitPackEndDerived(Us&.../* values*/)
        {
            /* do nothing */
        }

    private:

        template <bool Condition, typename U>
        using const_t = std::conditional_t<Condition, std::add_const_t<U>, U>;

        template <bool Const>
        size_t incrementVisitingLevel()
        {
            if constexpr (Const)
            {
                return ++m_constVisitingLevel;
            }
            else
            {
                return ++m_nonConstVisitingLevel;
            }
        }

        template <bool Const>
        size_t decrementVisitingLevel()
        {
            if constexpr (Const)
            {
                return --m_constVisitingLevel;
            }
            else
            {
                return --m_nonConstVisitingLevel;
            }
        }

        template <bool Const>
        void visitBegin()
        {
            if (incrementVisitingLevel<Const>() == 1)
            {
                derived().template visitBeginDerived<Const>();
            }
        }

        template <bool Const>
        void visitEnd()
        {
            if (decrementVisitingLevel<Const>() == 0)
            {
                derived().template visitEndDerived<Const>();
            }
        }

        template <bool Const, typename T>
        void visitStructInternal(const_t<Const, T>& instance, PropertySet includedProperties)
        {
            constexpr bool IsStruct = std::is_base_of_v<Struct, T>;
            static_assert(IsStruct);

            if constexpr (IsStruct)
            {
                if (derived().visitStructBeginDerived(instance, includedProperties))
                {
                    incrementVisitingLevel<Const>();

                    if constexpr (std::is_base_of_v<StaticStruct<T>, T>)
                    {
                        instance._applyProperties([&](auto&... properties)
                        {
                            // note: redundant explicit capture of this pointer is necessary here to resolve a
                            // compiling issue with GCC 8.3.
                            bool first = true;
                            auto visit_property = [&, this](auto& property)
                            {
                                if (std::decay_t<decltype(property)>::IsPartOf(includedProperties))
                                {
                                    using property_t = std::decay_t<decltype(property)>;
                                    visitPropertyInternal<Const, property_t>(property, first);
                                    first = false;
                                }
                            };
                            (void)visit_property;

                            (visit_property(properties), ...);
                        });
                    }
                    else
                    {
                        bool first = true;

                        for (const PropertyDescriptor& propertyDescriptor : instance._propertyDescriptors())
                        {
                            if (propertyDescriptor.set() <= includedProperties)
                            {
                                const_t<Const, ProxyProperty<>> property{ const_cast<PropertyArea&>(instance._propertyArea()), propertyDescriptor };
                                using property_t = std::decay_t<decltype(property)>;
                                visitPropertyInternal<Const, property_t>(property, first);
                                first = false;
                            }
                        }
                    }

                    decrementVisitingLevel<Const>();
                    derived().visitStructEndDerived(instance, includedProperties);
                }
            }
        }

        template <bool Const, typename T>
        void visitPropertyInternal(const_t<Const, T>& property, bool first = true)
        {
            using value_t = typename T::value_t;

            if (derived().visitPropertyBeginDerived(property, first))
            {
                if (property.isValid())
                {
                    if constexpr (std::is_same_v<value_t, Typeless>)
                    {
                        visitTypeInternal<Const, value_t>(property.storage(), property.descriptor().valueDescriptor());
                    }
                    else
                    {
                        visitTypeInternal<Const, value_t>(property.storage());
                    }
                }

                derived().visitPropertyEndDerived(property, first);
            }
        }

        template <bool Const, typename T>
        void visitVectorInternal(const_t<Const, Vector<T>>& vector, const Descriptor<Vector<T>>& descriptor)
        {
            if (derived().visitVectorBeginDerived(vector, descriptor))
            {
                incrementVisitingLevel<Const>();

                if constexpr (std::is_same_v<T, Typeless>)
                {
                    for (size_t i = 0; i < vector.typelessSize(); ++i)
                    {
                        if (derived().visitVectorValueBeginDerived(vector, descriptor, i))
                        {
                            visitTypeInternal<Const, T>(vector.typelessAt(i), descriptor.valueDescriptor());
                            derived().visitVectorValueEndDerived(vector, descriptor, i);
                        }
                    }
                }
                else
                {
                    size_t i = 0;

                    for (auto& value : vector)
                    {
                        if (derived().visitVectorValueBeginDerived(vector, descriptor, i))
                        {
                            if constexpr (std::is_same_v<T, bool_t>)
                            {
                                visitTypeInternal<Const, T>(reinterpret_cast<const_t<Const, T>&>(value));
                            }
                            else
                            {
                                visitTypeInternal<Const, T>(value);
                            }

                            derived().visitVectorValueEndDerived(vector, descriptor, i);
                        }

                        ++i;
                    }
                }

                decrementVisitingLevel<Const>();
                derived().visitVectorEndDerived(vector, descriptor);
            }
        }

        template <bool Const, typename T>
        void visitTypeInternal(const_t<Const, T>& value, const Descriptor<T>& descriptor)
        {
            if constexpr (std::is_same_v<T, Typeless>)
            {
                switch (descriptor.type())
                {
                    case Type::Vector: visitVectorInternal<Const, Typeless>(value.template to<Vector<>>(), static_cast<const VectorDescriptor&>(descriptor));
                        break;

                    case Type::Struct:
                    {
                        auto& instance = value.template to<Struct>();
                        visitStructInternal<Const, Struct>(instance, instance._validProperties());
                        break;
                    }

                    case Type::Enum:
                    {
                        const auto& enumDescriptor = static_cast<const EnumDescriptor<>&>(descriptor);
                        derived().visitEnumDerived(value, enumDescriptor);
                        break;
                    }

                    default:
                    {
                        switch (descriptor.type())
                        {
                            case Type::boolean: derived().visitFundamentalTypeDerived(value.template to<bool_t>(), descriptor.template to<Descriptor<bool_t>>());
                                break;
                            case Type::int8: derived().visitFundamentalTypeDerived(value.template to<int8_t>(), descriptor.template to<Descriptor<int8_t>>());
                                break;
                            case Type::uint8: derived().visitFundamentalTypeDerived(value.template to<uint8_t>(), descriptor.template to<Descriptor<uint8_t>>());
                                break;
                            case Type::int16: derived().visitFundamentalTypeDerived(value.template to<int16_t>(), descriptor.template to<Descriptor<int16_t>>());
                                break;
                            case Type::uint16: derived().visitFundamentalTypeDerived(value.template to<uint16_t>(), descriptor.template to<Descriptor<uint16_t>>());
                                break;
                            case Type::int32: derived().visitFundamentalTypeDerived(value.template to<int32_t>(), descriptor.template to<Descriptor<int32_t>>());
                                break;
                            case Type::uint32: derived().visitFundamentalTypeDerived(value.template to<uint32_t>(), descriptor.template to<Descriptor<uint32_t>>());
                                break;
                            case Type::int64: derived().visitFundamentalTypeDerived(value.template to<int64_t>(), descriptor.template to<Descriptor<int64_t>>());
                                break;
                            case Type::uint64: derived().visitFundamentalTypeDerived(value.template to<uint64_t>(), descriptor.template to<Descriptor<uint64_t>>());
                                break;
                            case Type::float32: derived().visitFundamentalTypeDerived(value.template to<float32_t>(), descriptor.template to<Descriptor<float32_t>>());
                                break;
                            case Type::float64: derived().visitFundamentalTypeDerived(value.template to<float64_t>(), descriptor.template to<Descriptor<float64_t>>());
                                break;
                            case Type::property_set: derived().visitFundamentalTypeDerived(value.template to<property_set_t>(), descriptor.template to<Descriptor<property_set_t>>());
                                break;

                            case Type::timepoint: derived().visitFundamentalTypeDerived(value.template to<timepoint_t>(), descriptor.template to<Descriptor<timepoint_t>>());
                                break;
                            case Type::steady_timepoint: derived().visitFundamentalTypeDerived(value.template to<steady_timepoint_t>(), descriptor.template to<Descriptor<steady_timepoint_t>>());
                                break;
                            case Type::duration: derived().visitFundamentalTypeDerived(value.template to<duration_t>(), descriptor.template to<Descriptor<duration_t>>());
                                break;

                            case Type::uuid: derived().visitFundamentalTypeDerived(value.template to<uuid_t>(), descriptor.template to<Descriptor<uuid_t>>());
                                break;
                            case Type::string: derived().visitFundamentalTypeDerived(value.template to<string_t>(), descriptor.template to<Descriptor<string_t>>());
                                break;

                            case Type::Vector:
                            case Type::Struct:
                            case Type::Enum:
                                /* do nothing */
                                break;
                        }
                    }
                }
            }
            else
            {
                if constexpr (std::is_base_of_v<Struct, T>)
                {
                    visitStructInternal<Const, T>(value, value._validProperties());
                }
                else if constexpr (std::is_base_of_v<Vector<>, T>)
                {
                    visitVectorInternal<Const, typename T::value_t>(value, descriptor);
                }
                else if constexpr (std::is_enum_v<T>)
                {
                    derived().template visitEnumDerived<T>(value, descriptor);
                }
                else
                {
                    derived().visitFundamentalTypeDerived(value, descriptor);
                }
            }
        }

        template <bool Const, typename T>
        void visitTypeInternal(const_t<Const, T>& value)
        {
            constexpr bool IsStruct = std::is_base_of_v<Struct, T>;
            constexpr bool HasDescriptor = has_descriptor_v<T>;
            static_assert(IsStruct || HasDescriptor);

            if constexpr (IsStruct)
            {
               visitStructInternal<Const, T>(value, value._validProperties());
            }
            else if (HasDescriptor)
            {
                visitTypeInternal<Const>(value, Descriptor<T>::Instance());
            }
        }

        template <bool Const, typename... Ts, std::enable_if_t<sizeof...(Ts) >= 2, int> = 0>
        void visitTypeInternal(const_t<Const, Ts>&... values)
        {
            if (derived().visitPackBeginDerived(values...))
            {
                auto visit_tuple_value = [this](auto& value, size_t index, size_t size)
                {
                    using value_t = std::decay_t<decltype(value)>;

                    if (derived().visitPackElementBeginDerived(value, index, size))
                    {
                        visitTypeInternal<Const, value_t>(value);
                        derived().visitPackElementEndDerived(value, index, size);
                    }
                };

                size_t i = 0;
                (visit_tuple_value(values, i++, sizeof...(Ts)), ...);

                derived().visitPackEndDerived(values...);
            }
        }

        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }

        size_t m_constVisitingLevel = 0;
        size_t m_nonConstVisitingLevel = 0;
    };

    template <>
    struct TypeVisitor<void> : TypeVisitor<TypeVisitor<void>>
    {
        TypeVisitor() = default;
        TypeVisitor(const TypeVisitor& other) = default;
        TypeVisitor(TypeVisitor&& other) = default;
        virtual ~TypeVisitor() = default;

        TypeVisitor& operator = (const TypeVisitor& rhs) = default;
        TypeVisitor& operator = (TypeVisitor&& rhs) = default;

    protected:

        friend TypeVisitor<TypeVisitor<void>>;

        virtual void visitConstBegin();
        virtual void visitConstEnd();
        virtual void visitMutableBegin();
        virtual void visitMutableEnd();

        virtual bool visitStructBegin(const Struct& instance, PropertySet& includedProperties);
        virtual bool visitStructBegin(Struct& instance, PropertySet& includedProperties);
        virtual void visitStructEnd(const Struct& instance, PropertySet includedProperties);
        virtual void visitStructEnd(Struct& instance, PropertySet includedProperties);

        virtual bool visitPropertyBegin(const ProxyProperty<>& property, bool first);
        virtual bool visitPropertyBegin(ProxyProperty<>& property, bool first);
        virtual void visitPropertyEnd(const ProxyProperty<>& property, bool first);
        virtual void visitPropertyEnd(ProxyProperty<>& property, bool first);

        virtual bool visitVectorBegin(const Vector<>& vector, const VectorDescriptor& descriptor);
        virtual bool visitVectorBegin(Vector<>& vector, const VectorDescriptor& descriptor);
        virtual bool visitVectorValueBegin(const Vector<>& vector, const VectorDescriptor& descriptor, size_t index);
        virtual bool visitVectorValueBegin(Vector<>& vector, const VectorDescriptor& descriptor, size_t index);
        virtual void visitVectorValueEnd(const Vector<>& vector, const VectorDescriptor& descriptor, size_t index);
        virtual void visitVectorValueEnd(Vector<>& vector, const VectorDescriptor& descriptor, size_t index);
        virtual void visitVectorEnd(const Vector<>& vector, const VectorDescriptor& descriptor);
        virtual void visitVectorEnd(Vector<>& vector, const VectorDescriptor& descriptor);

        virtual void visitEnum(const Typeless& value, const EnumDescriptor<>& descriptor);
        virtual void visitEnum(Typeless& value, const EnumDescriptor<>& descriptor);

        virtual bool visitFundamentalTypeBegin(const Typeless& value, const Descriptor<>& descriptor);
        virtual bool visitFundamentalTypeBegin(Typeless& value, const Descriptor<>& descriptor);

        virtual void visitFundamentalType(const bool_t& value);
        virtual void visitFundamentalType(bool_t& value);
        virtual void visitFundamentalType(const int8_t& value);
        virtual void visitFundamentalType(int8_t& value);
        virtual void visitFundamentalType(const uint8_t& value);
        virtual void visitFundamentalType(uint8_t& value);
        virtual void visitFundamentalType(const int16_t& value);
        virtual void visitFundamentalType(int16_t& value);
        virtual void visitFundamentalType(const uint16_t& value);
        virtual void visitFundamentalType(uint16_t& value);
        virtual void visitFundamentalType(const int32_t& value);
        virtual void visitFundamentalType(int32_t& value);
        virtual void visitFundamentalType(const uint32_t& value);
        virtual void visitFundamentalType(uint32_t& value);
        virtual void visitFundamentalType(const int64_t& value);
        virtual void visitFundamentalType(int64_t& value);
        virtual void visitFundamentalType(const uint64_t& value);
        virtual void visitFundamentalType(uint64_t& value);
        virtual void visitFundamentalType(const float32_t& value);
        virtual void visitFundamentalType(float32_t& value);
        virtual void visitFundamentalType(const float64_t& value);
        virtual void visitFundamentalType(float64_t& value);
        virtual void visitFundamentalType(const property_set_t& value);

        virtual void visitFundamentalType(property_set_t& value);
        virtual void visitFundamentalType(const timepoint_t& value);
        virtual void visitFundamentalType(timepoint_t& value);
        virtual void visitFundamentalType(const steady_timepoint_t& value);
        virtual void visitFundamentalType(steady_timepoint_t& value);
        virtual void visitFundamentalType(const duration_t& value);
        virtual void visitFundamentalType(duration_t& value);

        virtual void visitFundamentalType(const uuid_t& value);
        virtual void visitFundamentalType(uuid_t& value);
        virtual void visitFundamentalType(const string_t& value);
        virtual void visitFundamentalType(string_t& value);

        virtual void visitFundamentalTypeEnd(const Typeless& value, const Descriptor<>& descriptor);
        virtual void visitFundamentalTypeEnd(Typeless& value, const Descriptor<>& descriptor);

        template <bool Const>
        void visitBeginDerived()
        {
            if constexpr (Const)
            {
                visitConstBegin();
            }
            else
            {
                visitConstEnd();
            }
        }

        template <bool Const>
        void visitEndDerived()
        {
            if constexpr (Const)
            {
                visitMutableBegin();
            }
            else
            {
                visitMutableEnd();
            }
        }

        template <typename U>
        bool visitStructBeginDerived(U& instance, PropertySet& includedProperties)
        {
            return visitStructBegin(instance, includedProperties);
        }

        template <typename U>
        void visitStructEndDerived(U& instance, PropertySet includedProperties)
        {
            visitStructEnd(instance, includedProperties);
        }

        template <typename T, typename PropertyDerived>
        bool visitPropertyBeginDerived(const Property<T, PropertyDerived>& property, bool first)
        {
            if constexpr (std::is_same_v<PropertyDerived, ProxyProperty<T>>)
            {
                return visitPropertyBegin(static_cast<const PropertyDerived&>(property), first);
            }
            else
            {
                // TODO: avoid const cast
                const ProxyProperty<> proxyProperty{ Typeless::From(const_cast<T&>(property.storage())), property.descriptor() };
                return visitPropertyBegin(proxyProperty, first);
            }
        }

        template <typename T, typename PropertyDerived>
        bool visitPropertyBeginDerived(Property<T, PropertyDerived>& property, bool first)
        {
            if constexpr (std::is_same_v<PropertyDerived, ProxyProperty<T>>)
            {
                return visitPropertyBegin(static_cast<PropertyDerived&>(property), first);
            }
            else
            {
                ProxyProperty<> proxyProperty{ property };
                return visitPropertyBegin(proxyProperty, first);
            }
        }

        template <typename T, typename PropertyDerived>
        void visitPropertyEndDerived(const Property<T, PropertyDerived>& property, bool first)
        {
            if constexpr (std::is_same_v<PropertyDerived, ProxyProperty<T>>)
            {
                return visitPropertyEnd(static_cast<const PropertyDerived&>(property), first);
            }
            else
            {
                // TODO: avoid const cast
                const ProxyProperty<> proxyProperty{ Typeless::From(const_cast<T&>(property.storage())), property.descriptor() };
                return visitPropertyEnd(proxyProperty, first);
            }
        }

        template <typename T, typename PropertyDerived>
        void visitPropertyEndDerived(Property<T, PropertyDerived>& property, bool first)
        {
            if constexpr (std::is_same_v<PropertyDerived, ProxyProperty<T>>)
            {
                return visitPropertyEnd(static_cast<PropertyDerived&>(property), first);
            }
            else
            {
                ProxyProperty<> proxyProperty{ property };
                return visitPropertyEnd(proxyProperty, first);
            }
        }

        template <typename U>
        bool visitVectorBeginDerived(U& vector, const VectorDescriptor& descriptor)
        {
            return visitVectorBegin(vector, descriptor);
        }

        template <typename U>
        bool visitVectorValueBeginDerived(U& vector, const VectorDescriptor& descriptor, size_t index)
        {
            return visitVectorValueBegin(vector, descriptor, index);
        }

        template <typename U>
        void visitVectorValueEndDerived(U& vector, const VectorDescriptor& descriptor, size_t index)
        {
            visitVectorValueEnd(vector, descriptor, index);
        }

        template <typename U>
        void visitVectorEndDerived(U& vector, const VectorDescriptor& descriptor)
        {
           visitVectorEnd(vector, descriptor);
        }

        template <typename T, typename U, std::enable_if_t<std::is_same_v<T, std::remove_const_t<U>>, int> = 0>
        void visitEnumDerived(U& value, const EnumDescriptor<T>& descriptor)
        {
            if constexpr (std::is_same_v<std::decay_t<U>, Typeless>)
            {
                visitEnum(value, descriptor);
            }
            else
            {
                visitEnum(Typeless::From(value), descriptor);
            }
        }

        template <typename T, typename U, std::enable_if_t<std::is_same_v<T, std::remove_const_t<U>>, int> = 0>
        void visitFundamentalTypeDerived(U& value, const Descriptor<T>& descriptor)
        {
            auto& valueTypeless = [&value]() -> auto&
            {
                if constexpr (std::is_same_v<std::decay_t<U>, Typeless>)
                {
                    return value;
                }
                else
                {
                    return Typeless::From(value);
                }
            }();

            if (visitFundamentalTypeBegin(valueTypeless, descriptor))
            {
                visitFundamentalType(value);
                visitFundamentalTypeEnd(valueTypeless, descriptor);
            }
        }
    };
}
