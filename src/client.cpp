#include <iostream>
#include <string>
#include <sstream>

#include <unistd.h>

#include <GLT/Window.hpp>
#include <GLT/Mesh.hpp>
#include <optparse.hpp>

#include "Master.hpp"
#include "comm/Server.hpp"
#include "draw/Draw.hpp"


void AddOptions(OptionParser& opt) {
  opt.Add(Option("host", 'h', ARG_TYPE_STRING,
                 "IP address of host to connect to",
                 {"127.0.0.1"}));
  opt.Add(Option("port", 'p', ARG_TYPE_INT,
                 "Port to use for connection",
                 {_MPIGRAV_DEFAULT_PORT}));
}


int main(int argc, char **argv) {
  OptionParser opt(argc, argv, "mpigrav viewer application");
  AddOptions(opt);

  // Connect to the server
  std::string host = opt.Get("host");
  int port = opt.Get("port");
  Server server(host, port);

  // Open a window with title indicating the host
  std::stringstream ss;
  ss << host << ":" << port;
  GLT::Window window(1024, 768, ss.str());
  window.EnableFpsCounter();
  window.camera.SetPos(0, 0, -3);
  glfwSwapInterval(0);

  // Build shaders
  GLT::ShaderProgram bodyShader = BuildBodyShader();

  // Loop forever (for now)
  bool done = false;
  while(!done) {
    if(window.KeyPressed(GLFW_KEY_ESCAPE)) done = true;
    std::vector<Body> bodies = server.GetData();
    GLT::Mesh bodyMesh = MakeMeshFromBodyList(bodies);
    glm::mat4 m = glm::mat4(1.0f);
    window.Draw(bodyMesh, bodyShader, m);
    window.Refresh();
  }

  return 0;
}
