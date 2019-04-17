#include <comm/Server.hpp>

#include <iostream>
#include <memory>
#include <chrono>

using namespace boost::asio;
using ip::tcp;

#include <Body.hpp>


Server::Server(std::vector<Body> const& bodyData) {
  this->bodyDataMutex.lock();
  this->bodyData = bodyData;
  this->bodyDataMutex.unlock();
  this->done = false;
}


// Start the server
void Server::Start(int const port, double const updateFrequency) {
  this->port = port;
  this->updateFrequency = updateFrequency;

  // Connection listener thread
  this->connectionListenerThread =
    std::thread(&Server::ConnectionListenerMain, this);

  // Transmit thread
  this->clientUpdateThread =
    std::thread(&Server::ClientUpdateMain, this);
}


// Client listener thread
void Server::ConnectionListenerMain(void) {
  try {
    io_service ioService;
    ip::tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), this->port));

    std::cout << "Listening for connections on port: " << this->port << "\n";
    while(!this->done) {

      // Wait for someone to connect
      std::shared_ptr<tcp::socket> socket(new tcp::socket(ioService));
      acceptor.accept(*socket);
      socket->set_option(tcp::no_delay(true));
      std::cout << "Client connected!\n";

      // Create a new client thread
      this->socketListMutex.lock();
      this->sockets.push_back(socket);
      this->socketListMutex.unlock();
    }
  } catch(const std::exception& e) {
    std::cout << "Connection listener error: " << e.what() << "\n";
  }
}


void Server::SendSignal(
  std::shared_ptr<ip::tcp::socket> socket,
  signal_t sig) {

  write(*socket, buffer(&sig, sizeof(signal_t)));
}


void Server::SendInt(
  std::shared_ptr<boost::asio::ip::tcp::socket> socket,
  int i) {

  write(*socket, buffer(&i, sizeof(int)));
}


void Server::SendBodyData(
  std::shared_ptr<ip::tcp::socket> socket,
  std::vector<Body> const& buf) {

  this->SendSignal(socket, SIGNAL_TRANSMIT_BODY_DATA);
  this->SendInt(socket, buf.size());
  write(*socket, buffer(buf.data(), buf.size() * sizeof(Body)));
}


// Update connected clients, return true on success
void Server::UpdateClients(std::vector<Body> const& buf) {
  this->socketListMutex.lock();
  auto i = this->sockets.cbegin();
  while(i != this->sockets.cend()) {
    try {
      this->SendBodyData(*i, buf);
      i++;
    } catch(std::exception& e) {
      std::cout << "Client socket error, disconnecting\n";
      this->sockets.erase(i++);
    }
  }
  this->socketListMutex.unlock();
}


void Server::ClientUpdateMain(void) {
  using namespace std::chrono;
  while(!this->done) {
    high_resolution_clock::time_point tStart = high_resolution_clock::now();

    this->bodyDataMutex.lock();
    std::vector<Body> buf = this->bodyData;
    this->bodyDataMutex.unlock();
    this->UpdateClients(buf);

    std::this_thread::sleep_until(
      duration<double>(1 / this->updateFrequency) + tStart);
  }
}


// Set data to send to clients
void Server::SetBodyData(std::vector<Body> const& bodyData) {
  this->bodyDataMutex.lock();
  this->bodyData = bodyData;
  this->bodyDataMutex.unlock();
}
