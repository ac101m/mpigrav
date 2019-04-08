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
  __global float const* m,
  __global float const* r,
  __global float const* v,
  __global float const* a,
  // Output buffers
  __global float* rNext,
  __global float* vNext,
  __global float* aNext,
  // Control constants
  int const bodyCount,
  int const domainOffset) {

  // Get the index for this kernel
  int i = get_global_id(0) + domainOffset;

  // Simulation parameters, will be passed in properly later
  float dt = 1E-1;
  float G = 6.67408E-11;
  float d = 0.2;

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
