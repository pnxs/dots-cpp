#include <dots/io/Event.h>

namespace dots
{
    Event<type::NewStruct>::Event(const DotsHeader& header, const type::NewStruct& transmitted, const type::NewStruct& updated, const DotsCloneInformation& cloneInfo) :
        m_header(header),
        m_transmitted(transmitted),
        m_updated(updated),
        m_cloneInfo(cloneInfo)
    {
        /* do nothing */
    }

    /**
    * @return contained DOTS object
    */
    const type::NewStruct& Event<type::NewStruct>::operator () () const 
    {
        return m_updated; 
    }

    const DotsHeader& Event<type::NewStruct>::header() const
    {
        return m_header;
    }

    const type::NewStruct& Event<type::NewStruct>::transmitted() const
    {
        return m_transmitted;
    }

    const type::NewStruct& Event<type::NewStruct>::updated() const
    {
        return m_updated;
    }

    const DotsCloneInformation& Event<type::NewStruct>::cloneInfo() const
    {
        return m_cloneInfo; 
    }

    const type::NewStructDescriptor<>& Event<type::NewStruct>::descriptor() const
    {
	    return m_updated._descriptor();
    }

    DotsMt Event<type::NewStruct>::mt() const
    {
        return m_cloneInfo.lastOperation;
    }

    bool Event<type::NewStruct>::isCreate() const
    {
        return mt() == DotsMt::create; 
    }

    bool Event<type::NewStruct>::isUpdate() const 
    {
        return mt() == DotsMt::update;
    }

    bool Event<type::NewStruct>::isRemove() const
    { 
        return mt() == DotsMt::remove;
    }

    bool Event<type::NewStruct>::isOwnUpdate() const
    {
        return m_header.isFromMyself; 
    }
}