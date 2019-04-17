#ifndef _MPIGRAV_CLIENT_INDLUDED
#define _MPIGRAV_CLIENT_INDLUDED

#include <string>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>

#include "Body.hpp"
#include "comm/Signal.hpp"


class Client {
  private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket socket;

    std::thread signalListenerThread;
    bool done;

    std::vector<Body> bodyData;
    std::mutex bodyDataMutex;

//=====[PRIVATE METHODS]=====================================================//

    // Internal listener thread functions
    void SignalListenerMain(void);
    signal_t RecvSignal(void);
    int RecvInt(void);
    void RecvBodyData(void);

  public:
    Client(std::string const host, int const port);
    std::vector<Body> GetBodyData(void);
};


#endif // _MPIGRAV_SERVER_INDLUDED
