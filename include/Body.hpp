#ifndef _MPIGRAV_BODY_INCLUDED
#define _MPIGRAV_BODY_INCLUDED

#include <string>
#include <math.h>

#include "Master.hpp"
#include <util/Vec3.hpp>


class Body {
  public:
    Vec3 r;   // Position, meters
    float m;   // Mass, kilograms

  public:
    Body(void) : m(1) {}
    Body(Vec3 const pos, float const mass) : r(pos), m(mass) {}
};


#endif // _MPIGRAV_BODY_INCLUDED
