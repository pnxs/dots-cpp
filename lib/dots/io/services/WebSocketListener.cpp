#include "WebSocketListener.h"
#include <algorithm>
#include <dots/io/services/WebSocketChannel.h>

namespace dots
{
	WebSocketListener::WebSocketListener(boost::asio::io_context& ioContext, uint16_t port)
	{
		m_wsServer.init_asio(&ioContext);
        m_wsServer.set_access_channels(websocketpp::log::alevel::none);
        m_wsServer.set_error_channels(websocketpp::log::elevel::none);
        m_wsServer.set_reuse_addr(true);
        m_wsServer.listen(port);
	}

	void WebSocketListener::asyncAcceptImpl()
	{
		m_wsServer.set_validate_handler([this](websocketpp::connection_hdl hdl)
		{
			ws_connection_ptr_t connection = m_wsServer.get_con_from_hdl(hdl);
            const std::vector<std::string>& subProtocols = connection->get_requested_subprotocols();
            auto it = std::find(subProtocols.begin(), subProtocols.end(), "dots");

            if (it == subProtocols.end())
            {
                return false;
            }
            else
            {
                connection->select_subprotocol(*it);
                connection->set_open_handler([this, connection](ws_connection_hdl_t /*hdl*/)
                {
                    processAccept(std::make_shared<WebSocketChannel>(connection));
                });

                return true;
            }
		});

        m_wsServer.start_accept();
	}
}