#include "compute/Universe.hpp"


// standard
#include <iostream>
#include <fstream>
#include <string>


// External
#include "mpi.h"
#include "omp.h"
#include "compute/MiscMPI.hpp"


// Constructs a universe from vector of bodies
Universe::Universe(
  std::vector<Body> const& bodyData,
  float const G, float const dt, float const e) {

  this->bodyCount = bodyData.size();

  // Allocate integrator term buffers
  this->m = new float[bodyData.size()];
  this->v = new Vec3[bodyData.size()];
  this->r = new Vec3[bodyData.size()];
  this->a = new Vec3[bodyData.size()];
  this->rNext = new Vec3[bodyData.size()];
  this->vNext = new Vec3[bodyData.size()];
  this->aNext = new Vec3[bodyData.size()];

  // Initialise position and mass
  for(unsigned i = 0; i < bodyData.size(); i++) {
    this->m[i] = bodyData[i].m;
    this->r[i] = bodyData[i].r;
  }

  // Compute work assignments
  unsigned localBodyCount;
  unsigned domainOffset = 0;
  unsigned localBodyRemainder = bodyData.size() % RankCount();
  for(int i = 0; i < RankCount(); i++) {
    localBodyCount = bodyData.size() / RankCount();
    if(localBodyRemainder) {
      localBodyCount++;
      localBodyRemainder--;
    }
    this->rankBodyCounts.push_back(localBodyCount);
    this->rankBodyOffsets.push_back(domainOffset);
    domainOffset += localBodyCount;
  }

  // Print out work assignments
  if(MyRank() == 0) {
    std::cout << "\n[WORK DISTRIBUTION]\n";
    for(int i = 0; i < RankCount(); i++) {
      std::cout << "Process " << i << ") offset: " << this->rankBodyOffsets[i];
      std::cout << ", count: " << this->rankBodyCounts[i] << "\n";
    }
  }

  // Initialise opencl stuff
  try {
    this->InitCL();
  } catch (cl::Error err) {
    std::cout << err.what() << "(" << err.err() << ")\n";
    exit(1);
  }

  this->SetGravitationalConstant(G);
  this->SetTimestepSize(dt);
  this->SetSofteningFactor(e);
}


// Initialise opencl, horrible routine, will need to clean up
void Universe::InitCL(void) {
  if(!MyRank()) std::cout << "\n[OPENCL INITIALISATION]\n";

  // Get list of platforms
  std::vector<cl::Platform> clPlatforms;
  cl::Platform::get(&clPlatforms);

  // SELECT PLATFORM HERE

  // Create an opencl context
  //cl_context_properties cps[3] = {
  //  CL_CONTEXT_PLATFORM, (cl_context_properties)(clPlatforms[0]), 0};
  this->clContext = cl::Context(CL_DEVICE_TYPE_GPU);

  // Get all devices within this context
  std::vector<cl::Device> clDevices =
    this->clContext.getInfo<CL_CONTEXT_DEVICES>();

  // SELECT DEVICE HERE

  // Create a command queue
  this->clCommandQueue = cl::CommandQueue(this->clContext, clDevices[0]);

  // Load kernel source
  std::ifstream fp(MPIGRAV_LEAPGROG_KERNEL_PATH);
  std::string source(
    (std::istreambuf_iterator<char>(fp)),
    (std::istreambuf_iterator<char>()));

  // Make context-local program from source & build for devices
  this->clProgram = cl::Program(this->clContext, source);
  this->clProgram.build(clDevices);

  // Build the kernel
  this->clKernel = cl::Kernel(this->clProgram, "leapfrog");

  // Create opencl buffers (input)
  this->clBuf_m = cl::Buffer(
    this->clContext, CL_MEM_READ_ONLY, this->bodyCount * sizeof(float));
  this->clBuf_r = cl::Buffer(
    this->clContext, CL_MEM_READ_ONLY, this->bodyCount * sizeof(Vec3));
  this->clBuf_v = cl::Buffer(
    this->clContext, CL_MEM_READ_ONLY, this->bodyCount * sizeof(Vec3));
  this->clBuf_a = cl::Buffer(
    this->clContext, CL_MEM_READ_ONLY, this->bodyCount * sizeof(Vec3));

  // Create opencl buffers (output)
  this->clBuf_rNext = cl::Buffer(
    this->clContext, CL_MEM_WRITE_ONLY, this->GetDomainSize() * sizeof(Vec3));
  this->clBuf_vNext = cl::Buffer(
    this->clContext, CL_MEM_WRITE_ONLY, this->GetDomainSize() * sizeof(Vec3));
  this->clBuf_aNext = cl::Buffer(
    this->clContext, CL_MEM_WRITE_ONLY, this->GetDomainSize() * sizeof(Vec3));

  // Set kernel arguments (input)
  this->clKernel.setArg(0, this->clBuf_m);
  this->clKernel.setArg(1, this->clBuf_r);
  this->clKernel.setArg(2, this->clBuf_v);
  this->clKernel.setArg(3, this->clBuf_a);

  // Set kernel arguments (output)
  this->clKernel.setArg(4, this->clBuf_rNext);
  this->clKernel.setArg(5, this->clBuf_vNext);
  this->clKernel.setArg(6, this->clBuf_aNext);

  // Set kernel arguments (misc)
  int domainOffset = this->GetDomainStart();
  this->clKernel.setArg(10, this->bodyCount);
  this->clKernel.setArg(11, domainOffset);
}


// Swap buffer references
void Universe::SwapBuffers(void) {
  Vec3* tmp;
  tmp = this->r; this->r = this->rNext; this->rNext = tmp;
  tmp = this->v; this->v = this->vNext; this->vNext = tmp;
  tmp = this->a; this->a = this->aNext; this->aNext = tmp;
}


// Synchronize buffers between processes
void Universe::Synchronize(void) {
  if(RankCount() == 1) return;

  std::vector<int> rankByteCounts(rankBodyCounts.size());
  std::vector<int> rankByteOffsets(rankBodyOffsets.size());

  for(unsigned i = 0; i < rankByteCounts.size(); i++) {
    rankByteCounts[i] = this->rankBodyCounts[i] * sizeof(Vec3);
    rankByteOffsets[i] = this->rankBodyOffsets[i] * sizeof(Vec3);
  }

  // Synchronize positions
  MPI_Allgatherv(
    &this->r[this->GetDomainStart()],
    this->GetDomainSize() * sizeof(Vec3),
    MPI_BYTE, this->r,
    rankByteCounts.data(),
    rankByteOffsets.data(),
    MPI_BYTE, MPI_COMM_WORLD);

  // Synchronize velocity
  MPI_Allgatherv(
    &this->v[this->GetDomainStart()],
    this->GetDomainSize() * sizeof(Vec3),
    MPI_BYTE, this->v,
    rankByteCounts.data(),
    rankByteOffsets.data(),
    MPI_BYTE, MPI_COMM_WORLD);

  // Synchronize acceleration
  MPI_Allgatherv(
    &this->a[this->GetDomainStart()],
    this->GetDomainSize() * sizeof(Vec3),
    MPI_BYTE, this->a,
    rankByteCounts.data(),
    rankByteOffsets.data(),
    MPI_BYTE, MPI_COMM_WORLD);
}


unsigned Universe::GetDomainStart(void) {
  return this->rankBodyOffsets[MyRank()];
}

unsigned Universe::GetDomainEnd(void) {
  return this->rankBodyOffsets[MyRank()] + this->rankBodyCounts[MyRank()];
}

unsigned Universe::GetDomainSize(void) {
  return this->rankBodyCounts[MyRank()];
}


void Universe::SetGravitationalConstant(float const G) {
  if(G != this->G) {
    this->G = G;
    this->clKernel.setArg(8, G);
  }
}


void Universe::SetTimestepSize(float const dt) {
  if(dt != this->dt) {
    this->dt = dt;
    this->clKernel.setArg(7, dt);
  }
}


void Universe::SetSofteningFactor(float const e) {
  if(e != this->e) {
    this->e = e;
    float e2 = e * e;
    this->clKernel.setArg(9, e2);
  }
}


// Opencl iteration kernel
// returns the execution time of the iteration
double Universe::IterateCL(void) {
  double tStart = MPI_Wtime();

  // Copy inputs to opencl buffers
  this->clCommandQueue.enqueueWriteBuffer(
    this->clBuf_m, CL_TRUE, 0, this->bodyCount * sizeof(float), this->m);
  this->clCommandQueue.enqueueWriteBuffer(
    this->clBuf_r, CL_TRUE, 0, this->bodyCount * sizeof(Vec3), this->r);
  this->clCommandQueue.enqueueWriteBuffer(
    this->clBuf_v, CL_TRUE, 0, this->bodyCount * sizeof(Vec3), this->v);
  this->clCommandQueue.enqueueWriteBuffer(
    this->clBuf_a, CL_TRUE, 0, this->bodyCount * sizeof(Vec3), this->a);

  // Run the kernel
  cl::NDRange globalWork = this->GetDomainSize();
  cl::NDRange localWork = 256;
  this->clCommandQueue.enqueueNDRangeKernel(
    this->clKernel, cl::NullRange, globalWork, localWork);

  // Get outputs from kernel
  this->clCommandQueue.enqueueReadBuffer(
    this->clBuf_rNext, CL_TRUE, 0,
    this->GetDomainSize() * sizeof(Vec3),
    &this->rNext[this->GetDomainStart()]);
  this->clCommandQueue.enqueueReadBuffer(
    this->clBuf_vNext, CL_TRUE, 0,
    this->GetDomainSize() * sizeof(Vec3),
    &this->vNext[this->GetDomainStart()]);
  this->clCommandQueue.enqueueReadBuffer(
    this->clBuf_aNext, CL_TRUE, 0,
    this->GetDomainSize() * sizeof(Vec3),
    &this->aNext[this->GetDomainStart()]);

  // Swap references to next/previous buffers
  this->SwapBuffers();
  this->Synchronize();

  double tEnd = MPI_Wtime();
  return tEnd - tStart;
}


// Iterate simulation forward one step with given parameters
// returns the execution time of the iteration
double Universe::Iterate(void) {
  double tStart = MPI_Wtime();
  float e2 = this->e * this->e;

  // Iterate over all bodies
  #pragma omp parallel for
  for(unsigned i = this->GetDomainStart(); i < this->GetDomainEnd(); i++) {
    this->aNext[i] = Vec3(0, 0, 0);

    // Compute acceleration due to other bodies
    for(unsigned j = 0; j < this->bodyCount; j++) {
      if((i != j) && (this->r[i] != this->r[j])) {
        Vec3 dr = this->r[j] - this->r[i];
        float r2 = (dr.x * dr.x) + (dr.y * dr.y) + (dr.z * dr.z);
        float r = sqrt(r2);
        float aScalar = this->m[j] / (r2 + e2);
        this->aNext[i].x += aScalar * dr.x / r;
        this->aNext[i].y += aScalar * dr.y / r;
        this->aNext[i].z += aScalar * dr.z / r;
      }
    }
    this->aNext[i] = this->aNext[i] * this->G;

    // Compute next position
    this->rNext[i] =
      this->r[i] +
      (this->v[i] * this->dt) +
      ((this->a[i] * (this->dt * this->dt)) / 2);

    // Compute next velocity
    this->vNext[i] =
      this->v[i] +
      (((this->a[i] + this->aNext[i]) / 2) * this->dt);
  }

  // Swap references to next/previous buffers
  this->SwapBuffers();
  this->Synchronize();

  double tEnd = MPI_Wtime();
  return tEnd - tStart;
}


// Get a vector of body data from the universe
std::vector<Body> Universe::GetBodyData(void) {
  std::vector<Body> bodyData(this->bodyCount);
  for(unsigned i = 0; i < this->bodyCount; i++) {
    bodyData[i].m = this->m[i];
    bodyData[i].r = this->r[i];
  }
  return bodyData;
}


// Free integrator term buffers
Universe::~Universe(void) {
  delete [] this->m;
  delete [] this->r;
  delete [] this->v;
  delete [] this->a;
  delete [] this->rNext;
  delete [] this->vNext;
  delete [] this->aNext;
}
