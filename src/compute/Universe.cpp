#include "compute/Universe.hpp"


// standard
#include <iostream>


// External
#include "mpi.h"
#include "omp.h"
#include "compute/MiscMPI.hpp"


// Constructs a universe from vector of bodies
Universe::Universe(std::vector<Body> const& bodyData) {
  this->bodyCount = bodyData.size();

  // Allocate
  this->m = new fp_t[bodyData.size()];
  this->v = new Vec3[bodyData.size()];
  this->r = new Vec3[bodyData.size()];
  this->a = new Vec3[bodyData.size()];
  this->rNext = new Vec3[bodyData.size()];
  this->vNext = new Vec3[bodyData.size()];
  this->aNext = new Vec3[bodyData.size()];

  // Initialise
  for(unsigned i = 0; i < bodyData.size(); i++) {
    this->m[i] = bodyData[i].m;
    this->r[i] = bodyData[i].r;
    this->v[i] = Vec3(0, 0, 0);
    this->a[i] = Vec3(0, 0, 0);
    this->rNext[i] = Vec3(0, 0, 0);
    this->vNext[i] = Vec3(0, 0, 0);
    this->aNext[i] = Vec3(0, 0, 0);
  }

  // Distribute
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

  std::cout << "My domain " << GetDomainStart() << ", " << GetDomainEnd() << "\n";
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


// Get a vector of body data from the universe
std::vector<Body> Universe::GetBodyData(void) {
  std::vector<Body> bodyData(this->bodyCount);
  for(unsigned i = 0; i < this->bodyCount; i++) {
    bodyData[i].m = this->m[i];
    bodyData[i].r = this->r[i];
  }
  return bodyData;
}
