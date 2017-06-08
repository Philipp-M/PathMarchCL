#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }
template <typename T> inline unsigned long getPastTime(T t1) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - t1).count();
}
