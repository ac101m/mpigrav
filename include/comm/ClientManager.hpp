#ifndef _MPIGRAV_CLIENTMANAGER_INDLUDED
#define _MPIGRAV_CLIENTMANAGER_INDLUDED

#include <boost/asio.hpp>

#include <comm/Client.hpp>
#include <Body.hpp>


class ClientManager {
  private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor;

    std::vector<Client> clients;
    std::vector<Body> bodies;

  public:
    ClientManager(int const port);
    void Update(std::vector<Body> const& bodies);
    void Update(Body const * const bodies, int const n);
};


#endif // _MPIGRAV_CLIENTMANAGER_INDLUDED
