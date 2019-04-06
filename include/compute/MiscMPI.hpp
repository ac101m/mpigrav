#ifndef _MPI_MISC_TOOLS_INCLUDED
#define _MPI_MISC_TOOLS_INCLUDED

/*
 *   Some handy functions to make using MPI less of a pain
 */


int MyRank(void);
int MyRank(int const comm);

int RankCount(void);
int RankCount(int const comm);


#endif // _MPI_MISC_TOOLS_INCLUDED
