#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "tp_exceptions.h"

namespace TailProduce {
    template <typename BASE, typename DERIVED, typename... TAIL> class RuntimeDispatcher {
      public:
        template <typename TYPE, typename CALLBACK> static void DispatchCall(const TYPE& x, CALLBACK c) {
            if (const DERIVED* d = dynamic_cast<const DERIVED*>(&x)) {
                c(*d);
            } else {
                RuntimeDispatcher<BASE, TAIL...>::DispatchCall(x, c);
            }
        }
    };

    template <typename BASE, typename DERIVED> class RuntimeDispatcher<BASE, DERIVED> {
      public:
        template <typename TYPE, typename CALLBACK> static void DispatchCall(const TYPE& x, CALLBACK c) {
            if (const DERIVED* d = dynamic_cast<const DERIVED*>(&x)) {
                c(*d);
            } else {
                const BASE* b = dynamic_cast<const BASE*>(&x);
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
