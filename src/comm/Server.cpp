#include <comm/Server.hpp>

#include <iostream>
#include <memory>

using namespace boost::asio;
using ip::tcp;

#include <Body.hpp>


// Constructor, starts listening on port
Server::Server(int const port) {
  this->port = port;
}


// Start the server thread
void Server::Start(void) {
  this->connectionListenerThread =
    std::thread(&Server::ConnectionListenerMain, this);
}


// Client listener thread
void Server::ConnectionListenerMain(void) {
  try {
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
        std::thread(&Server::ClientResponderMain, this, socket));
    }
  } catch(const std::exception& e) {
    std::cout << "Connection listener error: " << e.what() << "\n";
  }
}


// Client responder main
void Server::ClientResponderMain(std::shared_ptr<tcp::socket> socket) {
  try {
    while(1) {
      switch(this->GetClientRequest(socket)) {
        case REQUEST_BODY_DATA:
          if(this->updateRequired) {
            this->SendBodyData(socket);
            this->updateRequired = false;
          } else {
            this->SendInt(socket, 0);
          }
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


request_t Server::GetClientRequest(std::shared_ptr<tcp::socket>& socket) {
  request_t request;
  read(*socket, buffer(&request, sizeof(request_t)));
  return request;
}


void Server::SendInt(std::shared_ptr<tcp::socket>& socket, int i) {
  write(*socket, buffer(&i, sizeof(int)));
}


void Server::SendBodyData(std::shared_ptr<tcp::socket>& socket) {

  // Create intermediate buffer so compute doesn't stall during transmission
  this->bodyDataMutex.lock();
  std::vector<Body> buf = this->bodies;
  this->bodyDataMutex.unlock();

  // Transmit body data
  this->SendInt(socket, buf.size());
  write(*socket, buffer(buf.data(), buf.size() * sizeof(Body)));
}


// Update internal body buffer
void Server::UpdateBodyData(std::vector<Body> const& bodies) {
  this->bodyDataMutex.lock();
  this->bodies = bodies;
  this->bodyDataMutex.unlock();
  this->updateRequired = true;
}
