// Standard
#include <iostream>
#include <vector>
#include <unistd.h>
#include <cmath>
using namespace std;

// Extern
#include "omp.h"
#include "mpi.h"
#include <optparse.hpp>

// Internal headers
#include "Master.hpp"
#include "Body.hpp"
#include "compute/Universe.hpp"
#include "comm/ClientManager.hpp"
#include "compute/MiscMPI.hpp"


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
  MPI_Init(&argc, &argv);

  OptionParser opt(argc, argv, "mpigrav compute server");
  AddOptions(opt);

  int n = opt.Get("nbodies");
  fp_t G = opt.Get("gravitation");
  fp_t dt = opt.Get("timestep");
  fp_t d = opt.Get("damping");

  // Set some initial body positions
  std::vector<Body> bodies(n);
  for(int i = 0; i < n; i++) {
    bodies[i].m = 100000;
    do {
      bodies[i].r.x = ((float)((rand() % 65536) - 32768)) / 32768.0f;
      bodies[i].r.y = ((float)((rand() % 65536) - 32768)) / 32768.0f;
      bodies[i].r.z = ((float)((rand() % 65536) - 32768)) / 32768.0f;
    } while(Magnitude(bodies[i].r) > 1.0f);
  }

  // Initialise universe from initial body positions
  Universe universe(bodies);

  // Listen for incoming client connections
  ClientManager clients(opt.Get("port"));

  // If thread count was specified, override OMP_NUM_THREADS
  if(opt.Specified("threadcount")) {
    int threadCount = opt.Get("threadcount");
    omp_set_num_threads(threadCount);
  }

  if(!MyRank()) std::cout << "\n[COMPUTE BEGINS]\n";

  // Limit number of iterations based on command line option
  int iterationCount = 0;
  int iterationLimit = opt.Get("iterationlimit");
  while(!iterationLimit || iterationCount < iterationLimit) {
    iterationCount++;
    clients.UpdateBodyData(universe.GetBodyData());
    double tIteration = universe.IterateCL(dt, G);
    if(!MyRank()) std::cout << "Iteration time: " << tIteration << "s\n";
  }

  MPI_Finalize();
  return 0;
}
