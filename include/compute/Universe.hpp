#ifndef _MPIGRAV_UNIVERSE_INCLUDED
#define _MPIGRAV_UNIVERSE_INCLUDED


// standard
#include <vector>


// Internal
#include "Master.hpp"
#include "util/Vec3.hpp"
#include "Body.hpp"


class Universe {
  private:
    // Which bodies this instance is responsible for
    std::vector<unsigned> rankBodyCounts;
    std::vector<unsigned> rankBodyOffsets;

    // Integrator term buffers
    unsigned bodyCount;
    fp_t* m;
    Vec3* r;
    Vec3* v;
    Vec3* a;
    Vec3* rNext;
    Vec3* vNext;
    Vec3* aNext;

//====[METHODS]==============================================================//

    void SwapBuffers(void);   // Swaps intermediate buffers
    void Synchronize(void);   // Synchronizes buffers between processes

    unsigned GetDomainStart(void);
    unsigned GetDomainEnd(void);
    unsigned GetDomainSize(void);

  public:
    Universe(std::vector<Body> const& bodyData);
    ~Universe(void);

    // Iterate simulation forward one step with given parameters
    // returns the execution time of the iteration
    double Iterate(fp_t const dt, fp_t const G);

    // Gets content of the universe as vector of body classes
    std::vector<Body> GetBodyData(void);
};


#endif // _MPIGRAV_UNIVERSE_INCLUDED
