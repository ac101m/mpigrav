#include "comm/Client.hpp"
using namespace boost::asio;
using ip::tcp;


// Constructor, passed open socket
Client::Client(std::shared_ptr<tcp::socket> socket) : socket(socket) {}


// Same as above, but with raw pointers
void Client::Update(Body const * const bodies, int const n) {
  this->socket->send(
    boost::asio::buffer(&n, sizeof(int)));            // Send number of bodies
  this->socket->send(
    boost::asio::buffer(bodies, n * sizeof(Body)));   // Then send body data
}


// Update buffer from vector too
void Client::Update(std::vector<Body> const& bodies) {
  this->Update(&bodies[0], bodies.size());
}
