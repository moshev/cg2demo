// vim: sw=2 ts=8 et
// Suggested editors for this file: sublime text, vim, etc.
// visual studio fucks up the formatting (╯°□°）╯︵ ┻━┻)

SCENESPEC(0, 0, 0, 1500,
SC_MIN(
  SC_SPHERE(SC_VEC3(SC_FIXED(0),
                    SC_OMIX(SC_FIXED(-0.4),
                            SC_FIXED(0.4),
                            SC_SMOOTH(SC_FIXED(0.06666), SC_FIXED(0.52222), SC_TIME2(SC_FIXED(1.5)))),
                    SC_FIXED(0)),
            SC_FIXED(0.3)),
  SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(-0.4), SC_FIXED(0)), SC_FIXED(0.4))
)),

SCENESPEC(0, 0, 0, 2250,
SC_MIN(
  SC_SPHERE(SC_VEC3(SC_FIXED(0),
                    SC_OMIX(SC_FIXED(-0.2),
                            SC_FIXED(0.2),
                            SC_SMOOTH(SC_FIXED(0.06666), SC_FIXED(0.52222), SC_TIME2(SC_FIXED(1.5)))),
                    SC_FIXED(0)),
            SC_FIXED(0.15)),
  SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(-0.2), SC_FIXED(0)), SC_FIXED(0.2))
)),

SCENESPEC(0, 0, 0, 3000,
SC_MIN(
  SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_OMIX(SC_FIXED(0.4), SC_FIXED(0.0), SC_TIME(SC_FIXED(3))), SC_FIXED(0)),
            SC_OMIX(SC_FIXED(0.3), SC_FIXED(0.8), SC_SMOOTH(SC_FIXED(0), SC_FIXED(0.4), SC_TIME(SC_FIXED(3))))),
  SC_CUBE(SC_VEC3(SC_FIXED(0),
                  SC_OMIX(SC_FIXED(-0.4), SC_FIXED(0.0), SC_TIME(SC_FIXED(3))),
                  SC_FIXED(0)), SC_FIXED(0.4))
)
),

SCENESPEC(0, 0, 0, 3000,
SC_MIX(
  SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.8)),
  SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1)),
  SC_TIME(SC_FIXED(3))
)
),

// grid appear
SCENESPEC(0, 0, 0, 5000,
SC_MAX(
  SC_TILED(SC_VEC3(SC_FIXED(0.2), SC_FIXED(0.2), SC_FIXED(100)),
    SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1))
  ),
  SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_OMIX(SC_FIXED(0.12), SC_FIXED(20),
                                                                  SC_SMOOTH(SC_FIXED(0.2), SC_FIXED(1.0), SC_TIME(SC_FIXED(5)))))
)
),

// TORUSES
SCENESPEC(0, 0, 0, 21000,
SC_MIX(
  SC_TILED(SC_VEC3(SC_FIXED(0.2), SC_FIXED(0.2), SC_FIXED(100)),
    SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1))
  ),
  SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)),
           SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
           SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(3))),
                    SC_FIXED(0.25), SC_FIXED(0.6)),
           SC_FIXED(0.2)),
  SC_SMOOTH(SC_FIXED(0.15), SC_FIXED(0.9), SC_TIME2(SC_FIXED(21)))
)
),

// transition to robots
SCENESPEC(0, 0, 0, 4000,
SC_MIX(
    SC_TILED(SC_VEC3(SC_FIXED(0.2), SC_FIXED(0.2), SC_FIXED(100)),
      SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1))
    ),
    SC_MIN(
      SC_MIN(
        SC_CUBE3(SC_VEC3(SC_FIXED(0), SC_FIXED(0.5),
                        SC_OMIX(SC_FIXED(0), SC_FIXED(0.3), SC_TIME(SC_FIXED(4)))),
                 SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.5), SC_FIXED(0.25))),
        SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0),
                         SC_OMIX(SC_FIXED(0), SC_FIXED(-0.3), SC_TIME(SC_FIXED(4)))),
                 SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
                 SC_FIXED(0.25),
                 SC_FIXED(0.2))
      ),
      SC_PLANE(SC_VEC3(SC_FIXED(0),
                       SC_OMIX(SC_FIXED(0), SC_FIXED(-2), SC_SMOOTH(SC_FIXED(0),
                                                                    SC_FIXED(0.5),
                                                                    SC_TIME(SC_FIXED(4)))),
                       SC_FIXED(0)),
               SC_VEC3(SC_FIXED(0.1), SC_FIXED(1), SC_FIXED(0.1)))
  ),
  SC_TIME(SC_FIXED(4))
)
),

// robots
SCENESPEC(0, 0, 0, 11000,
SC_MIN(
  SC_PLANE(SC_VEC3(SC_FIXED(0), SC_FIXED(-2), SC_FIXED(0)),
           SC_VEC3(SC_FIXED(0.1), SC_FIXED(1), SC_FIXED(0.1))),
  SC_MIN(
    SC_MIX(
      SC_MIN(SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_FIXED(0.25), SC_FIXED(0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2))),
             SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_FIXED(0.75), SC_FIXED(0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2)))),
      SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0.3)),
               SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
               SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(2.4))),
                                  SC_FIXED(0.25), SC_FIXED(0.6)),
               SC_FIXED(0.2)),
      SC_TIME2(SC_FIXED(11))
    ),
    SC_MIX(
      SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(-0.3)),
               SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
               SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(2.4))),
                                  SC_FIXED(0.25), SC_FIXED(0.6)),
               SC_FIXED(0.2)),
      SC_MIN(SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_FIXED(0.25), SC_FIXED(-0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2))),
             SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_FIXED(0.75), SC_FIXED(-0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2)))),
      SC_TIME2(SC_FIXED(11))
    )
  )
)
),

// transition to only plane
SCENESPEC(0, 0, 0, 4000,
SC_MIN(
  SC_PLANE(SC_VEC3(SC_FIXED(0), SC_OMIX(SC_FIXED(-2), SC_FIXED(0), SC_TIME(SC_FIXED(4))), SC_FIXED(0)),
           SC_VEC3(SC_FIXED(0.1), SC_FIXED(1), SC_FIXED(0.1))),
  SC_MIN(
    SC_MIX(
      SC_MIN(SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_OMIX(SC_FIXED(0.25), SC_FIXED(-0.7), SC_TIME(SC_FIXED(4))), SC_FIXED(0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2))),
             SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_OMIX(SC_FIXED(0.75), SC_FIXED(-0.7), SC_TIME(SC_FIXED(4))), SC_FIXED(0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2)))),
      SC_TORUS(SC_VEC3(SC_FIXED(0), SC_OMIX(SC_FIXED(0), SC_FIXED(-0.7), SC_TIME(SC_FIXED(4))), SC_FIXED(0.3)),
               SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
               SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(3))),
                                  SC_FIXED(0.25), SC_FIXED(0.6)),
               SC_FIXED(0.2)),
      SC_TIME2(SC_FIXED(11))
    ),
    SC_MIX(
      SC_TORUS(SC_VEC3(SC_FIXED(0), SC_OMIX(SC_FIXED(0), SC_FIXED(-0.7), SC_TIME(SC_FIXED(4))), SC_FIXED(-0.3)),
               SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
               SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(3))),
                                  SC_FIXED(0.25), SC_FIXED(0.6)),
               SC_FIXED(0.2)),
      SC_MIN(SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_OMIX(SC_FIXED(0.25), SC_FIXED(-0.7), SC_TIME(SC_FIXED(4))), SC_FIXED(-0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2))),
             SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_OMIX(SC_FIXED(0.75), SC_FIXED(-0.7), SC_TIME(SC_FIXED(4))), SC_FIXED(-0.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2)))),
      SC_TIME2(SC_FIXED(11))
    )
  )
)
),

// plane to cubes
SCENESPEC(0, 0, 0, 2500,
SC_MIX(
  SC_PLANE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)),
           SC_VEC3(SC_FIXED(0.1), SC_FIXED(1), SC_FIXED(0.1))),
  SC_TILED(SC_VEC3(SC_FIXED(1), SC_FIXED(100), SC_FIXED(1)),
    SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.2))
  ),
  SC_TIME(SC_FIXED(2.5))
)
),

// cubes to mushrooms, hold mushrooms
SCENESPEC(0, 0, 0, 8000,
SC_MIX(
  SC_TILED(SC_VEC3(SC_FIXED(1), SC_FIXED(100), SC_FIXED(1)),
    SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.2))
  ),
  SC_TILED(SC_VEC3(SC_FIXED(0.1), SC_FIXED(100), SC_FIXED(0.1)),
    SC_MIX(
      SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0), SC_FIXED(0.12), SC_FIXED(0)),
                      SC_VEC3(SC_FIXED(0), SC_FIXED(0.2), SC_FIXED(0)),
                      SC_FIXED(0.03)),
      SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0.25), SC_FIXED(0)), SC_FIXED(0.09)),
      SC_SMOOTH(SC_FIXED(0.20), SC_FIXED(0.25), SC_PY)
    )
  ),
  // 8.1 because it sometimes skips a bit :/
  SC_SMOOTH(SC_FIXED(0), SC_FIXED(0.5), SC_TIME(SC_FIXED(8.1)))
)
),

SCENESPEC(0.5, 0, 0.5, 2600,
SC_MIX(
  SC_TILED(SC_VEC3(SC_FIXED(0.1), SC_FIXED(100), SC_FIXED(0.1)),
    SC_MIX(
      SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0), SC_FIXED(0.12), SC_FIXED(0)),
                      SC_VEC3(SC_FIXED(0), SC_FIXED(0.2), SC_FIXED(0)),
                      SC_FIXED(0.03)),
      SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0.25), SC_FIXED(0)), SC_FIXED(0.09)),
      SC_SMOOTH(SC_FIXED(0.20), SC_FIXED(0.25), SC_PY)
    )
  ),
  SC_MIN(
    SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
              SC_OMIX(SC_FIXED(1), SC_FIXED(0.12), SC_TIME(SC_FIXED(2.6)))),
    SC_SPHERE(SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_FIXED(0)),
              SC_OMIX(SC_FIXED(1), SC_FIXED(0.12), SC_TIME(SC_FIXED(2.6))))
  ),
  SC_TIME(SC_FIXED(2.6))
)
),

SCENESPEC(0.5, 0, 0.5, 4800,
SC_MIX(
SC_MIN(
  SC_MIX(
    SC_SPHERE(SC_VEC3(SC_TIME(SC_FIXED(1.2)), SC_FIXED(0), SC_FIXED(0)),
              SC_FIXED(0.12)),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.1), SC_FIXED(0), SC_FIXED(0)),
                    SC_VEC3(SC_FIXED(0.9), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1)),
    SC_TIME2(SC_FIXED(1.2))
  ),
  SC_MIX(
    SC_SPHERE(SC_VEC3(SC_OMIX(SC_FIXED(1), SC_FIXED(0), SC_TIME(SC_FIXED(1.2))), SC_FIXED(0), SC_FIXED(1)),
              SC_FIXED(0.12)),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.1), SC_FIXED(0), SC_FIXED(1)),
                    SC_VEC3(SC_FIXED(0.9), SC_FIXED(0), SC_FIXED(1)), SC_FIXED(0.1)),
    SC_TIME2(SC_FIXED(1.2))
  )
),
SC_MIN(
  SC_MIX(
    SC_SPHERE(SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_TIME(SC_FIXED(1.2))),
              SC_FIXED(0.12)),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_FIXED(0.1)),
                    SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_FIXED(0.9)), SC_FIXED(0.1)),
    SC_TIME2(SC_FIXED(1.2))
  ),
  SC_MIX(
    SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_OMIX(SC_FIXED(1), SC_FIXED(0), SC_TIME(SC_FIXED(1.2)))),
              SC_FIXED(0.12)),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0.1)),
                    SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0.9)), SC_FIXED(0.1)),
    SC_TIME2(SC_FIXED(1.2))
  )
),
SC_SMOOTH(SC_FIXED(0.50001), SC_FIXED(0.49999), SC_TIME(SC_FIXED(2.4)))
)
),

SCENESPEC(0.5, 0, 0.5, 2000,
SC_MIX(
  // worms
  SC_MIX(
  SC_MIN(
    SC_MIX(
      SC_SPHERE(SC_VEC3(SC_TIME(SC_FIXED(1.2)), SC_FIXED(0), SC_FIXED(0)),
                SC_FIXED(0.12)),
      SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.1), SC_FIXED(0), SC_FIXED(0)),
                      SC_VEC3(SC_FIXED(0.9), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1)),
      SC_TIME2(SC_FIXED(1.2))
    ),
    SC_MIX(
      SC_SPHERE(SC_VEC3(SC_OMIX(SC_FIXED(1), SC_FIXED(0), SC_TIME(SC_FIXED(1.2))), SC_FIXED(0), SC_FIXED(1)),
                SC_FIXED(0.12)),
      SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.1), SC_FIXED(0), SC_FIXED(1)),
                      SC_VEC3(SC_FIXED(0.9), SC_FIXED(0), SC_FIXED(1)), SC_FIXED(0.1)),
      SC_TIME2(SC_FIXED(1.2))
    )
  ),
  SC_MIN(
    SC_MIX(
      SC_SPHERE(SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_TIME(SC_FIXED(1.2))),
                SC_FIXED(0.12)),
      SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_FIXED(0.1)),
                      SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_FIXED(0.9)), SC_FIXED(0.1)),
      SC_TIME2(SC_FIXED(1.2))
    ),
    SC_MIX(
      SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_OMIX(SC_FIXED(1), SC_FIXED(0), SC_TIME(SC_FIXED(1.2)))),
                SC_FIXED(0.12)),
      SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0.1)),
                      SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0.9)), SC_FIXED(0.1)),
      SC_TIME2(SC_FIXED(1.2))
    )
  ),
  SC_SMOOTH(SC_FIXED(0.50001), SC_FIXED(0.49999), SC_TIME(SC_FIXED(2.4)))
  ),
  SC_TORUS(SC_VEC3(SC_FIXED(0.5), SC_FIXED(0), SC_FIXED(0.5)),
           SC_VEC3(SC_FIXED(0), SC_FIXED(1), SC_FIXED(0)),
           SC_FIXED(0.5),
           SC_FIXED(0.1)),
  SC_TIME(SC_FIXED(2))
)
),

SCENESPEC(0, 0, 0, 26500,
SC_MIX(
  SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)),
           SC_VEC3(SC_FIXED(0), SC_FIXED(1), SC_FIXED(0)),
           SC_FIXED(0.5),
           SC_FIXED(0.1)),
  // jake
  SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(
    SC_MAX(
      SC_MIN(
        // torso
        SC_MAX(
          SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0), SC_FIXED(0.55), SC_FIXED(0)),
                          SC_VEC3(SC_FIXED(0), SC_FIXED(-0.55), SC_FIXED(0)),
                          SC_FIXED(0.6)),
          SC_PLANE(SC_VEC3(SC_FIXED(0), SC_FIXED(-1), SC_FIXED(0)),
                   SC_VEC3(SC_FIXED(0), SC_FIXED(-1), SC_FIXED(0)))
        ),
        SC_MIN(
          // eye1
          SC_SPHERE(SC_VEC3(SC_FIXED(-0.26), SC_FIXED(0.7), SC_FIXED(0.5)), SC_FIXED(0.2)),
          // eye2
          SC_SPHERE(SC_VEC3(SC_FIXED(0.26), SC_FIXED(0.7), SC_FIXED(0.5)), SC_FIXED(0.2)) 
        )
      ),
      SC_MAX(
        SC_NEGATIVE(SC_SPHERE(SC_VEC3(SC_FIXED(-0.48), SC_FIXED(0.88), SC_FIXED(1.26)), SC_FIXED(0.8))),
        SC_NEGATIVE(SC_SPHERE(SC_VEC3(SC_FIXED(0.48), SC_FIXED(0.88), SC_FIXED(1.26)), SC_FIXED(0.8)))
      )
    ),
    // arm1
    SC_TORUS(SC_VEC3(SC_FIXED(-0.35), SC_FIXED(-0.1), SC_FIXED(0)),
             SC_VEC3(SC_FIXED(0), SC_FIXED(0.1), SC_FIXED(1)),
             SC_FIXED(0.5),
             SC_FIXED(0.1))),
    // arm2
    SC_TORUS(SC_VEC3(SC_FIXED(0.35), SC_FIXED(-0.1), SC_FIXED(0)),
             SC_VEC3(SC_FIXED(0), SC_FIXED(0.1), SC_FIXED(1)),
             SC_FIXED(0.5),
             SC_FIXED(0.1))),
    // leg1
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(-0.4), SC_FIXED(0), SC_FIXED(0)),
                    SC_VEC3(SC_FIXED(-0.4), SC_FIXED(-1.8), SC_FIXED(0)),
                    SC_FIXED(0.1))),
    // leg2
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.4), SC_FIXED(0), SC_FIXED(0)),
                    SC_VEC3(SC_FIXED(0.4), SC_FIXED(-1.8), SC_FIXED(0)),
                    SC_FIXED(0.1))),
    // moustache
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(-0.2), SC_FIXED(0.5), SC_FIXED(0.56)),
                    SC_VEC3(SC_FIXED(0.2), SC_FIXED(0.5), SC_FIXED(0.56)),
                    SC_FIXED(0.11))),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(-0.205), SC_FIXED(0.495), SC_FIXED(0.56)),
                    SC_VEC3(SC_FIXED(-0.205), SC_FIXED(0.2), SC_FIXED(0.56)),
                    SC_FIXED(0.115))),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.205), SC_FIXED(0.495), SC_FIXED(0.56)),
                    SC_VEC3(SC_FIXED(0.205), SC_FIXED(0.2), SC_FIXED(0.56)),
                    SC_FIXED(0.115))),
    // nose..
    SC_MIX(
      SC_CUBE3(SC_VEC3(SC_FIXED(0), SC_FIXED(0.4), SC_FIXED(0.58)),
               SC_VEC3(SC_FIXED(0.15), SC_FIXED(0.06), SC_FIXED(0.1))),
      SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0.4), SC_FIXED(0.54)), SC_FIXED(0.2)),
      SC_FIXED(0.65)
    )
  ),
  SC_SMOOTH(SC_FIXED(0.0), SC_FIXED(0.586), SC_TIME(SC_FIXED(26.5)))
)
),

SCENESPEC(0, 0, 0, 10000,
SC_MIX(
  // jake
  SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(SC_MIN(
    SC_MAX(
      SC_MIN(
        // torso
        SC_MAX(
          SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0), SC_FIXED(0.55), SC_FIXED(0)),
                          SC_VEC3(SC_FIXED(0), SC_FIXED(-0.55), SC_FIXED(0)),
                          SC_FIXED(0.6)),
          SC_PLANE(SC_VEC3(SC_FIXED(0), SC_FIXED(-1), SC_FIXED(0)),
                   SC_VEC3(SC_FIXED(0), SC_FIXED(-1), SC_FIXED(0)))
        ),
        SC_MIN(
          // eye1
          SC_SPHERE(SC_VEC3(SC_FIXED(-0.26), SC_FIXED(0.7), SC_FIXED(0.5)), SC_FIXED(0.2)),
          // eye2
          SC_SPHERE(SC_VEC3(SC_FIXED(0.26), SC_FIXED(0.7), SC_FIXED(0.5)), SC_FIXED(0.2)) 
        )
      ),
      SC_MAX(
        SC_NEGATIVE(SC_SPHERE(SC_VEC3(SC_FIXED(-0.48), SC_FIXED(0.88), SC_FIXED(1.26)), SC_FIXED(0.8))),
        SC_NEGATIVE(SC_SPHERE(SC_VEC3(SC_FIXED(0.48), SC_FIXED(0.88), SC_FIXED(1.26)), SC_FIXED(0.8)))
      )
    ),
    // arm1
    SC_TORUS(SC_VEC3(SC_FIXED(-0.35), SC_FIXED(-0.1), SC_FIXED(0)),
             SC_VEC3(SC_FIXED(0), SC_FIXED(0.1), SC_FIXED(1)),
             SC_FIXED(0.5),
             SC_FIXED(0.1))),
    // arm2
    SC_TORUS(SC_VEC3(SC_FIXED(0.35), SC_FIXED(-0.1), SC_FIXED(0)),
             SC_VEC3(SC_FIXED(0), SC_FIXED(0.1), SC_FIXED(1)),
             SC_FIXED(0.5),
             SC_FIXED(0.1))),
    // leg1
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(-0.4), SC_FIXED(0), SC_FIXED(0)),
                    SC_VEC3(SC_FIXED(-0.4), SC_FIXED(-1.8), SC_FIXED(0)),
                    SC_FIXED(0.1))),
    // leg2
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.4), SC_FIXED(0), SC_FIXED(0)),
                    SC_VEC3(SC_FIXED(0.4), SC_FIXED(-1.8), SC_FIXED(0)),
                    SC_FIXED(0.1))),
    // moustache
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(-0.2), SC_FIXED(0.5), SC_FIXED(0.56)),
                    SC_VEC3(SC_FIXED(0.2), SC_FIXED(0.5), SC_FIXED(0.56)),
                    SC_FIXED(0.11))),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(-0.205), SC_FIXED(0.495), SC_FIXED(0.56)),
                    SC_VEC3(SC_FIXED(-0.205), SC_FIXED(0.2), SC_FIXED(0.56)),
                    SC_FIXED(0.115))),
    SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.205), SC_FIXED(0.495), SC_FIXED(0.56)),
                    SC_VEC3(SC_FIXED(0.205), SC_FIXED(0.2), SC_FIXED(0.56)),
                    SC_FIXED(0.115))),
    // nose..
    SC_MIX(
      SC_CUBE3(SC_VEC3(SC_FIXED(0), SC_FIXED(0.4), SC_FIXED(0.58)),
               SC_VEC3(SC_FIXED(0.15), SC_FIXED(0.06), SC_FIXED(0.1))),
      SC_SPHERE(SC_VEC3(SC_FIXED(0), SC_FIXED(0.4), SC_FIXED(0.54)), SC_FIXED(0.2)),
      SC_FIXED(0.65)
    )
  ),
  SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(-0.4), SC_FIXED(0)), SC_FIXED(0.4)),
  SC_TIME(SC_FIXED(10))
)
),

#if 0
0 1500
1 2250
2 3000
3 3000
4 5000
5 21000
6 4000
7 11000
8 4000
9 2500
10 8000
11 2600
12 4800
13 2000
14 26500
15 10000
text 6000
#endif

