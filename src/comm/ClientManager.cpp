#include <comm/ClientManager.hpp>

#include <iostream>
#include <memory>

using namespace boost::asio;
using ip::tcp;

#include <Body.hpp>


// Constructor, starts listening on port
ClientManager::ClientManager(int const port) : port(port) {
  this->connectionListenerThread =
    std::thread(&ClientManager::ConnectionListenerMain, this);
}


// Client listener thread
void ClientManager::ConnectionListenerMain(void) {
  io_service ioService;
  ip::tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), this->port));

  std::cout << "Listening for connections on port: " << this->port << "\n";
  while(1) {

    // Wait for someone to connect
    std::shared_ptr<tcp::socket> socket(new tcp::socket(ioService));
    acceptor.accept(*socket);
    socket->set_option(tcp::no_delay(true));
    std::cout << "Client connected!\n";

    // Create a new client thread
    this->clientThreads.push_back(
      std::thread(&ClientManager::ClientResponderMain, this, socket));
  }
}


// Client responder main
void ClientManager::ClientResponderMain(std::shared_ptr<tcp::socket> socket) {
  try {
    while(1) {
      switch(this->GetClientRequest(socket)) {
        case REQUEST_BODY_DATA:
          this->updateRequired = true;
          this->SendBodyData(socket);
          break;
        default:
          std::cout << "Warning, unrecognised request ignored\n";
          break;
      }
    }
  } catch(const std::exception& e) {
    std::cout << "Client disconnected: " << e.what() << "\n";
  }
}


request_t ClientManager::GetClientRequest(std::shared_ptr<tcp::socket>& socket) {
  request_t request;
  socket->receive(buffer(&request, sizeof(request_t)));
  return request;
}


void ClientManager::SendBodyData(std::shared_ptr<tcp::socket>& socket) {
  this->bodyDataMutex.lock();
  unsigned n = this->bodies.size();
  socket->send(boost::asio::buffer(&n, sizeof(unsigned)));
  socket->send(boost::asio::buffer(this->bodies.data(), n * sizeof(Body)));
  this->bodyDataMutex.unlock();
}


// Update internal buffer
void ClientManager::Update(Body const * const bodies, int const n) {
  this->bodyDataMutex.lock();

  // Copy body data to internal buffer
  this->updateRequired = false;
  this->bodies.clear();
  this->bodies.reserve(n);
  for(int i = 0; i < n; i++) {
    this->bodies.push_back(bodies[i]);
  }

  this->bodyDataMutex.unlock();
}


// Update buffer from vector
void ClientManager::Update(std::vector<Body> const& bodies) {
  this->Update(&bodies[0], bodies.size());
}
