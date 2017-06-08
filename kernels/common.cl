
//------------------------------------------------------------------------------
// Datastructures
//------------------------------------------------------------------------------
typedef struct Ray {
  float3 origin;
  float3 dir;
} Ray;

typedef struct {
  float4 m[3];
} float3x4;

typedef struct {
  float4 m[4];
} float4x4;

//------------------------------------------------------------------------------
// Matrix operations
//------------------------------------------------------------------------------

// transform vector by matrix (no translation)
float3 matMul3x4NoTrans(constant float3x4 *M, const float3 v) {
  float3 r;
  r.x = dot(v, M->m[0].xyz);
  r.y = dot(v, M->m[1].xyz);
  r.z = dot(v, M->m[2].xyz);
  return r;
}

// transform vector by matrix with translation
float4 matMul3x4(constant float3x4 *M, const float4 v) {
  float4 r;
  r.x = dot(v, M->m[0]);
  r.y = dot(v, M->m[1]);
  r.z = dot(v, M->m[2]);
  r.w = 1.0f;
  return r;
}

// transform vector by matrix with translation
float4 matMul4x4(const float4x4 *M, const float4 v) {
  float4 r;
  r.x = dot(v, M->m[0]);
  r.y = dot(v, M->m[1]);
  r.z = dot(v, M->m[2]);
  r.w = dot(v, M->m[3]);
  return r;
}

// transform vector by matrix with translation
float4 matMul4x4constant(constant float4x4 *M, const float4 v) {
  float4 r;
  r.x = dot(v, M->m[0]);
  r.y = dot(v, M->m[1]);
  r.z = dot(v, M->m[2]);
  r.w = dot(v, M->m[3]);
  return r;
}

float4x4 transpose4x4(const float4x4 *M) {
  float4x4 retVal;
  retVal.m[0].x = M->m[0].x;
  retVal.m[0].y = M->m[1].x;
  retVal.m[0].z = M->m[2].x;
  retVal.m[0].w = M->m[3].x;
  retVal.m[1].x = M->m[0].y;
  retVal.m[1].y = M->m[1].y;
  retVal.m[1].z = M->m[2].y;
  retVal.m[1].w = M->m[3].y;
  retVal.m[2].x = M->m[0].z;
  retVal.m[2].y = M->m[1].z;
  retVal.m[2].z = M->m[2].z;
  retVal.m[2].w = M->m[3].z;
  retVal.m[3].x = M->m[0].w;
  retVal.m[3].y = M->m[1].w;
  retVal.m[3].z = M->m[2].w;
  retVal.m[3].w = M->m[3].w;
  return retVal;
}

float4x4 transpose4x4constant(constant float4x4 *M) {
  float4x4 retVal;
  retVal.m[0].x = M->m[0].x;
  retVal.m[0].y = M->m[1].x;
  retVal.m[0].z = M->m[2].x;
  retVal.m[0].w = M->m[3].x;
  retVal.m[1].x = M->m[0].y;
  retVal.m[1].y = M->m[1].y;
  retVal.m[1].z = M->m[2].y;
  retVal.m[1].w = M->m[3].y;
  retVal.m[2].x = M->m[0].z;
  retVal.m[2].y = M->m[1].z;
  retVal.m[2].z = M->m[2].z;
  retVal.m[2].w = M->m[3].z;
  retVal.m[3].x = M->m[0].w;
  retVal.m[3].y = M->m[1].w;
  retVal.m[3].z = M->m[2].w;
  retVal.m[3].w = M->m[3].w;
  return retVal;
}

//------------------------------------------------------------------------------
// Random number generator
// combined Tausworthe and LCG generator
//------------------------------------------------------------------------------

inline uint tausStep(uint* z, int S1, int S2, int S3, uint M) {
  uint b=((((*z) << S1) ^ (*z)) >> S2);
  return *z = ((((*z) & M) << S3) ^ b);
}

inline uint lcgStep(uint* z, uint A, uint C) {
  return *z=(A*(*z)+C);
}
inline float rand(uint4* z) {
  // Combined period is lcm(p1,p2,p3,p4)~ 2^121
  return 2.3283064365387e-10f * (float)(              // Periods
      tausStep((uint*)((uint*)z+0), 13, 19, 12, 4294967294) ^  // p1=2^31-1
      tausStep((uint*)((uint*)z+1), 2, 25, 4, 4294967288) ^    // p2=2^30-1
      tausStep((uint*)((uint*)z+2), 3, 11, 17, 4294967280) ^   // p3=2^28-1
      lcgStep((uint*)((uint*)z+3), 1664525, 1013904223)        // p4=2^32
      );
}

