#ifndef _MPIGRAV_CLIENT_INDLUDED
#define _MPIGRAV_CLIENT_INDLUDED

#include <memory>

#include <boost/asio.hpp>

#include <Body.hpp>


class Client {
  private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;

  public:
    Client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void Update(Body const * const bodies, int n);
    void Update(std::vector<Body> const& bodies);
};


#endif // _MPIGRAV_CLIENT_INDLUDED
