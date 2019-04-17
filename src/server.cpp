// Standard
#include <iostream>
#include <vector>
#include <cmath>

// External
#include "omp.h"
#include "mpi.h"
#include <optparse.hpp>

// Internal
#include "Master.hpp"
#include "Body.hpp"
#include "compute/Universe.hpp"
#include "comm/Server.hpp"
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
  opt.Add(Option("updaterate", 'u', ARG_TYPE_INT,
                 "How often the server should try to update connected clients",
                 {"10"}));
}


int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  OptionParser opt(argc, argv, "mpigrav compute server");
  AddOptions(opt);

  // If thread count was specified, override OMP_NUM_THREADS
  int clientUpdateFrequency = opt.Get("updaterate");
  int threadCount = opt.Get("threadcount");
  int commPort = opt.Get("port");

  if(!MyRank()) {
    std::cout << "\n[SYSTEM PARAMETERS]\n";
    std::cout << "Client update rate: " << clientUpdateFrequency << "\n";
    std::cout << "Communication port: " << commPort << "\n";
    std::cout << "Thread count: ";
    if(!opt.Specified("threadcount")) std::cout << "OMP_NUM_THREADS\n";
    else std::cout << threadCount << "\n";
  }

  int n = opt.Get("nbodies");
  float G = opt.Get("gravitation");
  float dt = opt.Get("timestep");
  float d = opt.Get("damping");
  int iterationLimit = opt.Get("iterationlimit");

  if(!MyRank()) {
    std::cout << "\n[ALGORITHM PARAMETERS]\n";
    std::cout << "Body count: " << n << "\n";
    std::cout << "Gravitation: " << G << "\n";
    std::cout << "Timestep: " << dt << "\n";
    std::cout << "Damping factor: " << d << "\n";
    std::cout << "Iteration limit: ";
    if(!iterationLimit) std::cout << "None\n";
    else std::cout << iterationLimit << "\n";
  }

  if(!MyRank()) std::cout << "\n[INITIAL CONFIGURATION]\n";
  if(opt.Specified("threadcount")) {
    omp_set_num_threads(threadCount);
  }

  // Set some initial body positions
  if(!MyRank()) std::cout << "Initialising body positions\n";
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
  Universe universe(bodies, G, dt, d);

  // Listen for incoming client connections (only on rank 0)
  Server server(bodies);
  if(!MyRank()) server.Start(commPort, clientUpdateFrequency);

  if(!MyRank()) std::cout << "\n[SIMULATION BEGINS]\n";

  // Limit number of iterations based on command line option
  for(int i = 0; i < iterationLimit || !iterationLimit; i++) {

    // Update server body data
    server.SetBodyData(universe.GetBodyData());

    // Perform the iteration
    double tIteration;
    try {
      tIteration = universe.IterateCL();
    } catch(cl::Error err) {
      std::cout << err.what() << "(" << err.err() << ")\n";
      exit(1);
    }

    // Print out the iteration time
    if(!MyRank()) {
      std::cout << "Iteration " << i << ") time: " << tIteration << "s\n";
    }
  }

  MPI_Finalize();
  return 0;
}
