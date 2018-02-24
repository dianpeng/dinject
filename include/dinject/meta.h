#ifndef DINJECT_META_H_
#define DINJECT_META_H_
#include "error.h"

#include <typeinfo>
#include <cassert>
#include <cstring>
#include <memory>
#include <any>
#include <variant>
#include <string>
#include <cstdint>
#include <vector>

namespace dinject {
namespace detail {

class Attribute;
class Klass;
class KlassBuilder;

#define DINJECT_PRIMITIVE_TYPE(__)                      \
  __(kTypeBool   ,bool         ,"bool" ,bool)           \
  __(kTypeInt8   ,std::int8_t  ,"int8" ,std::int64_t)   \
  __(kTypeUInt8  ,std::uint8_t ,"uint8",std::int64_t)   \
  __(kTypeInt16  ,std::int16_t ,"int16",std::int64_t)   \
  __(kTypeUInt16 ,std::uint16_t,"uint16",std::int64_t)  \
  __(kTypeInt32  ,std::int32_t ,"int32",std::int64_t)   \
  __(kTypeUInt32 ,std::uint32_t,"uint32",std::int64_t)  \
  __(kTypeInt64  ,std::int64_t ,"int64",std::int64_t)   \
  __(kTypeUInt64 ,std::uint64_t,"uint64",std::int64_t)  \
  __(kTypeFloat  ,float        ,"float",double)         \
  __(kTypeDouble ,double       ,"double",double)

#define DINJECT_STRING_TYPE(__)                         \
  __(kTypeString ,std::string  ,"string",std::string)

#define DINJECT_OBJECT_TYPE(__)                         \
  __(kTypeStruct,std::any,"object",std::any)            \
  __(kTypeObject  ,std::any,"object",std::any)

#define DINJECT_CPP_TYPE(__)             \
  DINJECT_PRIMITIVE_TYPE(__)             \
  DINJECT_STRING_TYPE(__)                \
  DINJECT_OBJECT_TYPE(__)

// Type that is supported for injection
enum CppType {

#define __(A,...) A,
DINJECT_CPP_TYPE(__)
#undef __ // __

  kSizeOfCppType
};

const char* GetCppTypeName( CppType );

#define DINJECT_VALUE_PRIMITIVE_TYPE(__)               \
  __(bool)                                             \
  __(std::int64_t)                                     \
  __(double)                                           \
  __(std::string)

// Our variant value , used to hold all the value needs
// to be used to set to corresponding attribute
typedef std::variant<

#define __(A) A,
DINJECT_VALUE_PRIMITIVE_TYPE(__)
#undef __ // __

  std::any> Value;

template< typename T >
struct MapPrimitiveCppTypeToUniversalType {};

// Used to do static type mapping
#define __(A,B,C,D)                                      \
  template<> struct MapPrimitiveCppTypeToUniversalType<B> { \
    typedef D type;                                      \
  };
DINJECT_PRIMITIVE_TYPE(__)
#undef __ // __

template< typename T > struct MapPrimitiveCppTypeToEnum {};

#define DO(X,VALUE)                     \
  template<> struct MapPrimitiveCppTypeToEnum<X> \
  { static const CppType value = VALUE; };

#define __(A,B,...) DO(B,A)
DINJECT_PRIMITIVE_TYPE(__)
#undef __ // __

#undef DO // DO

// Object to record injected information for a certain class
class Klass : public std::enable_shared_from_this<Klass> {
 public:
  Klass( const char* name ) : name_(name), parents_() , attributes_ () {}

  virtual ~Klass() {}

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

// Used to perform reflection for setting each attributes
class KlassBuilder {
 public:
  KlassBuilder( const std::shared_ptr<Klass>& c ): klass_ (c) {}

  virtual ~KlassBuilder() {}

  // Build the primitive attribute
  virtual void Build( const char* , Value&& )  = 0;
  virtual void Build( Attribute*   , Value&& ) = 0;
  virtual std::unique_ptr<KlassBuilder> BuildStruct( Attribute* )   = 0;

  // Find the attribute based on the name
  Attribute* FindAttribute( const char* );

  // Get the corresponding Klass object
  const Klass* klass() const { return klass_.get(); }

  // Release the object back to the user
  template< typename T > std::unique_ptr<T> Get() { 
    auto holder = Release();
    T* ptr;
    try {
      ptr = std::any_cast<T*>(holder);
    } catch(...) {
      Fatal("You are trying to get object as type %s, but actual registered "
            "type information is %s",typeid(T).name(),holder.type().name());
    }
    return std::unique_ptr<T>(ptr);
  }

  // Get the value as std::any wrapped value , user to recursively build
  // needed object in a type safe way
  std::any GetAny() { return Release(); }

 protected:
  // Use std::any for type safe purpose
  virtual std::any Release() { assert(false); return std::any(); }

 private:
  std::shared_ptr<Klass> klass_;
};

class Attribute {
 public:
  Attribute( const char* name  , CppType type , const char* dep ):
    name_(name),
    dep_ (dep) ,
    type_(type)
  {}

  const char* name() const { return name_; }
  const char* dep () const { return dep_ ; }
  CppType     type() const { return type_; }
  inline const char* type_name() const;

  virtual ~Attribute() {}

 private:
  const char* name_; // name of attribute
  const char* dep_ ; // if it is an object, the specific type name
  CppType type_;     // type of attribute
};

template< typename OBJ >
struct ObjectAttributeSetter : public Attribute {
  typedef OBJ ObjectType;

  virtual void Set( OBJ* , Value&& , const Klass* ) = 0;

  ObjectAttributeSetter( const char* n , CppType type , const char* dep ):
    Attribute(n,type,dep) {}
};

template< typename OBJ >
struct ObjectAttributeGetter : public Attribute {
  typedef OBJ ObjectType;

  virtual std::unique_ptr<KlassBuilder> Get( OBJ* , const char* ) = 0;

  ObjectAttributeGetter( const char* n , CppType type , const char* dep ):
    Attribute(n,type,dep) {}
};

template< typename OBJ , typename T >
struct PrimitiveImpl : public ObjectAttributeSetter<OBJ> {};

#define DO(X)                                                      \
  template<typename OBJ>                                           \
  struct PrimitiveImpl<OBJ,X> : public ObjectAttributeSetter<OBJ> {\
    typedef void (OBJ::*Func)( X );                                \
    typedef ObjectAttributeSetter<OBJ> Base;                       \
    virtual void Set(OBJ* object,Value&& value,                    \
        const Klass* klass) {                                      \
      typedef typename MapPrimitiveCppTypeToUniversalType<X>::type \
        FromType;                                                  \
      FromType v;                                                  \
      try {                                                        \
        v = std::get<FromType>(value);                             \
      } catch(...) {                                               \
        Fatal("object %s's attribute %s expect type %s\n",         \
            klass->name(),Base::name(),Base::type_name());         \
      }                                                            \
      (object->*func)(static_cast<X>(v));                          \
    }                                                              \
    PrimitiveImpl( const char* name , Func f ):                    \
      Base(name,MapPrimitiveCppTypeToEnum<X>::value,NULL),func(f)  \
    {}                                                             \
    Func func;                                                     \
  };

#define __(A,B,...) DO(B)
DINJECT_PRIMITIVE_TYPE(__)
#undef __ // __

#undef DO // DO

template<typename OBJ>
struct StringImpl : public ObjectAttributeSetter<OBJ> {
  typedef ObjectAttributeSetter<OBJ> Base;
  typedef void (OBJ::*CRSetter)( const std::string& );
  typedef void (OBJ::*MVSetter)( std::string&&      );

  virtual void Set( OBJ* object , Value&& value , const Klass* klass ) {

    try {
      std::string& v = std::get<std::string>(value);
    } catch(...) {
      Fatal("object %s's attribute %s expect type %s",
          klass->name(),Base::name(),Base::type_name());
    }

    std::string& v = std::get<std::string>(value);
    if(cr_setter) {
      (object->*cr_setter)(v);
    } else {
      (object->*mv_setter)(std::move(v));
    }
  }

  CRSetter cr_setter;
  MVSetter mv_setter;

  StringImpl( const char* name , CRSetter cr ):
    Base(name,kTypeString,NULL),
    cr_setter(cr),
    mv_setter()
  {}

  StringImpl( const char* name , MVSetter mv ):
    Base(name,kTypeString,NULL),
    cr_setter(),
    mv_setter(mv)
  {}
};

template<typename OBJ,typename T>
struct StructImpl : public ObjectAttributeGetter<OBJ> {
  static_assert(std::is_class<T>::value,
                "require an struct type here to be declared used as struct type");
  typedef ObjectAttributeGetter<OBJ> Base;
  typedef T* (OBJ::*Getter)();

  virtual std::unique_ptr<KlassBuilder> Get( OBJ* , const char* name );

  StructImpl( const char* name , const char* dep , Getter g ):
    Base(name,kTypeStruct,dep),
    getter(g)
  {}

  Getter getter;
};

template<typename OBJ,typename T>
struct ObjectImpl : public ObjectAttributeSetter<OBJ> {
  static_assert(std::is_class<T>::value,
                "require an object pointer here to be delcared as object type");

  typedef ObjectAttributeSetter<OBJ> Base;
  typedef void (OBJ::*Func)( T* );

  virtual void Set( OBJ* object, Value&& value , const Klass* klass ) {
    std::any holder;

    try {
      holder = std::get<std::any>(value);  // type safe
    } catch(...) {
      Fatal("object %s's attribute %s except type %s",
          klass->name() , Base::name() , Base::type_name() );
    }

    auto raw = std::any_cast<T*> (holder); // type safe
    (object->*func)(raw);
  }

  ObjectImpl( const char* name , const char* dep , Func f ):
    Base(name,kTypeObject,dep), func(f)
  {}

  Func func;
};

template< typename T >
struct HeapKlassBuilderImpl : public KlassBuilder {
  typedef T ObjectType;
  HeapKlassBuilderImpl( const std::shared_ptr<Klass>& klass ) :
    KlassBuilder(klass) , object_( new T() )
  {}

  virtual void Build( const char* , Value&& value );
  virtual void Build( Attribute*  , Value&& value );
  virtual std::unique_ptr<KlassBuilder> BuildStruct( Attribute* );

  virtual std::any Release()
  { assert(object_); return std::any(object_.release()); }

 private:
  std::unique_ptr<T> object_;
};

template< typename T >
struct StructKlassBuilderImpl : public KlassBuilder {
  typedef T ObjectType;

  StructKlassBuilderImpl( const std::shared_ptr<Klass>& klass ,
      T* object ):
    KlassBuilder(klass) , object_(object)
  {}


  virtual void Build( const char* , Value&& value );
  virtual void Build( Attribute*  , Value&& value );
  virtual std::unique_ptr<KlassBuilder> BuildStruct( Attribute* );

 private:
  T* object_;
};

template< typename T >
class KlassImpl : public Klass {
 public:
  virtual std::unique_ptr<KlassBuilder> New() {
    return std::unique_ptr<KlassBuilder>(
        new HeapKlassBuilderImpl<T>(shared_from_this()) );
  }

  KlassImpl& Inherit ( const char* );

  template< typename PTYPE >
  KlassImpl& AddPrimitive( const char* name , void (T::*setter)(PTYPE) ) {
    return AddAttribute( new PrimitiveImpl<T,PTYPE>(name,setter) );
  }

  KlassImpl& AddString ( const char* name , void (T::*setter)( const std::string& ) ) {
    return AddAttribute( new StringImpl<T>(name,setter) );
  }

  KlassImpl& AddString ( const char* name , void (T::*setter)( std::string&& ) ) {
    return AddAttribute( new StringImpl<T>(name,setter) );
  }

  template< typename PTYPE >
  KlassImpl& AddStruct   ( const char* name , const char* dep ,
                                              PTYPE* (T::*getter)() ) {
    return AddAttribute( new StructImpl<T,PTYPE>(name,dep,getter) );
  }

  template< typename PTYPE >
  KlassImpl& AddObject   ( const char* name , const char* dep ,
                                              void (T::*setter)(PTYPE*) ) {
    return AddAttribute( new ObjectImpl<T,PTYPE>(name,dep,setter) );
  }

  KlassImpl( const char* name ) : Klass(name) {}

 private:
  KlassImpl& AddAttribute( Attribute* );
};


// Find the global class object
Klass* GetKlass( const char* );

// Add a Klass object with its class name
void   AddKlass( const char* , const std::shared_ptr<Klass>& );

// Create a KlassBuilder based on the name
std::unique_ptr<KlassBuilder> NewKlassObject  ( const char* );

template< typename OBJ , typename T >
std::unique_ptr<KlassBuilder> StructImpl<OBJ,T>::Get( OBJ* obj ,
                                                      const char* name ) {
  auto ret = (obj->*getter)();
  auto klass = GetKlass(name);
  return std::unique_ptr<KlassBuilder>(
      new StructKlassBuilderImpl<T>(klass->shared_from_this(),ret));
}

inline const char* Attribute::type_name() const {
  if(type() != kTypeObject)
    return GetCppTypeName(type());
  else
    return dep();
}

// New a Klass object
template< typename T > KlassImpl<T>& NewKlass( const char* name ) {
  auto impl = std::make_shared<KlassImpl<T>>(name);
  AddKlass(name,std::static_pointer_cast<Klass>(impl));
  return *impl;
}

template< typename T >
void HeapKlassBuilderImpl<T>::Build( Attribute* attr , Value&& value ) {
  assert( attr->type() != kTypeStruct ); // struct is handled specifically
  auto oattr = static_cast<ObjectAttributeSetter<T>*>(attr);
  oattr->Set(object_.get(),std::move(value),klass());
}

template< typename T >
void HeapKlassBuilderImpl<T>::Build( const char* name , Value&& value ) {
  auto attr = FindAttribute(name);
  if(attr) {
    Build(attr,std::move(value));
  }
}

template< typename T >
std::unique_ptr<KlassBuilder>
HeapKlassBuilderImpl<T>::BuildStruct( Attribute* attr ) {
  assert(attr->type() == kTypeStruct );
  auto oattr = static_cast<ObjectAttributeGetter<T>*>(attr);
  return oattr->Get(object_.get(),attr->dep());
}

template< typename T >
void StructKlassBuilderImpl<T>::Build( Attribute* attr , Value&& value ) {
  assert( attr->type() != kTypeStruct );
  auto oattr = static_cast<ObjectAttributeSetter<T>*>(attr);
  oattr->Set(object_,std::move(value),klass());
}

template< typename T >
void StructKlassBuilderImpl<T>::Build( const char* name , Value&& value ) {
  auto attr = FindAttribute(name);
  if(attr) {
    Build(attr,std::move(value));
  }
}

template< typename T >
std::unique_ptr<KlassBuilder>
StructKlassBuilderImpl<T>::BuildStruct( Attribute* attr ) {
  assert(attr->type() == kTypeStruct );
  auto oattr = static_cast<ObjectAttributeGetter<T>*>(attr);
  return oattr->Get(object_,attr->dep());
}

template< typename T >
KlassImpl<T>& KlassImpl<T>::Inherit ( const char* name ) {
  auto klass = GetKlass(name);
  if(klass) {
#ifndef NDEBUG
    for( auto &e : parents_ ) {
      assert( strcmp(e->name(),name) != 0 );
    }
#endif // NDEBUG
    parents_.push_back(klass->shared_from_this());
  }
  return *this;
}

template< typename T >
KlassImpl<T>& KlassImpl<T>::AddAttribute( Attribute* attribute ) {
#ifndef NDEBUG
  for( auto &e : attributes_ ) {
    assert(strcmp(e->name(),attribute->name()) != 0);
  }
#endif // NDEBUG
  attributes_.emplace_back(attribute);
  return *this;
}

} // namespace detail
} // namesapce dinject

#endif // DINJECT_META_H_
