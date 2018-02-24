#ifndef DINJECT_MACRO_INTERFACE_H_
#define DINJECT_MACRO_INTERFACE_H_

/**
 * Define a bunch of macros to help dinject to let user
 * automatically register the metadata information
 */

#define _DINJECT_CLASS_REGISTRY_NAME(X) __dinject_##X##_registry

#define _DINJECT_CLASS_V0(X)                                       \
  class _DINJECT_CLASS_REGISTRY_NAME(X) {                          \
   public:                                                         \
    _DINJECT_CLASS_REGISTRY_NAME(X) ();                            \
    static _DINJECT_CLASS_REGISTRY_NAME(X) kRegistry;              \
  };                                                               \
  _DINJECT_CLASS_REGISTRY_NAME(X)                                  \
    _DINJECT_CLASS_REGISTRY_NAME(X)::kRegistry;                    \
  _DINJECT_CLASS_REGISTRY_NAME(X)::_DINJECT_CLASS_REGISTRY_NAME(X) \
  ()

#define _DINJECT_FRIEND_REGISTRY_V0(X) \
  friend class _DINJECT_CLASS_REGISTRY_NAME(X)

#endif // DINJECT_MACRO_INTERFACE_H_
