#include <iostream>
#include <unistd.h>
#include <GLT/Window.hpp>

int main(void) {
  std::cout << "I am the viewer app\n";
  GLT::Window window(640, 480, "viewer");
  usleep(1000000);
  return 0;
}
