#include <iostream>
#include <vector>
using namespace std;

#include <optparse.hpp>
#include <unistd.h>
#include <math.h>

#include "mpi.h"

#include "Body.hpp"


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
                 "Damping coefficient to stop infinities",
                 {"1"}));
  opt.Add(Option("framerate", 'f', ARG_TYPE_FLOAT,
                 "How many frames per second to send to clients",
                 {"60"}));
}


int main(int argc, char **argv) {
  OptionParser opt(argc, argv, "mpigrav compute engine");
  AddOptions(opt);

  int n = opt.Get("nbodies");
  double G = opt.Get("gravitation");
  double dt = opt.Get("timestep");
  double d = opt.Get("damping");

  Body<double>* body = new Body<double>[n];   // Bodies
  Vec3<double>* f = new Vec3<double>[n];      // Total force on each body

  // Set some initial body positions
  body[0].pos = Vec3<double>(-5, 0, 0);
  body[0].m = 10000;
  body[1].pos = Vec3<double>(5, 0, 0);
  body[1].m = 10000;

  // Loop forever (for now)
  while(1) {

    // Compute gravitation on each body
    for(int i = 0; i < n; i++) {
      f[i].x = f[i].y = f[i].z = 0;

      // For each other body add force (G*m1*m2/r^2)
      for(int j = 0; j < n; j++) if(i != j) {
        Vec3<double> dij = body[j].pos - body[i].pos;
        double r2 = (dij.x * dij.x) + (dij.y * dij.y) + (dij.z * dij.z);
        double force = (G * body[i].m * body[j].m) / (r2 + d);
        double r = sqrt(r2);
        f[i].x += force * dij.x / r;
        f[i].y += force * dij.y / r;
        f[i].z += force * dij.z / r;
      }

      // Adjust body acceleration
      Vec3<double> a = f[i] / body[i].m;
      body[i].v = body[i].v + (a * dt);
    }

    // Update body positions
    for(int i = 0; i < n; i++) {
      body[i].Update(dt);
    }
  }

  delete [] body;
  delete [] f;
  return 0;
}
