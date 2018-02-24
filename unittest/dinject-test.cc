#include "dinject.h"

#include <iostream>
#include <cstdint>

class MyObject {
 public:
  MyObject():a(),b(),c() {}

  std::int32_t a;
  std::int64_t b;
  bool c;
  double X;
  std::string str;

 private:
  void SetA( std::int32_t v )         { a = v; }
  void SetB( std::int64_t v )         { b = v; }
  void SetC( bool v )                 { c = v; }
  void SetX( double v )               { X = v; }
  void SetStr( const std::string& s ) { str = s; }

  DINJECT_FRIEND_REGISTRY(MyObject);
};

DINJECT_CLASS(MyObject) {
  dinject::Class<MyObject>("myobj")
    .AddPrimitive<std::int32_t>("a",&MyObject::SetA)
    .AddPrimitive<std::int64_t>("b",&MyObject::SetB)
    .AddPrimitive<bool>        ("c",&MyObject::SetC)
    .AddPrimitive<double>      ("X",&MyObject::SetX)
    .AddString                 ("Str",&MyObject::SetStr);
}

class MyObject2 {
 public:
  MyObject2() : a() , x(), obj() {}

  void SetA( double v ) { a = v; }
  void SetX( std::string&& v ) { x = std::move(v); }
  void SetObj( MyObject* v )   { obj.reset(v); }

  double a;
  std::string x;
  std::unique_ptr<MyObject> obj;
};

DINJECT_CLASS(MyObject2) {
  dinject::Class<MyObject2>("myobj2")
    .AddPrimitive<double> ("a",&MyObject2::SetA)
    .AddString            ("x",&MyObject2::SetX)
    .AddObject            ("obj","myobj",&MyObject2::SetObj);
}

int main() {
  auto config = dinject::NewDefaultConfigObject();

  config->Set("a",dinject::Val(1));
  config->Set("b",dinject::Val(100));
  config->Set("c",dinject::Val(true));
  config->Set("X",dinject::Val(4.0f));
  config->Set("Str",dinject::Val("string"));

  auto root = dinject::NewDefaultConfigObject();
  root->Set("a",dinject::Val(1.0));
  root->Set("x",dinject::Val("xx"));
  root->Set("obj",dinject::Val(config));

  auto object = dinject::New<MyObject2>("myobj2",*root);

  assert( object->a == 1.0 );
  assert( object->x == "xx");
  assert( object->obj->a == 1);
  assert( object->obj->b ==100);
  assert( object->obj->c );
  assert( object->obj->X == 4.0);
  assert( object->obj->str == "string");

  std::cout<<"tests passed\n";
  return 0;
}
