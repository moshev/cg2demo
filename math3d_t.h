#if !defined(CG2_MATH3D_T_H)
#define CG2_MATH3D_T_H

#define VEC(n) typedef struct t_vec##n { float v[n]; } vec##n;

/* NOTE: Matrices are column-major like in OpenGL (and fortran) */
#define MAT(n) typedef struct t_mat##n { struct t_vec##n c[n]; } mat##n;

VEC(2);
VEC(3);
VEC(4);
MAT(3);
MAT(4);

#endif
