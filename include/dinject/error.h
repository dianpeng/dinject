#ifndef DINJECT_ERROR_H_
#define DINJECT_ERROR_H_

namespace dinject {
namespace detail  {

// Used to print out error message and then abort from the current
// processes. It is a better way to report error , unlike the old
// way just bailout at exception
void Fatal( const char* format , ... );

} // namespace detail
} // namespace dinject

#endif // DINJECT_ERROR_H_
