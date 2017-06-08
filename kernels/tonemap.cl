
// Simple Linear tone-mapping
kernel void tonemapSimpleLinear(
    global float4 *imageRaw,
    global uchar4 *tonemappedOutput,
    const int width,
    const int height,
    const float sampleCount,
    const float exposure
    ) {
  const int x = get_global_id(0);
  const int y = get_global_id(1);

  if (x >= width || y >= height)
    return;

  const uint imgIndex = y*width + x;

  // Apply tone-mapping
  float4 mapped = imageRaw[imgIndex] / sampleCount;

  // Apply gamma correction and scale
  float4 normalizedOutput = clamp(pow(mapped, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f;

  tonemappedOutput[imgIndex] = (uchar4)(
      (uchar)normalizedOutput.x,
      (uchar)normalizedOutput.y,
      (uchar)normalizedOutput.z,
      255
      );
}

// Simple Reinhard tone-mapping
kernel void tonemapSimpleReinhard(
    global float4 *imageRaw,
    global uchar4 *tonemappedOutput,
    const int width,
    const int height,
    const float sampleCount,
    const float exposure
    ){
  const int x = get_global_id(0);
  const int y = get_global_id(1);

  if (x >= width || y >= height)
    return;

  const uint imgIndex = y*width + x;

  // Apply tone-mapping
  float4 hdrColor = imageRaw[imgIndex] / sampleCount * exposure;
  float4 mapped = hdrColor / (hdrColor + 1.0f);

  // Apply gamma correction and scale
  float4 normalizedOutput = clamp(pow(mapped, 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f;

  tonemappedOutput[imgIndex] = (uchar4)(
      (uchar)normalizedOutput.x,
      (uchar)normalizedOutput.y,
      (uchar)normalizedOutput.z,
      255
      );
}

