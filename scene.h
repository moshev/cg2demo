#if !defined(CG2DEMO_SCENE_H)
#define CG2DEMO_SCENE_H

enum distance_func {
	// cube, centre, radius
	DF_CUBE = 'C',
	// cube, centre, vec3 radiuses
	DF_CUBE3 = 'D',
	// sphere, centre, radius
	DF_SPHERE = 'S',
	// cylinder, centreA, centreB, radius
	DF_CYLINDER_CAP = 'I',
	// plane, point, normal
	DF_PLANE = 'P',
	// mix, distance_func, distance_func, generic_func
	DF_MIX = 'X',
	// min, distance_func, distance_func
	DF_MIN = 'm',
	// max, distance_func, distance_func
	DF_MAX = 'M',
};

enum generic_func {
	// literal signed 8-bit int
	GF_LITERAL8 = '8',
	// literal signed 16-bit int
	GF_LITERALI = 'I',
	// literal signed 16-bit "big" int
	// 1 bit sign, 4 bits exponent, 11 bit mantissa
	// value = sign * 2 ** exp * mantissa
	GF_LITERALB = 'B',
	// literal 16-bit signed fixed point
	GF_LITERALF = 'F',
	// time, period
	GF_TIME = 'T',
	// time2 (oscillating), period
	GF_TIME2 = 'U',
	// smoothstep, min, max, arg
	GF_SMOOTH = 'H',
	// clamp, min, max, arg
	GF_CLAMP = 'C',
};

#endif

