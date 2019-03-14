#include <iostream>
#include <vector>
#include <unistd.h>
#include <math.h>
#include "mpi.h"
using namespace std;

#include <optparse.hpp>

#include "Master.hpp"
#include "Body.hpp"
#include "comm/ClientManager.hpp"


void AddOptions(OptionParser& opt) {
  opt.Add(Option("gravitation", 'g', ARG_TYPE_FLOAT,
                 "Gravitational constant for the simulation",
                 {"6.67408E-11"}));
  opt.Add(Option("nbodies", 'n', ARG_TYPE_INT,
                 "Number of bodies to use for the simulation",
                 {"2"}));
  opt.Add(Option("timestep", 'T', ARG_TYPE_FLOAT,
                 "Major time step in seconds",
                 {"1"}));
  opt.Add(Option("damping", 'd', ARG_TYPE_FLOAT,
                 "Damping coefficient to stop infinities making a mess of things",
                 {"1"}));
  opt.Add(Option("updaterate", 'f', ARG_TYPE_FLOAT,
                 "How many times per second to update connected clients",
                 {"60"}));
  opt.Add(Option("port", 'p', ARG_TYPE_INT,
                 "Port to listen for clients on",
                 {_MPIGRAV_DEFAULT_PORT}));
}


int main(int argc, char **argv) {
  OptionParser opt(argc, argv, "mpigrav compute engine");
  AddOptions(opt);

  int n = opt.Get("nbodies");
  fp_t G = opt.Get("gravitation");
  fp_t dt = opt.Get("timestep");
  fp_t d = opt.Get("damping");

  std::vector<Body> body(n);
  std::vector<Vec3> f(n);

  // Set some initial body positions
  body[0].pos = Vec3( -1, 0, 0); body[0].m = 100000;
  body[1].pos = Vec3(0.5, 0, 0); body[1].m = 200000;

  // Listen for incoming client connections
  ClientManager clientManager(opt.Get("port"));

  // Loop forever (for now)
  while(1) {

    // Update clients about simulation progress
    if(clientManager.UpdateRequired()) {
      clientManager.Update(body);
    }

    // For each body, sum forces
    for(int i = 0; i < n; i++) {
      f[i].x = f[i].y = f[i].z = 0;

      // For each other body add force (G*m1*m2/r^2)
      for(int j = 0; j < n; j++) if(i != j) {
        Vec3 dij = body[j].pos - body[i].pos;
        fp_t r2 = (dij.x * dij.x) + (dij.y * dij.y) + (dij.z * dij.z);
        fp_t force = (G * body[i].m * body[j].m) / (r2 + d);
        fp_t r = sqrt(r2);
        f[i].x += force * dij.x / r;
        f[i].y += force * dij.y / r;
        f[i].z += force * dij.z / r;
      }
    }

    // Update body positions and velocities
    for(int i = 0; i < n; i++) {
      body[i].v = body[i].v + ((f[i] / body[i].m) * dt);
      body[i].Update(dt);
    }
  }

  return 0;
}
