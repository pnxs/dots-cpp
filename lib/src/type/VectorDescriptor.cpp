#include <dots/type/VectorDescriptor.h>

namespace dots::type
{
    Descriptor<Vector<Typeless>>::Descriptor(key_t key, std::string name, Descriptor<>& valueDescriptor, size_t size, size_t alignment):
        Descriptor<Typeless>(key, Type::Vector, std::move(name), size, alignment),
        m_valueDescriptor(valueDescriptor.shared_from_this())
    {
        /* do nothing */
    }

    const Descriptor<Typeless>& Descriptor<Vector<Typeless>>::valueDescriptor() const
    {
        return *m_valueDescriptor;
    }

    Descriptor<Typeless>& Descriptor<Vector<Typeless>>::valueDescriptor()
    {
        return *m_valueDescriptor;
    }
}
