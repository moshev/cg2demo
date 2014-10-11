#if !defined(CG2DEMO_SCENEDSL_H)
#define CG2DEMO_SCENEDSL_H

#include <stdint.h>
#include <math.h>

#include "scene.h"

// why yes, I am mixing templates and C macros
// because I can

template <uint8_t _value>
struct ForceEval {
	static const uint8_t v = _value;
};

template <uint8_t _value>
struct Log2 {
	static const uint8_t v = ForceEval<Log2<_value / 2>::v + 1>::v;
};

// best we can do under the circumstances
template <>
struct Log2<0> {
	static const uint8_t v = 0;
};

template <>
struct Log2<1> {
	static const uint8_t v = 0;
};

#define FABSF(f) (f < 0.0f ? -f : f)
#define SC_FIXED(f) (ForceEval< ((f < 0) << 7) | (Log2<(uint8_t)FABSF(f)>::v & 0x7F) >::v),

#endif
