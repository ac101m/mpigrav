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
  write(this->socket, buffer(&request, sizeof(request_t)));
}


// Get data from server
std::vector<Body> Server::GetBodyData(void) {
  this->SendRequest(REQUEST_BODY_DATA);

  unsigned n;
  read(this->socket, buffer(&n, sizeof(unsigned)));
  if(n) {
    std::vector<Body> buf(n);
    read(this->socket, buffer(buf.data(), n * sizeof(Body)));
    this->bodies = buf;
  }

  return this->bodies;
}
