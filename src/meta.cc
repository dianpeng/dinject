#include "meta.h"
#include <unordered_map>
#include <queue>

namespace dinject {
namespace detail  {

const char* GetCppTypeName( CppType type ) {
#define __(A,B,C,...) case A: return C;
  switch(type) {
    DINJECT_CPP_TYPE(__)
    default: assert(false); return NULL;
  }
#undef __ // __
}

Attribute* KlassBuilder::FindAttribute( const char* name ) {
  std::queue<Klass*> queue;
  queue.push(klass_.get());

  while(!queue.empty()) {
    auto &cls = queue.front();
    auto attr = cls->FindAttribute(name);
    if(attr) return attr;
    queue.pop();

    auto p = cls->parents();
    for( auto &e : p ) {
      queue.push(e.get());
    }
  }
  return NULL;
}

Attribute* Klass::FindAttribute( const char* name ) const {
  for( auto &e : attributes_ ) {
    if(strcmp(e->name(),name)==0)
      return e.get();
  }
  return NULL;
}

namespace {

class MetaManager {
 public:

  static MetaManager& GetInstance() {
    static MetaManager kInstance;
    return kInstance;
  }

  Klass* GetKlass( const char* name ) const {
    auto itr = sets_.find(name);
    return (itr == sets_.end()) ? NULL : itr->second.get();
  }

  void AddKlass( const char* name , const std::shared_ptr<Klass>& kls ) {
    sets_[name] = kls;
  }
 private:
  MetaManager():sets_() {}
  std::unordered_map<std::string,std::shared_ptr<Klass>> sets_;
};

} // namespace

std::unique_ptr<KlassBuilder> NewKlassObject( const char* name ) {
  auto kls = MetaManager::GetInstance().GetKlass(name);
  if(kls) {
    return kls->New();
  }
  return std::unique_ptr<KlassBuilder>();
}

Klass* GetKlass( const char* name ) {
  return MetaManager::GetInstance().GetKlass(name);
}

void AddKlass( const char* name , const std::shared_ptr<Klass>& kls ) {
  MetaManager::GetInstance().AddKlass(name,kls);
}

} // namespace detail
} // namespace dinject
