#ifndef _MPIGRAV_SERVER_INDLUDED
#define _MPIGRAV_SERVER_INDLUDED

#include <string>

#include <boost/asio.hpp>

#include "Body.hpp"
#include "comm/Request.hpp"


class Server {
  private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket socket;

    std::vector<Body> bodies;

//=====[PRIVATE METHODS]=====================================================//

    void SendRequest(request_t request);

  public:
    Server(std::string const host, int const port);
    std::vector<Body> GetBodyData(void);
};


#endif // _MPIGRAV_SERVER_INDLUDED
