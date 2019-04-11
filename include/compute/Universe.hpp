#ifndef _MPIGRAV_UNIVERSE_INCLUDED
#define _MPIGRAV_UNIVERSE_INCLUDED


// standard
#include <vector>


// External
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>


// Internal
#include "Master.hpp"
#include "util/Vec3.hpp"
#include "Body.hpp"


// Kernel paths
#define _MPIGRAV_LEAPGROG_KERNEL_PATH "kernels/leapfrog.cl"


class Universe {
  private:
    // Which bodies this instance is responsible for
    std::vector<unsigned> rankBodyCounts;
    std::vector<unsigned> rankBodyOffsets;

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
    cl_platform_id clPlatformID;
    cl_device_id clDeviceID;
    cl_context clContext;
    cl_command_queue clCommandQueue;
    cl_program clProgram;
    cl_kernel clKernel;

    // OpenCL buffers
    cl_mem clBuf_m;
    cl_mem clBuf_r;
    cl_mem clBuf_v;
    cl_mem clBuf_a;
    cl_mem clBuf_rNext;
    cl_mem clBuf_vNext;
    cl_mem clBuf_aNext;

//====[METHODS]==============================================================//

    void InitCL(void);        // Initialises opencl stuff

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
    double Iterate(float const dt, float const G, float const e2);

    // OpenCL iterate method (woo speedy)
    double IterateCL(float const dt, float const G, float const e);

    // Gets content of the universe as vector of body classes
    std::vector<Body> GetBodyData(void);
};


#endif // _MPIGRAV_UNIVERSE_INCLUDED
