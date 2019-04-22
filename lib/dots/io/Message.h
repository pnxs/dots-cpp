#pragma once
#include <dots/cpp_config.h>
#include <DotsTransportHeader.dots.h>

namespace dots
{
	/*!
	 * Object, that holds a the data, needed for sending a DOTS-object to a client.
	 * It contains a DotsTransportHeader and the serialized payload.
	 */
	struct Message
	{
		Message(const DotsTransportHeader& header, const std::vector<uint8_t>& data);

		const DotsTransportHeader& header() const;
		const std::vector<uint8_t>& data()  const;

		bool operator == (const Message& rhs) const
		{
			if (rhs.m_header != m_header) return false;
			if (rhs.m_data != m_data) return false;
			return true;
		}		

	private:

		DotsTransportHeader m_header;
		const std::vector<uint8_t >& m_data;
		
	};
}