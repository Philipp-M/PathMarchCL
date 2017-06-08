//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
#define MAX_SCENE_BOUNDS 100000.0f
#define MAX_RAYMARCH_STEPS 250
#define RAYMARCH_PRECISION 0.0000001f
#define KIFS_ITERATIONS 40

#define light ((float3)(2.0,-4.0,-9.0))
#define matColor ((float3)(1.0,1.0,1.0))
#define lightColor (19.0f * (float3)(1.0,0.9,0.8))
#define cameraLightColor (0.01f*(float3)(0.1,0.1,0.15))
#define backgroundColor ((float3)(0.0,0.0,0.0))

float4x4 calc_transform(float3 offset, float3 axis, float angle, float scale) {
  angle *= radians(1.0f);

  float c = cos(angle);
  float s = sin(angle);

  float3 t = (1.0f-c) * axis;

  float4x4 retVal;
  retVal.m[0] = (float4)(c + t.x * axis.x, t.y * axis.x - s * axis.z, t.z * axis.x + s * axis.y, 0.0f) * scale;
  retVal.m[1] = (float4)(t.x * axis.y + s * axis.z, (c + t.y * axis.y), t.z * axis.y - s * axis.x, 0.0f) * scale;
  retVal.m[2] = (float4)(t.x * axis.z - s * axis.y, t.y * axis.z + s * axis.x, c + t.z * axis.z, 0.0f) * scale;
  retVal.m[3] = (float4)(offset, 1.0f);
  return retVal;
}
float DEKIFS(float3 p, float s) {
  const float4x4 mtmp = calc_transform((float3)(-0.4f, -0.9f, -0.49f),normalize((float3)(1.0f, 1.0f, 2.1f)), 40.0f, 1.5f);
  const float4x4 m1 = transpose4x4(&mtmp);
  p /= s;

  for (int i = 0; i < KIFS_ITERATIONS; i++) {
    p = fabs(p);

    // apply transform
    p = matMul4x4(&m1, (float4)(p.x,p.y,p.z, 0.3f)).xyz;
  }
  return ((fast_length(p) - 1.0f) * (pow(1.5f, -(float)(KIFS_ITERATIONS)))) * s;
}

inline float DE(const float3 pos) {
  return DEKIFS(pos, 1.0f);
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

float calcAO(float3 pos, float3 nor, uint4* randState )
{
  float totao = 0.0f;
  for(int aoi=0; aoi<8; aoi++) {
    float3 aopos = -1.0f+2.0f*(float3)(rand(randState), rand(randState), rand(randState));
    aopos *= sign( dot(aopos,nor) );
    aopos = pos + nor*0.01f + aopos*0.04f;
    float dd = clamp( DE(aopos)*4.0f, 0.0f, 1.0f );
    totao += dd;
  }
  totao /= 8.0f;
  return clamp( totao*totao*50.0f, 0.0f, 1.0f );
}

float softshadow(const Ray toLightray, const float mint, const float maxt, const float k ) {
  float res = 1.0;
  int steps = 0;
  for( float t=mint; t < maxt && steps < MAX_RAYMARCH_STEPS; ) {
    float h = DE(toLightray.origin + toLightray.dir*t);
    if( h < RAYMARCH_PRECISION )
      return 0.0;
    res = min( res, k*h/t );
    t += h;
    steps++;
  }
  return res;
}

inline float3 trace(const Ray ray, uint4* randState) {
  float t;
  int steps;
  if((steps = march(ray, &t)) != -1) {
    // fixed ligthning
    const float3 pos = ray.origin + ray.dir * (t);
    float3 normal = calcNormal(pos);
    float3 lPos = light - pos;
    float llen = length(lPos);
    float3 lPosNorm = normalize(lPos);
    Ray shadowRay = {pos, lPosNorm};
    // lightning from the camera
    float3 lcPos = ray.origin - pos;
    float lclen = length(lcPos);
    float3 lcPosNorm = normalize(lcPos);
    Ray shadowRayCam = {pos, lcPosNorm};
    
    /* return ((float3)(1.0, 0.9, 0.8))*1.0f/max(steps*0.1f, 1.0f); // this version is significantly faster */
    return fmax(0.07f, softshadow(shadowRay, 2.0f * RAYMARCH_PRECISION, MAX_SCENE_BOUNDS, 4.0f)) * 
                      matColor * lightColor * fmax(0.3f, dot(lPosNorm, normal)) * 1.0f/(llen * llen * 0.03f) *
                      pow(calcAO(pos, normal, randState), 1.0f/2.2f);
  }
  return backgroundColor;
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
  const float u = ((float)x + 0.5f + dx) * invWidth * 2.0f - 1.0f;
  const float v = ((float)y + 0.5f + dy) * invWidth * 2.0f - (float)height/(float)width;
  const float3 dir = matMul3x4NoTrans(vMatrix, normalize((float3)(u,v, fmin(-fov, -0.0001f))));
  const Ray ray = {matMul3x4(vMatrix, (float4)(0.0f, 0.0f, 0.0f, 1.0f)).xyz, dir};
  float4 val;
  val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f)) + (float4)(trace(ray, &r), 1.0f);
  imageRaw[imgIndex] = val;
  randStates[imgIndex] = r;

  write_imagef(image, coords, val/(float)sampleCount);
}
