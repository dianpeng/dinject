#include "error.h"

#include <cstdarg>
#include <cstring>
#include <iostream>
#include <cstdlib>

namespace dinject {
namespace detail  {

void Fatal( const char* format , ... ) {
  char buf[1024];
  va_list va;
  va_start(va,format);
  std::vsprintf(buf,format,va);
  std::cerr << "DINJECT fatal error:" << buf << std::endl;
  std::abort();
}

} // namespace detail
} // namespace dinject
