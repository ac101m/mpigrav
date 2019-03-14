#include "comm/Client.hpp"
using namespace boost::asio;
using ip::tcp;

#include <iostream>


// Constructor, passed open socket
Client::Client(std::shared_ptr<tcp::socket> const socket,
               std::vector<Body>& bodies) :
               socket(socket), bodies(bodies) {

  // Start the client request handler thread
  this->connectionHandlerThread =
    std::thread(&Client::RequestHandler, this);
  //this->RequestHandler();
}


// Handle requests from the client
void Client::RequestHandler(void) {
  while(1) {
    switch(this->GetRequest()) {
      case REQUEST_BODY_DATA:
        std::cout << "Body data requested\n";
        this->SendBodyData();
        break;
      default:
        std::cout << "Warning, unrecognised request ignored\n";
        break;
    }
  }
}


// Wait for an incoming request
request_t Client::GetRequest(void) {
  request_t request;
  this->socket->receive(buffer(&request, sizeof(request_t)));
  return request;
}


// Same as above, but with raw pointers
void Client::SendBodyData(void) {
  unsigned n = this->bodies.size();
  std::cout << "Sending " << n << " bodies\n";
  this->socket->send(boost::asio::buffer(&n, sizeof(unsigned)));
  Body* bodyData = &this->bodies[0];
  this->socket->send(boost::asio::buffer(bodyData, n * sizeof(Body)));
}
