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

#include <unistd.h>


void AddOptions(OptionParser& opt) {
  opt.Add(Option("address", 'a', ARG_TYPE_STRING,
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
  std::string address = opt.Get("address");
  int port = opt.Get("port");
  Server server(address, port);

  // Set up a window for drawing
  std::stringstream ss;
  ss << address << ":" << port;
  GLT::Window window(1024, 768, ss.str());
  window.EnableFpsCounter();
  window.camera.SetPos(0, 0, -3);

  // Build shaders
  GLT::ShaderProgram bodyShader = BuildBodyShader();


//====[TEMPORARY]============================================================//
  // Input sensitifity stuff
  float rotateSpeed = 1.0f;
	float moveSpeed = 1.0f;
	float mouseSensitivity = 0.003f;
  float dFwd, dRight, dUp, dr = 0.0f;
//====[TEMPORARY]============================================================//


  // Loop forever (for now)1
  while(!window.ShouldClose()) {


//====[TEMPORARY]============================================================//
    // get current time
    float dt = window.GetTimeDelta();

    // Cursor capture control
    glm::vec2 cursorDelta = window.GetCursorDelta() * mouseSensitivity;
    if(window.KeyPressed(GLFW_KEY_ESCAPE)) window.FreeCursor();
    if(window.KeyPressed(GLFW_KEY_M)) window.CaptureCursor();

    // Camera translation & rotation
    dr = dFwd = dRight = dUp = 0.0f;
    if(window.KeyPressed(GLFW_KEY_W)) dFwd += (dt * moveSpeed);
    if(window.KeyPressed(GLFW_KEY_S)) dFwd -= (dt * moveSpeed);
    if(window.KeyPressed(GLFW_KEY_A)) dRight += (dt * moveSpeed);
    if(window.KeyPressed(GLFW_KEY_D)) dRight -= (dt * moveSpeed);
    if(window.KeyPressed(GLFW_KEY_SPACE)) dUp += (dt * moveSpeed);
    if(window.KeyPressed(GLFW_KEY_C)) dUp -= (dt * moveSpeed);
    if(window.KeyPressed(GLFW_KEY_E)) dr += (dt * rotateSpeed);
    if(window.KeyPressed(GLFW_KEY_Q)) dr -= (dt * rotateSpeed);

    // Update window camera
    window.camera.Move(dRight, dUp, dFwd);
    window.camera.MoveLook(-cursorDelta.x, cursorDelta.y, dr);
//====[TEMPORARY]============================================================//


    // Get body data and draw
    std::vector<Body> bodies = server.GetBodyData();
    GLT::Mesh bodyMesh = MakeMeshFromBodyList(bodies);
    glm::mat4 m = glm::mat4(1.0f);
    window.Draw(bodyMesh, bodyShader, m);
    window.Refresh();
  }

  return 0;
}
