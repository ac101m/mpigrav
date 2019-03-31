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
  opt.Add(Option("iterationlimit", 'i', ARG_TYPE_INT,
                 "Limits the number of iterations to perform 0 = infinite",
                 {"0"}));
}


int main(int argc, char **argv) {
  OptionParser opt(argc, argv, "mpigrav compute engine");
  AddOptions(opt);

  int n = opt.Get("nbodies");
  fp_t G = opt.Get("gravitation");
  fp_t dt = opt.Get("timestep");
  fp_t d = opt.Get("damping");

  std::vector<Body> body(n);
  std::vector<Vec3> v(n);
  std::vector<Vec3> a(n);

  // Set some initial body positions
  for(int i = 0; i < n; i++) {
    body[i].r.x = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    body[i].r.y = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    body[i].r.z = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    body[i].r = Normalize(body[i].r);
    body[i].m = 100000;
  }

  // Listen for incoming client connections
  ClientManager clients(opt.Get("port"));

  // If custom thread count was supplied, override OMP_NUM_THREADS
  if(opt.Specified("threadcount")) {
    int threadCount = opt.Get("threadcount");
    omp_set_num_threads(threadCount);
  }

  // Limit number of iterations based on command line option
  int iterationCount = 0;
  int iterationLimit = opt.Get("iterationlimit");
  while(!iterationLimit || iterationCount < iterationLimit) {
    iterationCount++;

    // Update clients about simulation progress
    if(clients.UpdateRequired()) {
      clients.UpdateBodyData(body);
    }

    // For each body
    #pragma omp parallel for
    for(int i = 0; i < n; i++) {
      a[i].x = a[i].y = a[i].z = 0;

      // Compute acceletation due to other bodies
      for(int j = 0; j < n; j++) {
        if((i != j) && (body[j].r != body[i].r)) {
          Vec3 dr = body[j].r - body[i].r;
          fp_t r2 = (dr.x * dr.x) + (dr.y * dr.y) + (dr.z * dr.z);
          fp_t acceleration = body[j].m / (r2 + d);
          fp_t r = sqrt(r2);
          a[i].x += acceleration * dr.x / r;
          a[i].y += acceleration * dr.y / r;
          a[i].z += acceleration * dr.z / r;
        }
      }

      // Apply gravitational constant, no need to do this in every iteration
      a[i] = a[i] * G;
    }

    // Update body positions and velocities (integration step?)
    #pragma omp parallel for
    for(int i = 0; i < n; i++) {
      body[i].r = body[i].r + (v[i] * dt) + ((a[i] * (dt * dt)) / 2);
      v[i] = v[i] + (a[i] * dt);
    }
  }

  return 0;
}
