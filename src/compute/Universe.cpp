#include "compute/Universe.hpp"


// standard
#include <iostream>
#include <stdio.h>


// External
#include "mpi.h"
#include "omp.h"
#include "compute/MiscMPI.hpp"


// Constructs a universe from vector of bodies
Universe::Universe(std::vector<Body> const& bodyData) {
  this->bodyCount = bodyData.size();

  // Allocate integrator term buffers
  this->m = new fp_t[bodyData.size()];
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

  // Compute domain discretisation
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

  // Print out discretiation
  if(MyRank() == 0) {
    std::cout << "\n[DOMAIN DISCRETISATION]\n";
    for(int i = 0; i < RankCount(); i++) {
      std::cout << "RANK " << i << ", start: " << this->rankBodyOffsets[i];
      std::cout << ", count: " << this->rankBodyCounts[i] << "\n";
    }
  }

  // Initialise opencl stuff
  this->InitCL();
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
    MPI_BYTE,
    this->r,
    rankByteCounts.data(),
    rankByteOffsets.data(),
    MPI_BYTE,
    MPI_COMM_WORLD);

  // Synchronize velocity
  MPI_Allgatherv(
    &this->v[this->GetDomainStart()],
    this->GetDomainSize() * sizeof(Vec3),
    MPI_BYTE,
    this->v,
    rankByteCounts.data(),
    rankByteOffsets.data(),
    MPI_BYTE,
    MPI_COMM_WORLD);

  // Synchronize acceleration
  MPI_Allgatherv(
    &this->a[this->GetDomainStart()],
    this->GetDomainSize() * sizeof(Vec3),
    MPI_BYTE,
    this->a,
    rankByteCounts.data(),
    rankByteOffsets.data(),
    MPI_BYTE,
    MPI_COMM_WORLD);
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


// Iterate simulation forward one step with given parameters
// returns the execution time of the iteration
double Universe::Iterate(fp_t const dt, fp_t const G) {
  double tStart = MPI_Wtime();

  // Iterate over all bodies
  #pragma omp parallel for
  for(unsigned i = this->GetDomainStart(); i < this->GetDomainEnd(); i++) {
    this->aNext[i] = Vec3(0, 0, 0);

    // Compute acceleration due to other bodies
    for(unsigned j = 0; j < this->bodyCount; j++) {
      if((i != j) && (this->r[i] != this->r[j])) {
        Vec3 dr = this->r[j] - this->r[i];
        fp_t r2 = (dr.x * dr.x) + (dr.y * dr.y) + (dr.z * dr.z);
        fp_t r = sqrt(r2);
        fp_t aScalar = this->m[j] / (r2 + 0.2);
        this->aNext[i].x += aScalar * dr.x / r;
        this->aNext[i].y += aScalar * dr.y / r;
        this->aNext[i].z += aScalar * dr.z / r;
      }
    }
    this->aNext[i] = this->aNext[i] * G;

    // Compute next position
    this->rNext[i] =
      this->r[i] +
      (this->v[i] * dt) +
      ((this->a[i] * (dt * dt)) / 2);

    // Compute next velocity
    this->vNext[i] =
      this->v[i] +
      (((this->a[i] + this->aNext[i]) / 2) * dt);
  }

  // Swap references to next/previous buffers
  this->SwapBuffers();
  this->Synchronize();

  double tEnd = MPI_Wtime();
  return tEnd - tStart;
}


// Initialise opencl, horrible routine, will need to clean up
void Universe::InitCL(void) {
  if(!MyRank()) std::cout << "\n[OPENCL INITIALISATION]\n";
  cl_int rc;

  // Get information about all platforms
  cl_uint platformCount;
  clGetPlatformIDs(0, NULL, &platformCount);
  std::vector<cl_platform_id> platformIDs(platformCount);
  rc = clGetPlatformIDs(
    platformCount, platformIDs.data(), &platformCount);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to get platform IDs, code: " << rc << "\n";
    exit(1);
  }

  // Print out available platforms
  if(!MyRank()) {
    std::cout << "Available opencl platforms:\n";
    for(unsigned i = 0; i < platformIDs.size(); i++) {
      size_t vendorSize;
      char* vendorCstring;
      rc = clGetPlatformInfo(
        platformIDs[i], CL_PLATFORM_VENDOR, 1, NULL, &vendorSize);
      vendorCstring = (char*)malloc(sizeof(char)*vendorSize);
      rc = clGetPlatformInfo(
        platformIDs[i], CL_PLATFORM_VENDOR, vendorSize, vendorCstring, NULL);
      std::cout << "\t[" << i << "] " << vendorCstring << "\n";
    }
  }

  // Just use the first platform
  if(!MyRank()) std::cout << "Selecting platform [0]\n";
  this->clPlatformID = platformIDs[0];

  // Get IDs for devices on the local platform
  cl_uint deviceCount;
  clGetDeviceIDs(this->clPlatformID, CL_DEVICE_TYPE_ALL, 1, NULL, &deviceCount);
  std::vector<cl_device_id> deviceIDs(deviceCount);
  rc = clGetDeviceIDs(
    this->clPlatformID, CL_DEVICE_TYPE_ALL, deviceCount, deviceIDs.data(), &deviceCount);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to get device IDs, code: " << rc << "\n";
    exit(1);
  }

  // Print out available devices on this platform
  if(!MyRank()) {
    std::cout << "Available opencl devices on selected platform:\n";
    for(unsigned i = 0; i < deviceIDs.size(); i++) {
      std::cout << "\t[" << i << "]\n";
    }
  }

  // Just use the first device
  if(!MyRank()) std::cout << "Selecting device [0]\n";
  this->clDeviceID = deviceIDs[0];

  // Create a context
  this->clContext = clCreateContext(
    NULL, 1, &this->clDeviceID, NULL, NULL, &rc);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to create context ID, code: " << rc << "\n";
    exit(1);
  }

  // Create a command queue
  this->clCommandQueue = clCreateCommandQueue(
    this->clContext, this->clDeviceID, 0, &rc);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to create command queue, code: " << rc << "\n";
    exit(1);
  }

  // Load kernel source
  char* source_str;
  FILE* fp = fopen(_MPIGRAV_LEAPGROG_KERNEL_PATH, "r");
  if(!fp) {
    std::cout << "Error, failed to open kernel source file '";
    std::cout << "'" << _MPIGRAV_LEAPGROG_KERNEL_PATH << "'\n";
    exit(1);
  }
  source_str = (char*)malloc(65536);
  size_t source_size = fread(source_str, 1, 65536, fp);
  fclose(fp);

  // Build kernel
  this->clProgram = clCreateProgramWithSource(
    this->clContext, 1, (char const**)&source_str, (size_t const*)&source_size, &rc);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to make cl program from source, code: " << rc << "\n";
    exit(1);
  }

  // Build the program
  rc = clBuildProgram(
    this->clProgram, 1, &this->clDeviceID, NULL, NULL, NULL);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to build program, code: ";
    std::cout << rc << "\n";

    size_t logSize;
    clGetProgramBuildInfo(
      this->clProgram, this->clDeviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
    char* buildLog = new char[logSize + 1];
    clGetProgramBuildInfo(
      this->clProgram, this->clDeviceID, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
    std::cout << buildLog;

    exit(1);
  }

  this->clKernel = clCreateKernel(
    this->clProgram, "leapfrog", &rc);
  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to create kernel, code: " << rc << "\n";
    exit(1);
  }

  // Create opencl buffers (input)
  this->clBuf_m = clCreateBuffer(
    this->clContext, CL_MEM_READ_ONLY,
    this->bodyCount * sizeof(float), NULL, &rc);
  this->clBuf_r = clCreateBuffer(
    this->clContext, CL_MEM_READ_ONLY,
    this->bodyCount * sizeof(Vec3), NULL, &rc);
  this->clBuf_v = clCreateBuffer(
    this->clContext, CL_MEM_READ_ONLY,
    this->bodyCount * sizeof(Vec3), NULL, &rc);
  this->clBuf_a = clCreateBuffer(
    this->clContext, CL_MEM_READ_ONLY,
    this->bodyCount * sizeof(Vec3), NULL, &rc);

  // Create opencl buffers (output)
  this->clBuf_rNext = clCreateBuffer(
    this->clContext, CL_MEM_WRITE_ONLY,
    this->GetDomainSize() * sizeof(Vec3), NULL, &rc);
  this->clBuf_vNext = clCreateBuffer(
    this->clContext, CL_MEM_WRITE_ONLY,
    this->GetDomainSize() * sizeof(Vec3), NULL, &rc);
  this->clBuf_aNext = clCreateBuffer(
    this->clContext, CL_MEM_WRITE_ONLY,
    this->GetDomainSize() * sizeof(Vec3), NULL, &rc);

  // Set kernel arguments (input)
  clSetKernelArg(this->clKernel, 0, sizeof(cl_mem), (void*)&this->clBuf_m);
  clSetKernelArg(this->clKernel, 1, sizeof(cl_mem), (void*)&this->clBuf_r);
  clSetKernelArg(this->clKernel, 2, sizeof(cl_mem), (void*)&this->clBuf_v);
  clSetKernelArg(this->clKernel, 3, sizeof(cl_mem), (void*)&this->clBuf_a);

  // Set kernel arguments (output)
  clSetKernelArg(this->clKernel, 4, sizeof(cl_mem), (void*)&this->clBuf_rNext);
  clSetKernelArg(this->clKernel, 5, sizeof(cl_mem), (void*)&this->clBuf_vNext);
  clSetKernelArg(this->clKernel, 6, sizeof(cl_mem), (void*)&this->clBuf_aNext);

  // Set kernel arguments (misc)
  int domainOffset = this->GetDomainStart();
  clSetKernelArg(
    this->clKernel, 7, sizeof(int), (void*)&this->bodyCount);
  clSetKernelArg(
    this->clKernel, 8, sizeof(int), (void*)&this->rankBodyOffsets[MyRank()]);
}


// Opencl iteration kernel
// returns the execution time of the iteration
double Universe::IterateCL(fp_t const dt, fp_t const G) {
  double tStart = MPI_Wtime();
  cl_int rc;

  // Copy inputs to opencl buffers
  clEnqueueWriteBuffer(
    this->clCommandQueue, this->clBuf_m, CL_TRUE, 0,
    this->bodyCount * sizeof(float), this->m,
    0, NULL, NULL);
  clEnqueueWriteBuffer(
    this->clCommandQueue, this->clBuf_r, CL_TRUE, 0,
    this->bodyCount * sizeof(Vec3), this->r,
    0, NULL, NULL);
  clEnqueueWriteBuffer(
    this->clCommandQueue, this->clBuf_v, CL_TRUE, 0,
    this->bodyCount * sizeof(Vec3), this->v,
    0, NULL, NULL);
  clEnqueueWriteBuffer(
    this->clCommandQueue, this->clBuf_a, CL_TRUE, 0,
    this->bodyCount * sizeof(Vec3), this->a,
    0, NULL, NULL);

  // Run the kernel
  size_t globalWorkSize = this->GetDomainSize();
  size_t localWorkSize = 32;
  rc = clEnqueueNDRangeKernel(
    this->clCommandQueue, this->clKernel, 1, NULL,
    &globalWorkSize, &localWorkSize,
    0, NULL, NULL);

  if(rc != CL_SUCCESS) {
    std::cout << "Error, failed to run kernel, code: " << rc << "\n";
    exit(1);
  }

  // Get outputs from kernel
  clEnqueueReadBuffer(
    this->clCommandQueue, this->clBuf_rNext, CL_TRUE, 0,
    this->GetDomainSize() * sizeof(float) * 3,
    &this->rNext[this->GetDomainStart()],
    0, NULL, NULL);
  clEnqueueReadBuffer(
    this->clCommandQueue, this->clBuf_vNext, CL_TRUE, 0,
    this->GetDomainSize() * sizeof(float) * 3,
    &this->vNext[this->GetDomainStart()],
    0, NULL, NULL);
  clEnqueueReadBuffer(
    this->clCommandQueue, this->clBuf_aNext, CL_TRUE, 0,
    this->GetDomainSize() * sizeof(float) * 3,
    &this->aNext[this->GetDomainStart()],
    0, NULL, NULL);

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
