#include "Message.h"

namespace dots {

Message::Message(const DotsTransportHeader &header, const std::vector<uint8_t> &data)
:m_header(header)
,m_data(data)
{

}

const DotsTransportHeader &Message::header() const
{
    return m_header;
}

const std::vector<uint8_t> &Message::data() const
{
    return m_data;
}

}