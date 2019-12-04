#ifndef _MPIGRAV_UNIVERSE_INCLUDED
#define _MPIGRAV_UNIVERSE_INCLUDED


// standard
#include <vector>


// External
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>


// Internal
#include "Master.hpp"
#include "util/Vec3.hpp"
#include "Body.hpp"


// Kernel paths
#define MPIGRAV_LEAPGROG_KERNEL_PATH "kernels/leapfrog.cl"


class Universe {
  private:
    // Which bodies this instance is responsible for
    std::vector<unsigned> rankBodyCounts;
    std::vector<unsigned> rankBodyOffsets;

    // Simulation parameters
    float G;
    float dt;
    float e;

    // Integrator term buffers
    unsigned bodyCount;
    float* m;
    Vec3* r;
    Vec3* v;
    Vec3* a;
    Vec3* rNext;
    Vec3* vNext;
    Vec3* aNext;

    // OpenCL handles
    cl::Context clContext;
    cl::CommandQueue clCommandQueue;
    cl::Program clProgram;
    cl::Kernel clKernel;

    // OpenCL buffers
    cl::Buffer clBuf_m;
    cl::Buffer clBuf_r;
    cl::Buffer clBuf_v;
    cl::Buffer clBuf_a;
    cl::Buffer clBuf_rNext;
    cl::Buffer clBuf_vNext;
    cl::Buffer clBuf_aNext;

//====[METHODS]==============================================================//

    void InitCL(void);        // Initialises opencl stuff

    void SwapBuffers(void);   // Swaps intermediate buffers
    void Synchronize(void);   // Synchronizes buffers between processes

    unsigned GetDomainStart(void);
    unsigned GetDomainEnd(void);
    unsigned GetDomainSize(void);

  public:
    Universe(
      std::vector<Body> const& bodyData,
      float const G, float const dt, float const e);

    // Iteration routines
    double Iterate(void);     // Slow cpu code
    double IterateCL(void);   // Opencl kernel, woo, speedy

    // Gets content of the universe as vector of body classes
    std::vector<Body> GetBodyData(void);

    // Sets for various simulation parameters
    void SetGravitationalConstant(float G);
    void SetTimestepSize(float dt);
    void SetSofteningFactor(float e);

    ~Universe(void);
};


#endif // _MPIGRAV_UNIVERSE_INCLUDED
