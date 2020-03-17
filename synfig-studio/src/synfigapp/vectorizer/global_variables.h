class global_variables {
  public:
    static const double Polyg_eps_max;     // Sequence simplification max error
    static const double Polyg_eps_mul;  // Sequence simple thickness-multiplier error
    static const double Quad_eps_max;  // As in centerlinetostrokes.cpp, for sequence conversion into strokes
    static float unit_size;
    static float h_factor;
    static float w_factor;
    static bool max_thickness_zero;
};
