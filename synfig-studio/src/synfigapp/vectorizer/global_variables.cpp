#include "global_variables.h"

const double global_variables::Polyg_eps_max = 1;     // Sequence simplification max error
const double global_variables::Polyg_eps_mul = 0.75;  // Sequence simple thickness-multiplier error
const double global_variables::Quad_eps_max = 1000000;  // As in centerlinetostrokes.cpp, for sequence conversion into strokes
float global_variables::unit_size;
float global_variables::h_factor = 1;
float global_variables::w_factor = 1;
bool global_variables::max_thickness_zero = false;