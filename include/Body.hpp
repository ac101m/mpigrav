#ifndef _MPIGRAV_BODY_INCLUDED
#define _MPIGRAV_BODY_INCLUDED

#include <string>
#include <math.h>

#include "Master.hpp"
#include <util/Vec3.hpp>


class Body {
  public:
    Vec3 pos;      // Position, meters
    Vec3 v;        // Velocity, meters per second
    fp_t m;              // Mass, kilograms

  public:
    Body(void) : m(1) {}
    Body(Vec3 const pos, fp_t const mass) : pos(pos), m(mass) {}

    // Applies veolocity with given timestep
    inline void Update(fp_t const dt) {this->pos = this->pos + (v * dt);}
    inline Vec3 Force(Body const& other);

    std::string Str(void) {
      std::stringstream ss;
      ss << "Pos: " << this->pos.Str();
      ss << "\tv: " << sqrt(v.x*v.x + v.y*v.y + v.z*v.z) << "m/s";
      return ss.str();
    }
};


#endif // _MPIGRAV_BODY_INCLUDED
