#include <iostream>
#include <iterator>
#include <boost/program_options.hpp>
#include "socks4a/socks4a.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  try {

    int portnum = -1;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "produce help message")
      ("port,p", po::value<int>(&portnum)->default_value(8878), "set port number");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    }

    cout << "Port number is " << portnum << ".\n";
    
    boost::asio::io_service io_service;
    socks4a::server(io_service, portnum);
    io_service.run();
  }
  catch(exception& e) {
    cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch(...) {
    cerr << "Exception of unknown type!\n";
    return 1;
  }
  return 0;
}
