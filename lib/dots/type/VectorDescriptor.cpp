#include "VectorDescriptor.h"
#include "Registry.h"

namespace dots
{
namespace type
{

void VectorDescriptor::construct(void* obj) const
{
    new(obj) dots::vector_base;
}

void VectorDescriptor::destruct(void* obj) const
{
    clear(obj);

    const dots::vector_base* base = (const dots::vector_base*)obj;
    if (base->start() != 0)
    {
        ::operator delete(base->start());
    }
}

bool VectorDescriptor::usesDynamicMemory() const
{
    return true;
}

size_t VectorDescriptor::dynamicMemoryUsage(const void* lhs) const
{
    size_t size = get_size(lhs);
    size_t dynMemUsage = size * m_valueTypeDescriptor->sizeOf();

    for (size_t i = 0; i < size; ++i)
    {
        dynMemUsage += m_valueTypeDescriptor->dynamicMemoryUsage(get_data(lhs, i));
    }

    return dynMemUsage;   
}

std::string VectorDescriptor::to_string(const void */*lhs*/) const
{
    return std::string();
}

bool VectorDescriptor::from_string(void */*lhs*/, const std::string &/*str*/) const
{
    return false;
}

bool VectorDescriptor::equal(const void *lhs, const void *rhs) const
{
    auto size_lhs = get_size(lhs);
    auto size_rhs = get_size(rhs);

    if (size_lhs != size_rhs)
    {
        return false;
    }

    if (size_lhs < size_rhs)
    {
        for (std::size_t i = 0; i < size_lhs; ++i)
        {
            if (not m_valueTypeDescriptor->equal(get_data(lhs, i), get_data(rhs, i)))
            {
                return false;
            }
        }
    } else
    {
        for (std::size_t i = 0; i < size_rhs; ++i)
        {
            if (not m_valueTypeDescriptor->equal(get_data(lhs, i), get_data(rhs, i)))
            {
                return false;
            }
        }
    }

    return true;
}

bool VectorDescriptor::lessThan(const void *lhs, const void *rhs) const
{
    auto size_lhs = get_size(lhs);
    auto size_rhs = get_size(rhs);

    for (std::size_t i = 0; i < size_lhs && i < size_rhs; ++i)
    {
        if (m_valueTypeDescriptor->lessThan(get_data(lhs, i), get_data(rhs, i)))
        {
            return true;
        }

        if (m_valueTypeDescriptor->lessThan(get_data(rhs, i), get_data(lhs, i)))
        {
            return false;
        }
    }

    return size_lhs < size_rhs;
}

void VectorDescriptor::copy(void *lhs, const void *rhs) const
{
    auto size = get_size(rhs);

    resize(lhs, size);

    for (size_t i = 0; i < size; ++i)
    {
        m_valueTypeDescriptor->copy(get_data(lhs, i), get_data(rhs, i));
    }
}

void VectorDescriptor::swap(void *lhs, void *rhs) const
{
    auto base_lhs = reinterpret_cast<dots::vector_base*>(lhs);
    auto base_rhs = reinterpret_cast<dots::vector_base*>(rhs);

    base_lhs->swap(*base_rhs);
}

VectorDescriptor::VectorDescriptor(const Descriptor* vtd)
        :Descriptor("vector<" + vtd->name() + ">", DotsType::Vector, sizeof(dots::vector_base), alignof(dots::vector_base))
        ,m_valueTypeDescriptor(vtd)
{
    //printf("Construct VectorDescriptor: name=%s <-> dotsName=%s\n", name().c_str(), name().c_str());
}

const VectorDescriptor* VectorDescriptor::createDescriptor(const string& vectorTypeName)
{
    auto valueTypeDescriptor = Registry::fromWireName(vectorTypeName);
    if (valueTypeDescriptor == nullptr)
    {
        throw std::runtime_error("missing dots-type:" + vectorTypeName);
    }

    const VectorDescriptor* newVector = toVectorDescriptor(Descriptor::registry().findDescriptor("vector<" + vectorTypeName + ">"));
    if (not newVector) {
        newVector = new VectorDescriptor(valueTypeDescriptor);
    }
    return newVector;
}

void VectorDescriptor::clear(void *obj) const
{
    resize(obj, 0);
}

size_t VectorDescriptor::get_size(const void *obj) const
{
    auto base = reinterpret_cast<const dots::vector_base*>(obj);
    return (base->finish() - base->start()) / m_valueTypeDescriptor->sizeOf();
}

char * VectorDescriptor::get_data(const void *obj, size_t idx) const
{
    auto base = reinterpret_cast<const dots::vector_base*>(obj);
    return base->start() + idx * m_valueTypeDescriptor->sizeOf();
}

void VectorDescriptor::resize(const void *obj, size_t newSize) const
{
    auto base = (dots::vector_base*)(obj);
    const size_t  oldSize = get_size(obj);

    if (newSize > oldSize)
    {
        const size_t sizeOfElement = m_valueTypeDescriptor->sizeOf();
        size_t difference = newSize - oldSize;
        size_t availReserved = (base->end_of_storage() - base->finish()) / sizeOfElement;

        if (availReserved >= difference)
        {
            // No need to allocate more memory
            for (size_t i = oldSize; i < newSize; ++i)
            {
                // Construct new empty elements
                m_valueTypeDescriptor->construct(get_data(obj, i));
            }
            base->_M_finish = get_data(obj, newSize);
        } else {
            // More memory needed
            size_t length = std::max(2 * oldSize, newSize);
            char* newStart = (char*) ::operator new(length * sizeOfElement);

            // First construct elements in new memory
            for (size_t i = 0; i < newSize; ++i)
            {
                m_valueTypeDescriptor->construct(newStart + i * sizeOfElement);
            }

            // Second: copy data from old memory to new memory
            for (size_t i = 0; i < oldSize; ++i)
            {
                m_valueTypeDescriptor->copy(newStart + i * sizeOfElement, get_data(obj, i));
            }

            // Third: deconstruct elements from old memory
            for (size_t i = 0; i < oldSize; ++i)
            {
                m_valueTypeDescriptor->destruct(get_data(obj, i));
            }
            ::operator delete(base->start());

            base->_M_start = newStart;
            base->_M_finish = base->start() + newSize * sizeOfElement;
            base->_M_end_of_storage = base->start() + length * sizeOfElement;
        }

    }
    else if (newSize < oldSize)
    {
        for (size_t i = newSize; i < oldSize; ++i)
        {
            m_valueTypeDescriptor->destruct(get_data(obj, i));
        }
        base->_M_finish = get_data(obj, newSize);
    }
}

const Descriptor *VectorDescriptor::vtd() const
{
    return m_valueTypeDescriptor;
}


}
}