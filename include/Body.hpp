#ifndef _MPIGRAV_BODY_INCLUDED
#define _MPIGRAV_BODY_INCLUDED


#include <string>
#include "Vec3.hpp"
#include <math.h>


template<class T>
class Body {
  public:
    Vec3<T> pos;    // Position, meters
    Vec3<T> v;      // Velocity, meters per second
    T m;            // Mass, kilograms

  public:
    Body(void) : m(1) {}
    Body(Vec3<T> const pos, T const mass) : pos(pos), m(mass) {}

    // Applies veolocity with given timestep
    inline void Update(T const dt) {this->pos = this->pos + (v * dt);}
    inline Vec3<T> Force(Body<T> const& other);

    std::string Str(void) {
      std::stringstream ss;
      ss << "Pos: " << this->pos.Str();
      ss << "\tv: " << sqrt(v.x*v.x + v.y*v.y + v.z*v.z) << "m/s";
      return ss.str();
    }
};


#endif // _MPIGRAV_BODY_INCLUDED
