#include "OCLRenderer.hpp"
#include "CLUtils.hpp"
#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef __linux__

#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#endif

#include <glm/ext.hpp>

OCLRenderer::OCLRenderer(size_t width, size_t height, const std::string &renderKernelName,
                         const std::string &sourceFilename)
    : texture(Texture(width, height)), fov(1.0f) {
  setVMatrix(glm::mat4());
  try {
#ifdef __APPLE__
    CGLContextObj glContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);
    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)shareGroup,
    };
    cl_context c = clCreateContext(properties, 0, 0, nullptr, 0, 0);
    if (c != nullptr) {
      context = c;
      std::vector<cl::Device> devices;
      context.getInfo(CL_CONTEXT_DEVICES, &devices);
      if (devices.size() > 0) {
        device = devices[0];
        queue = cl::CommandQueue(context, device);
        // open and compile the program
        openProgram(sourceFilename, renderKernelName);
        // setup texture with the correct width and height
        reshape(width, height);
        return;
      }
    }
#elif (defined(__linux__) || defined(_WIN32))
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0) {
      std::cerr << "[OCLRenderer] no opencl platforms available" << std::endl;
      exit(EXIT_FAILURE);
    }
    for (const auto &p : platforms) {
// Find a CL capable device in the current GL context within the
// platform 'p'
#ifdef __linux__
      cl_context_properties properties[] = {CL_GL_CONTEXT_KHR,
                                            (cl_context_properties)glXGetCurrentContext(),
                                            CL_GLX_DISPLAY_KHR,
                                            (cl_context_properties)glXGetCurrentDisplay(),
                                            CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)(p)(),
                                            0};
#elif defined(_WIN32)
      cl_context_properties properties[] = {CL_GL_CONTEXT_KHR,
                                            (cl_context_properties)wglGetCurrentContext(),
                                            CL_WGL_HDC_KHR,
                                            (cl_context_properties)wglGetCurrentDC(),
                                            CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)(p)(),
                                            0};
#endif
      cl_device_id d;
      size_t devSize;
      clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id),
                            &d, &devSize);
      if (devSize > 0)
        device = d;
      else
        continue; // not the desired device, try the next platform
      context = cl::Context(device, properties);
      queue = cl::CommandQueue(context, device);
      // open and compile the program
      openProgram(sourceFilename, renderKernelName);
      // setup texture with the correct width and height
      reshape(width, height);
      // all setup
      return;
    }
#endif
    std::cerr
        << "[OCLRenderer] requested gpu not or no gpu with GL context and CL capability available"
        << std::endl;
    exit(EXIT_FAILURE);

  } catch (cl::Error error) {
    std::cerr << "[OCLRenderer] error: " << error.what() << "(" << cl::errorString(error.err())
              << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

bool OCLRenderer::openProgram(const std::string &filename, const std::string &renderKernelName) {
  try {
    std::ifstream sourcefile(filename);
    std::string sourcecode(std::istreambuf_iterator<char>(sourcefile),
                           (std::istreambuf_iterator<char>()));
    cl::Program::Sources source(1, std::make_pair(sourcecode.c_str(), sourcecode.length() + 1));

    // make program of the source code in the context
    program = cl::Program(context, source);

    // possibly some definitions for the kernel
    std::stringstream kerneloptions;
    kerneloptions << "-I kernels/";

    // build program
    std::vector<cl::Device> tmpdevices;
    tmpdevices.push_back(device);
    program.build(tmpdevices, kerneloptions.str().c_str());

    renderKernelFunc.reset(
        new cl::make_kernel<cl::ImageGL &, cl::Buffer &, cl::Buffer &, const cl::Buffer &, cl_int,
                            cl_int, cl_int, cl_float>(
            cl::Kernel(program, renderKernelName.c_str())));
    tonemapKernelFunc.reset(
        new cl::make_kernel<const cl::Buffer &, cl::Buffer &, cl_int, cl_int, cl_float, cl_float>(
            cl::Kernel(program, "tonemapSimpleReinhard")));
  } catch (cl::Error error) {
    std::cerr << error.what() << "(" << cl::errorString(error.err()) << ")" << std::endl;

    if (error.err() == CL_BUILD_PROGRAM_FAILURE)
      std::cerr << "Build log:" << std::endl
                << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;

    exit(EXIT_FAILURE);
  }
  return true;
}

void OCLRenderer::render(bool refresh) {
  glFinish();
  try {
    queue.enqueueAcquireGLObjects(&glObjs);
    cl::EnqueueArgs eargs(queue, cl::NDRange(cl::nextDivisible(texture.width, 8),
                                             cl::nextDivisible(texture.height, 8)),
                          cl::NDRange(8, 8));
    cl::Buffer vMatrixBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3x4));
    queue.enqueueWriteBuffer(vMatrixBuffer, CL_TRUE, 0, sizeof(cl_float3x4), &vMatrix);
    sampleCount = refresh ? 1 : (sampleCount + 1);
    (*renderKernelFunc)(eargs, imageBuffer, imageRawBuffer, randStatesBuffer, vMatrixBuffer,
                        texture.width, texture.height, sampleCount, fov);
    queue.enqueueReleaseGLObjects(&glObjs);
    queue.finish();
  } catch (cl::Error error) {
    std::cerr << error.what() << "(" << cl::errorString(error.err()) << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void OCLRenderer::reshape(size_t width, size_t height) {
  texture.width = width;
  texture.height = height;
  texture.createEmptyTexture();
#ifdef CL_VERSION_1_2
  imageBuffer = cl::ImageGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture.id);
#else
  imageBuffer = cl::Image2DGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture.id);
#endif
  imageRawBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, width * height * sizeof(cl_float4));
  randStatesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, width * height * sizeof(cl_uint4));
  cl_uint *randStatesInitial = new cl_uint[4 * width * height];
  for (size_t i = 0; i < 4 * width * height; ++i)
    randStatesInitial[i] = i;
  queue.enqueueWriteBuffer(randStatesBuffer, CL_TRUE, 0, width * height * sizeof(cl_uint4),
                           randStatesInitial);
  delete[] randStatesInitial;
  glObjs.clear();
  glObjs.push_back(imageBuffer);
}

const Texture &OCLRenderer::getTexture() const { return texture; }

void OCLRenderer::setVMatrix(cl_float3x4 m) { vMatrix = m; }

void OCLRenderer::setVMatrix(glm::mat4 m) {
  vMatrix = {{{{m[0][0], m[1][0], m[2][0], m[3][0]}},
              {{m[0][1], m[1][1], m[2][1], m[3][1]}},
              {{m[0][2], m[1][2], m[2][2], m[3][2]}}}};
}

void OCLRenderer::setFov(cl_float fov) { this->fov = fov; }

size_t OCLRenderer::getSampleCount() const { return sampleCount; }

std::vector<uint8_t> OCLRenderer::getImage() {
  std::vector<uint8_t> retVal(texture.width * texture.height * 4);
  glFinish();
  cl::EnqueueArgs eargs(
      queue, cl::NDRange(cl::nextDivisible(texture.width, 8), cl::nextDivisible(texture.height, 8)),
      cl::NDRange(8, 8));

  cl::Buffer tonemappedBuffer(context, CL_MEM_READ_ONLY,
                              texture.width * texture.height * sizeof(cl_uchar4));
  (*tonemapKernelFunc)(eargs, imageRawBuffer, tonemappedBuffer, texture.width, texture.height,
                       (cl_float)sampleCount, 1.0f);
  queue.enqueueReadBuffer(tonemappedBuffer, CL_TRUE, 0,
                          texture.width * texture.height * sizeof(cl_uchar4), &(retVal[0]));
  queue.finish();
  return retVal;
}
