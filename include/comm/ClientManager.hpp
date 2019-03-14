#ifndef _MPIGRAV_CLIENTMANAGER_INDLUDED
#define _MPIGRAV_CLIENTMANAGER_INDLUDED

#include <vector>
#include <mutex>
#include <thread>
#include <memory>

#include <boost/asio.hpp>

#include <comm/Request.hpp>
#include <Body.hpp>


class ClientManager {
  private:
    int const port;

    std::vector<Body> bodies;
    std::mutex bodyDataMutex;
    bool updateRequired;

    std::thread connectionListenerThread;
    std::vector<std::thread> clientThreads;

//====[PRIVATE METHODS]======================================================//

    // Connnection listener thread
    void ConnectionListenerMain(void);

    // Client reponder thread, handles requests from clients
    void ClientResponderMain(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    request_t GetClientRequest(std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void SendBodyData(std::shared_ptr<boost::asio::ip::tcp::socket>& socket);

  public:
    ClientManager(int const port);

    void UpdateBodyData(std::vector<Body> const& bodies);
    bool UpdateRequired(void) {return this->updateRequired;}
};


#endif // _MPIGRAV_CLIENTMANAGER_INDLUDED
