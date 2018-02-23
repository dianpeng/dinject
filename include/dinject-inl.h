#ifndef DINJECT_INL_H_
#define DINJECT_INL_H_

namespace dinject {

namespace detail {
void Build( KlassBuilder* builder , const ConfigObject& config );
} // namespace detail

template< typename T >
std::unique_ptr<T> New( const char* name , const ConfigObject& config ) {
  auto kb = detail::NewKlassObject(name);
  if(!kb) return std::unique_ptr<T>();
  detail::Build(kb.get(),config);
  return kb->Get<T>();
}

} // namespace dinject

#endif // DINJECT_INL_H_
