#include "comm/Client.hpp"
using namespace boost::asio;
using ip::tcp;

#include <iostream>


// Constructor attempts a connection
Client::Client(std::string const host, int const port) :
  socket(tcp::socket(this->ioService)) {

  std::cout << "Connecting to: " << host << ":" << port << "\n";
  this->socket.connect(tcp::endpoint(ip::address::from_string(host), port));
  socket.set_option(tcp::no_delay(true));

  this->done = false;
  this->signalListenerThread =
    std::thread(&Client::SignalListenerMain, this);
}


// Signal listener thread
void Client::SignalListenerMain(void) {
  while(!this->done) {
    switch(this->RecvSignal()) {
      case SIGNAL_TRANSMIT_BODY_DATA:
        this->RecvBodyData();
        break;
      case SIGNAL_CLIENT_DISCONNECT:
        this->done = true;
        break;
      default:
        std::cout << "Error, unrecognised signal from server, disconnecting\n";
        this->done = true;
        break;
    }
  }
}


// Send a request to the server
signal_t Client::RecvSignal(void) {
  signal_t sig;
  read(this->socket, buffer(&sig, sizeof(signal_t)));
  return sig;
}


// Get integer from server
int Client::RecvInt(void) {
  int i;
  read(this->socket, buffer(&i, sizeof(int)));
  return i;
}


// Get body data from server
void Client::RecvBodyData(void) {
  unsigned n = this->RecvInt();
  std::vector<Body> buf(n);
  read(this->socket, buffer(buf.data(), n * sizeof(Body)));

  // Update local buffer
  this->bodyDataMutex.lock();
  this->bodyData = buf;
  this->bodyDataMutex.unlock();
}


// Get data from server
std::vector<Body> Client::GetBodyData(void) {
  this->bodyDataMutex.lock();
  std::vector<Body> buf = this->bodyData;
  this->bodyDataMutex.unlock();
  return buf;
}
