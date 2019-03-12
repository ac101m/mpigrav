#ifndef _MPIGRAV_SERVER_INDLUDED
#define _MPIGRAV_SERVER_INDLUDED

#include <string>

#include <boost/asio.hpp>

#include "Body.hpp"


class Server {
  private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket socket;

  public:
    Server(std::string const host, int const port);
    std::vector<Body> GetData(void);
};


#endif // _MPIGRAV_SERVER_INDLUDED
