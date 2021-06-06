#include <dots/Event.h>

namespace dots
{
    Event<type::Struct>::Event(const DotsHeader& header, const type::Struct& transmitted, const type::Struct& updated, const DotsCloneInformation& cloneInfo, std::optional<DotsMt> mt/* = std::nullopt*/) :
        m_header(header),
        m_transmitted(transmitted),
        m_updated(updated),
        m_cloneInfo(cloneInfo),
        m_mt(mt == std::nullopt ? m_cloneInfo.lastOperation : *mt)
    {
        /* do nothing */
    }

    /**
    * @return contained DOTS object
    */
    const type::Struct& Event<type::Struct>::operator () () const
    {
        return m_updated;
    }

    const DotsHeader& Event<type::Struct>::header() const
    {
        return m_header;
    }

    const type::Struct& Event<type::Struct>::transmitted() const
    {
        return m_transmitted;
    }

    const type::Struct& Event<type::Struct>::updated() const
    {
        return m_updated;
    }

    const DotsCloneInformation& Event<type::Struct>::cloneInfo() const
    {
        return m_cloneInfo;
    }

    const type::StructDescriptor<>& Event<type::Struct>::descriptor() const
    {
        return m_updated._descriptor();
    }

    DotsMt Event<type::Struct>::mt() const
    {
        return m_mt;
    }

    bool Event<type::Struct>::isCreate() const
    {
        return mt() == DotsMt::create;
    }

    bool Event<type::Struct>::isUpdate() const
    {
        return mt() == DotsMt::update;
    }

    bool Event<type::Struct>::isRemove() const
    {
        return mt() == DotsMt::remove;
    }

    bool Event<type::Struct>::isFromMyself() const
    {
        return m_header.isFromMyself == true;
    }
}