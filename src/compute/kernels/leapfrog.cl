float3 ReadF3(__global float const* f, int const i) {
  float3 f3 = {f[i * 3], f[(i * 3) + 1], f[(i * 3) + 2]};
  return f3;
}

void WriteF3(float3 const f3, __global float* f, int const i) {
  f[i * 3] = f3.x;
  f[(i * 3) + 1] = f3.y;
  f[(i * 3) + 2] = f3.z;
}


float3 BodyBodyAcceleration(
  float3 ri, float3 rj,   // Positions
  float mi, float mj,     // Masses
  float e2, float3 ai) {

  float3 r = rj - ri;
  float r2 = r.x * r.x + r.y * r.y + r.z * r.z + e2;
  float r6 = r2 * r2 * r2;
  float r3inv = 1.0f / sqrt(r6);
  float s = mj * r3inv;
  ai.x += r.x * s;
  ai.y += r.y * s;
  ai.z += r.z * s;
  return ai;
}


// Simple brute-force kernel with leapfrog integrator
__kernel void leapfrog(
  // Input buffers
  __global float const* m,  // Body mass
  __global float const* r,  // Position, current
  __global float const* v,  // Position, current
  __global float const* a,  // Acceleration, current
  // Output buffers
  __global float* rNext,    // Position, next
  __global float* vNext,    // Velocity, next
  __global float* aNext,    // Acceleration, next
  // Simulation parameters
  float const dt,           // Time step
  float const G,            // Gravitational constant
  float const e2,           // Damping factor
  // Execution control
  int const bodyCount,
  int const domainOffset) {

  // Get the index for this kernel
  int i = get_global_id(0) + domainOffset;

  // Reset value of aNext
  float3 aNextInternal = 0;

  // Compute acceleration due to other bodies
  for(int j = 0; j < bodyCount; j++) {
    aNextInternal = BodyBodyAcceleration(
      ReadF3(r, i), ReadF3(r, j), m[i], m[j], e2, aNextInternal);
  }

  // Apply universal gravitational constant
  aNextInternal = aNextInternal * G;

  // Compute next position
  float3 rNextInternal =
    ReadF3(r, i) +
    (ReadF3(v, i) * dt) +
    ((ReadF3(a, i) * (dt * dt)) / 2);

  // Compute next velocity
  float3 vNextInternal =
    ReadF3(v, i) +
    (((ReadF3(a, i) + aNextInternal) / 2) * dt);

  // Write our outputs to the buffer
  WriteF3(rNextInternal, rNext, i - domainOffset);
  WriteF3(vNextInternal, vNext, i - domainOffset);
  WriteF3(aNextInternal, aNext, i - domainOffset);
}
