A very simple dependency inject library for C++
====================================================

Dependency injection for C++ and it mainly targets at dynamic dependency injection.
It is not designed to be stable but mainly target at desktop application that requires
lots of configuration ,ie games.

Basic functionality is you can dynamically register meta information for reflection and
can create an object by specify a runtime bounded data block which can be parsed from
json/xml/yaml.

example
```

  // for class MyObject
  class MyObject {
   public:
    void SetX( int val ) { x = val; }
    void SetEnable( bool val ) { enable = val; }
    void SetStr( const std::string& s ) { str = s; }

    void Print() {
      std::cout<<"X:"<<x<<" Enable:"<<enable<<" String:"<<str<<std::endl;
    }

   private:
    int x;
    bool enable;
    std::string str;
  };

  DINJECT_CLASS(MyObject) {
    dinject::Class<MyObject>("my_cool_object")->
      AddPrimitive<int>("x",&MyObject::SetX)->
      AddPrimitive<bool>("enable",&MyObject::SetEnable)->
      AddString("str",&MyObject::SetStr);
  }

  int main() {
    auto configs = dinject::NewDefaultConfigObject();
    configs->Set("x",dinject::Val(1));
    configs->Set("enable",dinject::Val(true));
    configs->Set("str",dinject::Val("Hello World"));

    auto my_object = dinject::New<MyObject>("my_cool_object",configs);

    my_object->Print(); // should see the value has been injected into the MyObject
    return 0;
  }
```

# Caveats

The library will crash (std::abort) when an error happened , like type mismatch or other
stuff. It is not designed to handle unreliable input but designed to be used for developing
desktop application with lots of configuration data.



I use it mainly for my desktop application development purpose
