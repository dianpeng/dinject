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

struct Entity {
  std::int32_t a;
  std::int32_t b;
  bool flag;
  std::unique_ptr<MyObject> obj;

  Entity() : a(), b(), flag() , obj() {}

  void SetA( std::int32_t v ) { a = v; }
  void SetB( std::int32_t v ) { b = v; }
  void SetF( bool v         ) { flag = v; }
  void SetObj( MyObject* v  ) { obj.reset(v); }
};

DINJECT_CLASS(Entity) {
  dinject::Class<Entity>("entity")
    .AddPrimitive<std::int32_t>("a",&Entity::SetA)
    .AddPrimitive<std::int32_t>("b",&Entity::SetB)
    .AddPrimitive<bool>        ("f",&Entity::SetF)
    .AddObject<MyObject>       ("obj","myobj",&Entity::SetObj);
}

class MyObject2 {
 public:
  MyObject2() : a() , x(), obj() {}

  void SetA( double v ) { a = v; }
  void SetX( std::string&& v ) { x = std::move(v); }
  void SetObj( MyObject* v )   { obj.reset(v); }

  Entity* GetEntity() { return &entity; }

  double a;
  std::string x;
  std::unique_ptr<MyObject> obj;
  Entity entity;
};

DINJECT_CLASS(MyObject2) {
  dinject::Class<MyObject2>("myobj2")
    .AddPrimitive<double> ("a",&MyObject2::SetA)
    .AddString            ("x",&MyObject2::SetX)
    .AddObject            ("obj","myobj",&MyObject2::SetObj)
    .AddStruct<Entity>    ("entity","entity",&MyObject2::GetEntity);
}

int main() {
  auto root = dinject::NewDefaultConfigObject();
  root->Set("a",dinject::Val(1.0));
  root->Set("x",dinject::Val("xx"));

  {
    auto config = dinject::NewDefaultConfigObject();
    config->Set("a",dinject::Val(1));
    config->Set("b",dinject::Val(100));
    config->Set("c",dinject::Val(true));
    config->Set("X",dinject::Val(4.0f));
    config->Set("Str",dinject::Val("string"));
    root->Set("obj",dinject::Val(config));
  }

  {
    auto config = dinject::NewDefaultConfigObject();
    config->Set("a",dinject::Val(1));
    config->Set("b",dinject::Val(100));
    config->Set("f",dinject::Val(true));

    {
      auto c= dinject::NewDefaultConfigObject();
      c->Set("a",dinject::Val(20));
      c->Set("b",dinject::Val(200));
      c->Set("c",dinject::Val(false));
      c->Set("X",dinject::Val(40.0f));
      c->Set("Str",dinject::Val("uu"));

      config->Set("obj",dinject::Val(c));
    }
    root->Set("entity",dinject::Val(config));
  }

  auto object = dinject::New<MyObject2>("myobj2",*root);

  assert( object->a == 1.0 );
  assert( object->x == "xx");

  assert( object->obj->a == 1);
  assert( object->obj->b ==100);
  assert( object->obj->c );
  assert( object->obj->X == 4.0);
  assert( object->obj->str == "string");
  assert( object->entity.a == 1 );
  assert( object->entity.b == 100 );
  assert( object->entity.flag );
  {
    auto c = object->entity.obj.get();

    assert( c->a == 20);
    assert( c->b == 200);
    assert( c->c == false );
    assert( c->X == 40.0);
    assert( c->str == "uu");
  }

  std::cout<<"tests passed\n";
  return 0;
}
