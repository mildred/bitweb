#ifndef __BITWEB_SOCKS4A_SOCKS4A_H__
#define __BITWEB_SOCKS4A_SOCKS4A_H__

#include <boost/asio.hpp>

namespace socks4a {

void session(boost::asio::ip::tcp::tcp::socket sock);
void server(boost::asio::io_service& io_service, unsigned short port);

}

#endif
