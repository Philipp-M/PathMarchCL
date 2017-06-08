//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
#define MAX_SCENE_BOUNDS 1000.0f
#define MAX_RAYMARCH_STEPS 1500
#define RAYMARCH_PRECISION 0.000001f

inline float DEBox(float3 pos, float hlen) {
  return max(fabs(pos.x), max(fabs(pos.y), fabs(pos.z))) - hlen;
}

inline float DEMengerSponge(float3 pos) {
  const float scale = 3.0f; //menger constants
  const float scaleM = 3.0f - 1.0f; //menger constants
  const float3 offset = (float3)(1.0f, 1.0f, 1.0f);
  const int iters = 10;
  const float psni = pow(scale, -(float)iters);
  for (int n = 0; n < iters; n++) {
    pos = fabs(pos);
    if (pos.x < pos.y)
      pos.xy = pos.yx;
    if (pos.x < pos.z)
      pos.xz = pos.zx;
    if (pos.y < pos.z)
      pos.yz = pos.zy;

    pos = pos * scale - offset * (scaleM);
    if (pos.z < -0.5f * offset.z * (scaleM))
      pos.z += offset.z * (scaleM);
  }
  return DEBox(pos, scale * 0.3333334f) * psni;
}

inline float DE(const float3 pos) {
  return DEMengerSponge(pos);
}

float3 calcNormal( const float3 pos ) {
  const float3 epsX = (float3)(RAYMARCH_PRECISION , 0.0f, 0.0f);
  const float3 epsY = (float3)(0.0f, RAYMARCH_PRECISION, 0.0f);
  const float3 epsZ = (float3)(0.0f, 0.0f, RAYMARCH_PRECISION);
  int tmpMatID;
  float3 n = (float3)(
      DE(pos+epsX) - DE(pos-epsX),
      DE(pos+epsY) - DE(pos-epsY),
      DE(pos+epsZ) - DE(pos-epsZ));
  return normalize(n);
}

int march(const Ray ray, float* t) {
  const float tmin = RAYMARCH_PRECISION*3.0f;
  float _t = tmin;
  int steps = -1;
  for(int i = 0; i < MAX_RAYMARCH_STEPS; ++i) {
    float dis = DE(ray.origin+ray.dir*_t);
    if( dis < RAYMARCH_PRECISION || _t > MAX_SCENE_BOUNDS)
      break;
    _t += dis;
    steps = i;
  }
  *t = _t;
  if( _t > MAX_SCENE_BOUNDS)
    return -1;
  return steps;
}

inline float3 trace(const Ray ray) {
  float t;
  int steps;
  if((steps = march(ray, &t)) != -1)
    return ((float3)(1.0, 0.9, 0.8))*1.0f/max(steps*0.1f, 1.0f);
  return (float3)(0.0f, 0.0f, 0.0f);
}

kernel void raymarch(read_write image2d_t image,
    global float4* imageRaw,
    global uint4* randStates,
    constant float3x4* vMatrix,
    const int width,
    const int height,
    int sampleCount,
    float fov) {
  const int x = get_global_id(0);
  const int y = get_global_id(1);

  if (x >= width || y >= height)
    return;

  const uint imgIndex = y*width + x;
  uint4 r = randStates[imgIndex];
  const int2 coords = (int2)(x, y);
  const float r1 = 2.0f*rand(&r);
  const float dx = r1<1.0f ? sqrt(r1)-1.0f: 1.0f-sqrt(2.0f-r1);
  const float r2 = 2.0f*rand(&r);
  const float dy = r2<1.0f ? sqrt(r2)-1.0f: 1.0f-sqrt(2.0f-r2);

  const float invWidth = 1.0f / (float)width;
  const float u = ((float)x + 0.5f + dx/2.0f) * invWidth * 2.0f - 1.0f;
  const float v = ((float)y + 0.5f + dy/2.0f) * invWidth * 2.0f - (float)height/(float)width;
  //  const float u = ((float)x) * invWidth * 2.0f - 1.0f;
  //  const float v = ((float)y) * invWidth * 2.0f - (float)height/(float)width;
  const float3 dir = matMul3x4NoTrans(vMatrix, normalize((float3)(u,v, fmin(-fov, -0.0001f))));
  const Ray ray = {matMul3x4(vMatrix, (float4)(0.0f, 0.0f, 0.0f, 1.0f)).xyz, dir};
  float4 val;
  val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)(trace(ray), 1.0f);
  imageRaw[imgIndex] = val;
  randStates[imgIndex] = r;

  write_imagef(image, coords, val/(float)sampleCount);
}
