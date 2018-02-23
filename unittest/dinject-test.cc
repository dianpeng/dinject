#include "dinject.h"

#include <iostream>
#include <cstdint>

class MyObject {
 public:
  MyObject():a(),b(),c() {}

  void SetA( std::int32_t v )         { a = v; }
  void SetB( std::int64_t v )         { b = v; }
  void SetC( bool v )                 { c = v; }
  void SetX( double v )               { X = v; }
  void SetStr( const std::string& s ) { str = s; }

  std::int32_t a;
  std::int64_t b;
  bool c;
  double X;
  std::string str;
};

DINJECT_CLASS(MyObject) {
  dinject::Class<MyObject>("myobj")
    ->AddPrimitive<std::int32_t>("a",&MyObject::SetA)
    ->AddPrimitive<std::int64_t>("b",&MyObject::SetB)
    ->AddPrimitive<bool>        ("c",&MyObject::SetC)
    ->AddPrimitive<double>      ("X",&MyObject::SetX)
    ->AddString                 ("Str",&MyObject::SetStr);
}

int main() {
  auto config = dinject::NewDefaultConfigObject();

  config->Set("a",dinject::Val(1));
  config->Set("b",dinject::Val(100));
  config->Set("c",dinject::Val(true));
  config->Set("X",dinject::Val(4.0));
  config->Set("Str",dinject::Val("string"));

  auto object = dinject::New<MyObject>("myobj",*config);

  std::cout<<"A:"<<object->a<<'\n'
           <<"B:"<<object->b<<'\n'
           <<"C:"<<object->c<<'\n'
           <<"X:"<<object->X<<'\n'
           <<"S:"<<object->str<<std::endl;

  return 0;
}
