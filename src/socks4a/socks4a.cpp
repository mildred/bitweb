#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>

#include "socks4a.h"

namespace socks4a {

const int max_length = 1024;

void session(boost::asio::ip::tcp::tcp::socket sock)
{
  try
  {
    boost::system::error_code error;
    size_t length;

    uint8_t  vn, cd;
    uint16_t dstport;
    uint32_t dstip;
    char userid[max_length];
    char domain[max_length];

    length = boost::asio::read(sock, boost::asio::buffer(&vn, 1));
    length = boost::asio::read(sock, boost::asio::buffer(&cd, 1));
    length = boost::asio::read(sock, boost::asio::buffer(&dstport, 1));
    length = boost::asio::read(sock, boost::asio::buffer(&dstip, 1));
    length = boost::asio::read_until(sock, boost::asio::buffer(&userid, max_length), 0);
    length = boost::asio::read_until(sock, boost::asio::buffer(&domain, max_length), 0);

    boost::asio::write(sock, boost::asio::buffer(data, length));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_service& io_service, unsigned short port)
{
  boost::asio::ip::tcp::tcp::acceptor a(io_service, boost::asio::ip::tcp::tcp::endpoint(boost::asio::ip::tcp::tcp::v4(), port));
  for (;;)
  {
    boost::asio::ip::tcp::tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}

}
