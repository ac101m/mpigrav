#include <comm/ClientManager.hpp>

#include <iostream>
#include <memory>

using namespace boost::asio;
using ip::tcp;

#include <Body.hpp>

// Constructor, starts listening on port
ClientManager::ClientManager(int const port) :
  acceptor(this->ioService, tcp::endpoint(tcp::v4(), port)) {

  // Wait for someone to connect (temporary, we'll make this async later)
  std::cout << "Listening for clients on port: " << port << "\n";
  std::shared_ptr<tcp::socket> socket(new tcp::socket(this->ioService));
  this->acceptor.accept(*socket);

  // Client now managed lifetime of socket
  this->clients.push_back(Client(socket));
}


// Same as above, but with raw pointers
void ClientManager::Update(Body const * const bodies, int const n) {
  if(!this->clients.size()) return;

  // Copy body data to an internal buffer
  this->bodies.clear();
  this->bodies.reserve(n);
  for(int i = 0; i < n; i++) {
    this->bodies.push_back(bodies[i]);
  }

  // Transmit to clients
  for(unsigned i = 0; i < this->clients.size(); i++) {
    clients[i].Update(bodies, n);
  }
}


// Update buffer from vector too
void ClientManager::Update(std::vector<Body> const& bodies) {
  this->Update(&bodies[0], bodies.size());
}
