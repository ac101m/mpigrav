float3 ReadF3(__global float const* f, int const i) {
  float3 f3 = {f[i * 3], f[(i * 3) + 1], f[(i * 3) + 2]};
  return f3;
}

void WriteF3(float3 const f3, __global float* f, int const i) {
  f[i * 3] = f3.x;
  f[(i * 3) + 1] = f3.y;
  f[(i * 3) + 2] = f3.z;
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
  float const d,            // Damping factor
  // Execution control
  int const bodyCount,
  int const domainOffset) {

  // Get the index for this kernel
  int i = get_global_id(0) + domainOffset;

  // Reset value of aNext
  float3 aNextInternal = 0;

  // Compute acceleration due to other bodies
  for(int j = 0; j < bodyCount; j++) {
    if(i != j) {
      float3 dr = ReadF3(r, j) - ReadF3(r, i);
      float r2 = (dr.x * dr.x) + (dr.y * dr.y) + (dr.z * dr.z);
      float r = sqrt(r2);
      float aScalar = m[j] / (r2 + d);
      aNextInternal.x += aScalar * dr.x / r;
      aNextInternal.y += aScalar * dr.y / r;
      aNextInternal.z += aScalar * dr.z / r;
    }
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
