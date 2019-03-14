#include "comm/Server.hpp"
using namespace boost::asio;
using ip::tcp;

#include <iostream>


// Constructor attempts a connection
Server::Server(std::string const host, int const port) :
  socket(tcp::socket(this->ioService)) {

  std::cout << "Connecting to: " << host << ":" << port << "\n";
  this->socket.connect(tcp::endpoint(ip::address::from_string(host), port));
  socket.set_option(tcp::no_delay(true));
}


// Send a request to the server
void Server::SendRequest(request_t request) {
  this->socket.send(buffer(&request, sizeof(request_t)));
}


// Get data from server
std::vector<Body> Server::GetBodyData(void) {
  this->SendRequest(REQUEST_BODY_DATA);

  unsigned n = 2;
  this->socket.receive(buffer(&n, sizeof(unsigned)));
  std::vector<Body> bodies(n);
  this->socket.receive(buffer(bodies.data(), n * sizeof(Body)));

  return bodies;
}
