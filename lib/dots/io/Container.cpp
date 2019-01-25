#include "Container.h"

namespace dots
{


ContainerBase::ContainerBase(const type::StructDescriptor *td)
:m_td(td)
{

}

const type::StructDescriptor *ContainerBase::td() const
{
    return m_td;
}

}