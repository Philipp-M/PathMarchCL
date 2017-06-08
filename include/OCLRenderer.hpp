#pragma once

#define __CL_ENABLE_EXCEPTIONS

#include "Texture.hpp"
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <memory>

typedef struct { cl_float4 m[3]; } cl_float3x4; // for the view matrix

class OCLRenderer {
  cl_int sampleCount;          // the count of samples per pixel
  cl_float3x4 vMatrix;         // view matrix
  cl_float fov;                // has to be larger than 0, where larger values mean a smaller FOV
  cl::Context context;         // opencl context
  cl::Device device;           // the hardware device that is used to render
  cl::Program program;         // the rendering program with all the kernels
  cl::CommandQueue queue;      // the opencl queue
  cl::Buffer randStatesBuffer; // the states for random number generation
  cl::Buffer imageRawBuffer;   // the raw image, each pixel has to be divided by 'sampleCount'
  std::shared_ptr<cl::make_kernel<cl::ImageGL &, cl::Buffer &, cl::Buffer &, const cl::Buffer &,
                                  cl_int, cl_int, cl_int, cl_float>>
      renderKernelFunc; // the render kernel functor
  std::shared_ptr<
      cl::make_kernel<const cl::Buffer &, cl::Buffer &, cl_int, cl_int, cl_float, cl_float>>
      tonemapKernelFunc;          // the tonemap kernel functor
  std::vector<cl::Memory> glObjs; // shared opengl objects, it should be for now only the texture
#ifdef CL_VERSION_1_2
  cl::ImageGL imageBuffer; // the wrapping buffer for the shared opengl texture
#else
  cl::Image2DGL imageBuffer;
#endif
  Texture texture; // the texture that is containing the rendered result

public:
  /**
   * initializes opencl
   *
   * @param width the width of the desired texture size
   * @param height the height of the desired texture size
   * @param kernelname the name of the kernel e.g. mandelbrot, julia_set or mandelbrot_alt
   * @param sourceFilename the filename of the opencl file
   */
  OCLRenderer(size_t width, size_t height, const std::string &kernelname,
              const std::string &sourceFilename);

  /**
   * opens and compiles a program with the given filename and the given kernel name
   */
  bool openProgram(const std::string &filename, const std::string &kernelname);

  /**
   * renders to the texture
   *
   * @param refresh if set to true the texture gets flushed and starts with 1 samples, otherwise
   * there will be generated continously new samples for AA
   */
  void render(bool refresh);

  const Texture &getTexture() const;

  /**
   * resizes the opencl buffers and the texture
   *
   * @param width the width of the desired texture size
   * @param height the height of the desired texture size
   */
  void reshape(size_t width, size_t height);

  void setVMatrix(cl_float3x4 m);

  void setVMatrix(glm::mat4 m);

  void setFov(cl_float fov);

  size_t getSampleCount() const;

  std::vector<uint8_t> getImage();
};
