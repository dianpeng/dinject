#include "dinject.h"

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <utility>

namespace dinject {
namespace detail {

namespace {

bool BuildPrimitive( KlassBuilder* builder , const char* name ,
                                             const ConfigValue& config ) {
  detail::Value val;

  switch(config.index()) {

#define DO(TYPE,type)                      \
  case TYPE:                               \
    val = std::get<type>(config);          \
    builder->Build(name,std::move(val));   \
    return true

    DO(0,bool);
    DO(1,std::int64_t);
    DO(2,double);
    DO(3,std::string);

#undef DO // DO

  default: return false;
  }
}
} // namespace

void Build( KlassBuilder* builder , const ConfigObject& config ) {
  for( auto itr(config.NewIterator()); itr->HasNext() ; itr->Next() ) {
    std::string key;
    ConfigValue val;
    itr->Get(&key,&val);
    if(!BuildPrimitive(builder,key.c_str(),val)) {

      auto obj = std::get<std::shared_ptr<ConfigObject>>(val);
      auto attr= builder->klass()->FindAttribute(key.c_str());

      if(attr && attr->dep()) {
        auto sub = detail::NewKlassObject(attr->dep());
        if(sub) {
          // build the object recursively
          Build(sub.get(),*obj);
          auto holder = sub->GetAny();
          detail::Value wrapper(holder);
          builder->Build(attr,std::move(wrapper));
        }
      }
    }
  }
}

} // namespace detail

namespace {
typedef std::map<std::string,ConfigValue> STLConfigMap;

class STLConfigObjectIterator : public ConfigObject::Iterator {
 public:
  virtual bool HasNext() const {
    return itr_ != end_;
  }

  virtual bool Next() {
    ++itr_;
    return HasNext();
  }

  virtual void Get( std::string* key , ConfigValue* output ) {
    assert(HasNext());
    *key = itr_->first;
    *output= itr_->second;
  }

  STLConfigObjectIterator( STLConfigMap::const_iterator start ,
                           STLConfigMap::const_iterator end ):
    itr_(start),
    end_(end)
  {}

 private:
  STLConfigMap::const_iterator itr_;
  STLConfigMap::const_iterator end_;
};

class STLConfigObject : public ConfigObject {
 public:
  virtual const ConfigValue* Get( const char* name ) const {
    auto itr = map_.find(name);
    return itr != map_.end() ? &(itr->second) : NULL;
  }
  virtual const ConfigValue* Get( const std::string& name ) const {
    return Get(name.c_str());
  }
  virtual void Set( const char* name , const ConfigValue& value ) {
    map_[name] = value;
  }
  virtual void Set( const std::string& name , const ConfigValue& value ) {
    map_[name] = value;
  }

  virtual std::unique_ptr<Iterator> NewIterator() const {
    return std::unique_ptr<Iterator>(
        new STLConfigObjectIterator(map_.begin(),map_.end()));
  }

  virtual ~STLConfigObject() {}

 private:
  STLConfigMap map_;
};
} // namespace


std::shared_ptr<ConfigObject> NewDefaultConfigObject() {
  return std::make_shared<STLConfigObject>();
}
} // namespace dinject
