#include <iostream>
#include <vector>
using namespace std;

#include <unistd.h>
#include <cmath>
#include "mpi.h"
#include "omp.h"

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
  opt.Add(Option("port", 'p', ARG_TYPE_INT,
                 "Port to listen for clients on",
                 {_MPIGRAV_DEFAULT_PORT}));
  opt.Add(Option("threadcount", 't', ARG_TYPE_INT,
                 "Number of threads to use for each instance",
                 {"1"}));
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
  for(int i = 0; i < n; i++) {
    body[i].pos.x = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    body[i].pos.y = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    body[i].pos.z = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    body[i].pos = Normalize(body[i].pos);
    body[i].m = 100000;
  }

  // Listen for incoming client connections
  ClientManager clients(opt.Get("port"));

  // Tell openmp how many threads to use
  int threadCount = opt.Get("threadcount");
  omp_set_num_threads(threadCount);

  // Limit number of iterations. Temporary
  unsigned iterationCount = 0;
  unsigned iterationMax = 10000;
  while(iterationCount++ < iterationMax) {

    // Update clients about simulation progress
    if(clients.UpdateRequired()) {
      clients.UpdateBodyData(body);
    }

    // For each body, sum forces
    #pragma omp parallel for
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
    #pragma omp parallel for
    for(int i = 0; i < n; i++) {
      body[i].v = body[i].v + ((f[i] / body[i].m) * dt);
      body[i].Update(dt);
    }
  }

  return 0;
}
