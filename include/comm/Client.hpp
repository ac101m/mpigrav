#ifndef _MPIGRAV_CLIENT_INDLUDED
#define _MPIGRAV_CLIENT_INDLUDED

#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include "Body.hpp"
#include "comm/Request.hpp"


class Client {
  private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::vector<Body>& bodies;
    std::thread connectionHandlerThread;

//====[PRIVATE METHODS]======================================================//

    void RequestHandler(void);
    request_t GetRequest(void);
    void SendBodyData(void);

  public:
    Client(std::shared_ptr<boost::asio::ip::tcp::socket> const socket,
           std::vector<Body>& bodies);
};


#endif // _MPIGRAV_CLIENT_INDLUDED
