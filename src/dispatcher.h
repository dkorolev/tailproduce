#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "tp_exceptions.h"

namespace TailProduce {
    template <typename BASE, typename DERIVED, typename... TAIL> struct RuntimeDispatcher {
        typedef BASE T_BASE;
        typedef DERIVED T_DERIVED;
        template <typename TYPE, typename CALLBACK> static void DispatchCall(const TYPE& x, CALLBACK c) {
            if (const DERIVED* d = dynamic_cast<const DERIVED*>(&x)) {
                c(*d);
            } else {
                RuntimeDispatcher<BASE, TAIL...>::DispatchCall(x, c);
            }
        }
        template <typename TYPE, typename CALLBACK> static void DispatchCall(TYPE& x, CALLBACK c) {
            if (DERIVED* d = dynamic_cast<DERIVED*>(&x)) {
                c(*d);
            } else {
                RuntimeDispatcher<BASE, TAIL...>::DispatchCall(x, c);
            }
        }
    };

    template <typename BASE, typename DERIVED> struct RuntimeDispatcher<BASE, DERIVED> {
        typedef BASE T_BASE;
        typedef DERIVED T_DERIVED;
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
        template <typename TYPE, typename CALLBACK> static void DispatchCall(TYPE& x, CALLBACK c) {
            if (DERIVED* d = dynamic_cast<DERIVED*>(&x)) {
                c(*d);
            } else {
                BASE* b = dynamic_cast<BASE*>(&x);
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
