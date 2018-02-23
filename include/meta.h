#ifndef DINJECT_META_H_
#define DINJECT_META_H_

#include <cstring>
#include <memory>
#include <any>
#include <variant>
#include <string>
#include <cstdint>

namespace dinject {
namespace detail {

class Attribute;
class Klass;

#define DINJECT_PRIMITIVE_TYPE(__)       \
  __(kTypeBool,bool,"bool")              \
  __(kTypeInt8,std::int8_t,"int8")       \
  __(kTypeUInt8,std::uint8_t,"uint8")    \
  __(kTypeInt16,std::int16_t,"int16")    \
  __(kTypeUInt16,std::uint16_t,"uint16") \
  __(kTypeInt32,std::int32_t,"int32")    \
  __(kTypeUInt32,std::uint32_t,"uint32") \
  __(kTypeInt64,std::int64_t,"int64")    \
  __(kTypeUInt64,std::uint64_t,"uint64") \
  __(kTypeFloat,float,"float")           \
  __(kTypeDouble,double,"double")        \
  __(kTypeString,std::string,"string")   \
  __(kTypeCString,const char*,"cstring")

#define DINJECT_OBJECT_TYPE(__)          \
  __(kTypeObject,Object*,"object")

#define DINJECT_CPP_TYPE(__)             \
  DINJECT_PRIMITIVE_TYPE(__)             \
  DINJECT_OBJECT_TYPE(__)

// Type that is supported for injection
enum CppType {

#define __(A,B,C) A,
DINJECT_CPP_TYPE(__)
#undef __ // __

  kSizeOfCppType
};

const char* GetCppTypeName( CppType );

struct None   {};
struct Object {};

template< typename T > inline Object* ToObject( T* ptr ) {
  void* ptr = reinterpret_cast<void*>(ptr);
  return static_cast<T*>(ptr);
}

template< typename T > inline T* ToOther( Object* ptr ) {
  void* ptr = reinterpret_cast<void*>(ptr);
  return static_cast<T*>(ptr);
}

// Our variant value , used to hold all the value needs
// to be used to set to corresponding attribute
typedef Value std::variant<
#define __(A,B,C) B,
  DINJECT_CPP_TYPE(__)
#undef __
  None >;

template< typename T > struct MapCppTypeToEnum {};

#define DO(X,VALUE)                     \
  template<> struct MapCppTypeToEnum<X> \
  { static const CppType value = VALUE; };

#define __(A,B,C) DO(B,A)
DINJECT_CPP_TYPE(__)
#undef __ // __

#undef DO // DO

class Attribute {
 public:
  Attribute( const char* name  , CppType type ):
    name_(name),
    type_(type)
  {}

  const char* name() const { return name_; }
  CppType     type() const { return type_; }

 private:
  const char* name_; // name of attribute
  CppType type_;     // type of attribute
};

template< typename OBJ >
class ObjectAttribute : public Attribute {
 typedef OBJ ObjectType;
 virtual void Set( OBJ* , Value&& ) = 0;

 ObjectAttribute( const char* n , CppType type ):
   Attribute(n,type) {}
};

template< typename OBJ , typename T >
struct PrimitiveImpl : public ObjectAttribute<OBJ> {};

#define DO(X)                                                  \
  template<typename OBJ>                                       \
  struct PrimitiveImpl<OBJ,X> : public ObjectAttribute<OBJ> {  \
    typedef void (OBJ::*Func)( X );                            \
    virtual void Set( OBJ* object, Value&& value ) {           \
      (object->*func)(std::get<X>(value));                     \
    }                                                          \
    PrimitiveImpl( const char* name , FUNC func ):             \
      ObjectAttribute(name,MapCppTypeToEnum<X>::value),func(f) \
    {}                                                         \
    Func func;                                                 \
  };

#define __(A,B,C) DO(B)
DINJECT_PRIMITIVE_TYPE(__)
#undef __ // __

#undef DO // DO

template<typename OBJ,typename T>
struct ObjectImpl<OBJ,T*> : public ObjectAttribute<OBJ> {
  typedef void (OBJ::*FUNC)( T* );
  virtual void Set( OBJ* object, Value&& value ) {
    auto raw = std::get<Object*>(value);
    (object->*func)(ToOther<T>(raw));
  }
  ObjectImpl( const char* name , FUNC f ):
    ObjectAttribute(name,kCppTypeObject), func(f) {}
};

// Used to perform reflection for setting each attributes
class KlassBuilder {
 public:
  KlassBuilder( const std::shared_ptr<Klass>& c ): kclass_ (c) {}

  // Create the object , called at very first
  virtual void New() = 0;

  // Build the primitive attribute
  virtual void Build( const char* , Value&& ) = 0;

  // Build the object attribute
  virtual std::unique_ptr<KlassBuilder> Build( const char* ) = 0;

  // Get the corresponding Klass object
  const Klass* klass() const { return klass_.get(); }

  // Release the object back to the user
  template< typename T > Get() { return std::any_cast<T*>(Release()); }

 protected:
  // Use std::any for type safe purpose
  virtual std::any Release() = 0;

  Attribute* FindAttribute( const char* );

 private:
  std::shared_ptr<Klass> klass_;
};

template< typename T >
struct KlassBuilderImpl : public KlassBuilder {
  typedef T ObjectType;
  KlassBuilderImpl( const std::shared_ptr<Klass>& ) :
    KlassBuilder(klass) , object_()
  {}

  virtual void New() { assert(!object_); return object_.reset( new T() ); }
  virtual void Build( const char* , Value&& value );
  virtual std::unique_ptr<KlassBuilder> Build( const char* name );
  virtual std::any Release() { return std::any(object_.release()); }
 private:
  std::unique_ptr<T> object_;
};

// Object to record injected information for a certain class
class Klass : public std::enable_shared_from_this<Klass> {
 public:
  Klass( const char* name ) : name_(name), parents_() , attributes_ () {}

  const char* name() const { return name_; }

  // Factory class to create a specialized KlassBuilder object
  virtual std::unique_ptr<KlassBuilder> New() = 0;

  // Get list of parent
  const std::vector<std::shared_ptr<Klass>>& parents() const {
    return parents_;
  }

  Attribute* FindAttribute( const char* name ) const;

 protected:
  // Name of the Klass object
  const char* name_;

  // List of base class of Klass
  std::vector<std::shared_ptr<Klass>> parents_;

  // List of attributes for this Klass
  std::vector<std::unique_ptr<Attribute>> attributes_;
};

template< typename T >
class KlassImpl : public Klass {
 public:
  virtual std::unique_ptr<KlassBuilder> New() {
    return std::make_unique( new KlassBuilderImpl<T>(shared_from_this()) );
  }

  KlassImpl* Inherit ( const char* );
  KlassImpl* AddAttribute( ObjectAttribute<T>* );
};


// Find the global class object
Klass* GetKlass( const char* );
void   AddKlass( const char* , const std::shared_ptr<Klass>& );

// Create a KlassBuilder based on the name
std::unique_ptr<KlassBuilder> NewKlassObject( const char* );

// New a Klass object
template< typename T > KlassImpl<T>* NewKlass( const char* name ) {
  auto impl = std::make_shared<KlassImpl<T>>(name);
  AddKlass(name,std::static_pointer_cast<Klass>(impl));
  return impl;
}

template< typename T >
void KlassBuilderImpl<T>::Build( const char* name , Value&& value ) {
  auto attr = FindAttribute(name);
  if(attr) {
    auto oattr = static_cast<ObjectAttribute<T>>(attr);
    oattr->Set(object_.get(),value);
  }
}

template< typename T >
std::unique_ptr<KlassBuilder> KlassBuilderImpl<T>::Build( const char* name ) {
  return NewKlassObject(name);
}

template< typename T >
KlassImpl<T>* KlassImpl<T>::Inherit ( const char* name ) {
  auto klass = GetKlass(name);
  if(klass) {
#ifndef NDEBUG
    for( auto &e : parents_ ) {
      assert( strcmp(e->name(),name) != 0 );
    }
#endif // NDEBUG
    parents_.push_back(klass->shared_from_this());
  }
  return this;
}

template< typename T >
KlassImpl<T>* KlassImpl<T>::AddAttribute( ObjectAttribute<T>* attribute ) {
#ifndef NDEBUG
  for( auto &e : attributes_ ) {
    assert(strcmp(e->name(),attribute->name()) != 0);
  }
#endif // NDEBUG
  attributes_.push_back(std::unique_ptr<Attribute>(attribute));
  return this;
}

} // namespace detail
} // namesapce dinject

#endif DINJECT_META_H_
