#include "comm/Server.hpp"
using namespace boost::asio;
using ip::tcp;

#include <iostream>


// Constructor attempts a connection
Server::Server(std::string const host, int const port) :
  socket(tcp::socket(this->ioService)) {

  std::cout << "Connecting to: " << host << ":" << port << "\n";
  this->socket.connect(tcp::endpoint(ip::address::from_string(host), port));
}


// Get data from server
std::vector<Body> Server::GetData(void) {
  int n;
  this->socket.receive(buffer(&n, sizeof(int)));

  std::vector<Body> bodies(n);
  this->socket.receive(buffer(&bodies[0], n * sizeof(Body)));

  return bodies;
}
