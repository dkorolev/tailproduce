#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "tpexceptions.h"

namespace TailProduce {
    template <typename T_BASE, typename T_DERIVED, typename... T_TAIL> class RuntimeDispatcher {
      public:
        template <typename T_TYPE, typename T_CALLBACK> static void DispatchCall(const T_TYPE& x, T_CALLBACK c) {
            if (const T_DERIVED* d = dynamic_cast<const T_DERIVED*>(&x)) {
                c(*d);
            } else {
                RuntimeDispatcher<T_BASE, T_TAIL...>::DispatchCall(x, c);
            }
        }
    };

    template <typename T_BASE, typename T_DERIVED> class RuntimeDispatcher<T_BASE, T_DERIVED> {
      public:
        template <typename T_TYPE, typename T_CALLBACK> static void DispatchCall(const T_TYPE& x, T_CALLBACK c) {
            if (const T_DERIVED* d = dynamic_cast<const T_DERIVED*>(&x)) {
                c(*d);
            } else {
                const T_BASE* b = dynamic_cast<const T_BASE*>(&x);
                if (b) {
                    c(*b);
                } else {
                    throw UnrecognizedPolymorphicType();
                }
            }
        }
    };
};

#endif
