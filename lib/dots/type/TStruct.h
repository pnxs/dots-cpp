#pragma once
#include <string_view>
#include <type_traits>
#include "StructDescriptor.h"
#include "Registry.h"
#include "Struct.h"

namespace dots::type
{
    template <typename Derived>
    struct TStruct : Struct
    {
        TStruct() : Struct(_GetStructDescriptor())
        {
	        /* do nothing */
        }
        TStruct(const TStruct& other) = default;
        TStruct(TStruct&& other) = default;
        ~TStruct() = default;

        TStruct& operator = (const TStruct& rhs) = default;
        TStruct& operator = (TStruct&& rhs) = default;

        static const StructDescriptor& _GetStructDescriptor()
        {
			static const StructDescriptor* structDescriptor = Descriptor::registry().findStructDescriptor(Derived::Description.name.data());

			if (structDescriptor == nullptr)
			{
				std::vector<PropertyDescription> propertyDescriptions = std::apply([](auto&&... args)
				{
					(type::getDescriptor<typename std::remove_reference_t<decltype(args)>::value_t>(nullptr), ...);

					return std::vector<PropertyDescription>{
						PropertyDescription{
							std::remove_reference_t<decltype(args)>::Description
						}... };
				}, typename Derived::properties_t{});
				
				structDescriptor = MakeStructDescriptor(Derived::Description);
			}

			return *structDescriptor;
        }

    protected:

		template <typename... PropertyDescriptions>
		static constexpr StructDescription MakeStructDescription(const std::string_view& name, uint8_t flags, PropertyDescriptions&&... propertyDescriptions)
		{
			return StructDescription{ name, flags, { std::forward<PropertyDescriptions>(propertyDescriptions)... }, sizeof...(PropertyDescriptions) };
		}
    };
}