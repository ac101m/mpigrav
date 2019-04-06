#include "compute/MiscMPI.hpp"


// External
#include "mpi.h"


int MyRank(MPI_Comm const comm) {
  int myRank;
  MPI_Comm_rank(comm, &myRank);
  return myRank;
}


int MyRank(void) {
  return MyRank(MPI_COMM_WORLD);
}


int RankCount(MPI_Comm const comm) {
  int rankCount;
  MPI_Comm_size(comm, &rankCount);
  return rankCount;
}


int RankCount(void) {
  return RankCount(MPI_COMM_WORLD);
}
