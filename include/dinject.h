#ifndef DINJECT_H_
#define DINJECT_H_

#include <cstdint>
#include <variant>

#include "meta.h"

namespace dinject {

class ConfigObject;

/**
 * Here I define a simple DSL inside of C++ to do meta data building
 * It relies on a feature that global static object's constructor will
 * be invoked at very first stage. Another option will be use attribute
 * constructor if this one has issue
 *
 * The only concern is due to the order of invocation of these constructor
 * is unknown so we cannot use the dinject once we enter into main.
 */

// Function to allower user to register its own class meta information
template< typename T >
::dinject::detail::KlassImpl<T>* Class( const char* name ) {
  return ::dinject::detail::NewKlass<T>(name);
}

// Exported macro interfaces
#include "macro-interface.h"
#define DINJECT_CLASS _DINJECT_CLASS_V0

typedef std::variant<

#define __(A) A,
DINJECT_VALUE_PRIMITIVE_TYPE(__)
#undef __ // __

  std::shared_ptr<ConfigObject>
  > ConfigValue;

// Represents a compound type or a dictionary, user to recursively
// configure another dependency
class ConfigObject {
 public:
  virtual ~ConfigObject() {}

  virtual const ConfigValue* Get( const char* ) const = 0;
  virtual const ConfigValue* Get( const std::string& ) const = 0;

  virtual void Set( const char* , const ConfigValue& ) = 0;
  virtual void Set( const std::string& , const ConfigValue& ) = 0;

  virtual const std::string* GetAttribute( const char* ) const = 0;
  virtual void SetAttribute( const char* , const std::string& ) = 0;

  class Iterator {
   public:
    virtual ~Iterator() {}

    virtual bool HasNext() const = 0;
    virtual bool Next()    = 0;
    virtual void Get ( std::string* , ConfigValue* ) = 0;
  };

  virtual std::unique_ptr<Iterator> NewIterator() const = 0;
};

// Helper to create a simple ConfigObject with a std::map, used for
// testing or some other case you don't need a json/xml/yaml
std::unique_ptr<ConfigObject> NewDefaultConfigObject();

// Create an object of type T with certian namw of given input config
template< typename T > std::unique_ptr<T>
New( const char* name , const ConfigObject& );


// Helper to create ConfigValue , Val(1) , Val(true) , Val(false)

#define DO(T,M) \
  inline ConfigValue Val(T val) { return ConfigValue(static_cast<M>(val)); }

DO(bool,bool)
DO(std::int8_t,std::int64_t)
DO(std::uint8_t,std::int64_t)
DO(std::int16_t,std::int64_t)
DO(std::uint16_t,std::int64_t)
DO(std::int32_t,std::int64_t)
DO(std::uint32_t,std::int64_t)
DO(std::int64_t,std::int64_t)
DO(std::uint64_t,std::int64_t)
DO(float,double)
DO(double,double)

#undef DO // DO

inline ConfigValue Val( const std::string& val ) { return ConfigValue(val); }
inline ConfigValue Val( std::string&& val      ) { return ConfigValue(std::move(val)); }
inline ConfigValue Val( const char* val        ) { return ConfigValue(std::string(val)); }

} // namespace dinject


#include "dinject-inl.h"

#endif // DINJECT_H_
