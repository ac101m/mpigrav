#ifndef _MPIGRAV_SERVER_INDLUDED
#define _MPIGRAV_SERVER_INDLUDED

#include <vector>
#include <list>
#include <mutex>
#include <thread>
#include <memory>

#include <boost/asio.hpp>

#include <comm/Signal.hpp>
#include <Body.hpp>


class Server {
  private:
    int port;
    double updateFrequency;

    std::thread clientUpdateThread;
    std::thread connectionListenerThread;
    bool done;

    std::vector<Body> bodyData;
    std::mutex bodyDataMutex;

    std::list<std::shared_ptr<boost::asio::ip::tcp::socket>> sockets;
    std::mutex socketListMutex;

//====[PRIVATE METHODS]======================================================//

    // Connnection listener thread
    void ConnectionListenerMain(void);

    // Transmit routines
    void SendSignal(
      std::shared_ptr<boost::asio::ip::tcp::socket> socket,
      signal_t const sig);
    void SendInt(
      std::shared_ptr<boost::asio::ip::tcp::socket> socket,
      int const i);
    void SendBodyData(
      std::shared_ptr<boost::asio::ip::tcp::socket> socket,
      std::vector<Body> const& buf);

    // Client update thread
    void ClientUpdateMain(void);

    // Update connected clients

  public:
    Server();
    Server(std::vector<Body> const& bodyData);
    void Start(int const port, double const updateFrequency);

    // Sets for various parameters
    void UpdateClients(std::vector<Body> const& bodies);
    void SetBodyData(std::vector<Body> const& bodyData);
};


#endif // _MPIGRAV_SERVER_INDLUDED
