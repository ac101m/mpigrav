#ifndef _MPIGRAV_SERVER_INDLUDED
#define _MPIGRAV_SERVER_INDLUDED

#include <vector>
#include <mutex>
#include <thread>
#include <memory>

#include <boost/asio.hpp>

#include <comm/Request.hpp>
#include <Body.hpp>


class Server {
  private:
    int port;

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
    void SendInt(std::shared_ptr<boost::asio::ip::tcp::socket>& socket, int i);
    void SendBodyData(std::shared_ptr<boost::asio::ip::tcp::socket>& socket);

  public:
    Server(int const port);
    void Start(void);

    void UpdateBodyData(std::vector<Body> const& bodies);
    bool UpdateRequired(void) {return this->updateRequired;}
};


#endif // _MPIGRAV_SERVER_INDLUDED
