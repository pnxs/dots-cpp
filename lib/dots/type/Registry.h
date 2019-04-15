#pragma once

#include "Descriptor.h"
#include "StandardTypes.h"
#include "ChronoDescriptor.h"
#include "VectorDescriptor.h"
#include "EnumDescriptor.h"
#include "UuidDescriptor.h"
#include "Vector.h"
#include "StructDescriptor.h"
#include "Enum.h"
#include <dots/functional/signal.h>

#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <utility>

#define ENABLE_IF(E) typename std::enable_if<E::value>::type

namespace dots
{
namespace type
{

template <class T>
struct is_base: public std::is_arithmetic<T> {};

template<>
struct is_base<std::string>: public std::true_type {};

template<>
struct is_base<dots::property_set>: public std::true_type {};

template<>
struct is_base<void*>: public std::true_type {};

template<class T>
struct is_chrono: public std::false_type{};

template<> struct is_chrono<Duration>: public std::true_type{};
template<> struct is_chrono<TimePoint>: public std::true_type{};
template<> struct is_chrono<SteadyTimePoint>: public std::true_type{};

template<class T>
struct is_vector: public std::false_type{};

template<class T>
struct is_vector<dots::Vector<T>> : public std::true_type{};

template<class T>
struct is_uuid: public std::false_type{};

template<> struct is_uuid<dots::uuid>: public std::true_type{};

template<class T>
struct is_struct: public std::integral_constant<bool, std::is_class<T>::value && !is_base<T>::value && !is_chrono<T>::value && !is_vector<T>::value && !is_uuid<T>::value> {};



template<class T>
inline const Descriptor* get(const std::string& dotsName, DotsType dotsType, const T* /*val*/ = 0, ENABLE_IF(is_base<T>)* = 0)
{
    static const StandardTypeDescriptor<T>* td = new StandardTypeDescriptor<T>(dotsName, dotsType);
    return td;
}

template<class T>
inline const Descriptor* get(const std::string& dotsName, const T* /*val*/ = 0, ENABLE_IF(is_chrono<T>)* = 0)
{
    static const ChronoDescriptor<T> *cd = new ChronoDescriptor<T>(dotsName);
    return cd;
}

template<class T>
inline const Descriptor* get(const std::string& dotsName, const T* /*val*/ = 0, ENABLE_IF(is_uuid<T>)* = 0)
{
    static const UuidDescriptor* d = new UuidDescriptor(dotsName);
    return d;
}

template<class T>
inline
const StructDescriptor* getDescriptor(const T* /*val*/ = 0, ENABLE_IF(is_struct<T>)* = 0)
{
    return &T::_Descriptor();
}

template <class T>
inline
const EnumDescriptor* getDescriptor(const T*/*val*/ = 0, std::enable_if_t<has_enum_type_v<T>, int>* = 0)
{
    return &enum_type_t<T>::Descriptor();
}

template <class T>
inline
const Descriptor* getDescriptor(const T*/*val*/ = 0, ENABLE_IF(is_base<T>)* = 0)
{
    return nullptr;
}

template <class T>
inline
const Descriptor* getDescriptor(const T*/*val*/ = 0, ENABLE_IF(is_vector<T>)* = 0)
{
    typename T::value_type* tp = nullptr;
    getDescriptor(tp);
    return nullptr;
}

template <class T>
inline
const Descriptor* getDescriptor(const T*/*val*/ = 0, ENABLE_IF(is_chrono<T>)* = 0)
{
    return nullptr;
}

template <class T>
inline
const Descriptor* getDescriptor(const T*/*val*/ = 0, ENABLE_IF(is_uuid<T>)* = 0)
{
    dots::uuid uuid;
    return get("uuid", &uuid);
}

/*!
 * This class manages (stores) DOTS descriptors.
 */
class Registry
{
public:
    typedef std::unordered_map<std::string, const Descriptor*> type_storage_t;

    Registry();
    ~Registry();

    const Descriptor* findDescriptor(const std::string& name) const;
    const StructDescriptor* findStructDescriptor(const std::string& name) const;

    static const Descriptor* fromWireName(const std::string& t);

    const type_storage_t& getTypes() { return m_types; }

    void checkPopulate();
    void clear();

    pnxs::Signal<void (const StructDescriptor*)> onNewStruct;
    pnxs::Signal<void (const Descriptor*)> onNewType;

    void insertType(const std::string& name, const Descriptor* descriptor);
    void removeType(const std::string& name);

private:
    void populate_standard_types();

    type_storage_t m_types;
    static std::atomic_flag m_populated;
};


}
}
