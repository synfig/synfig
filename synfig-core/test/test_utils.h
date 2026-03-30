#ifndef SYNFIG_TEST_UTILS_H
#define SYNFIG_TEST_UTILS_H

#include <synfig/surface.h>
#include <synfig/color.h>
#include <cmath>
#include <string>
#include <fstream>
#include <limits>

namespace synfig {
namespace test {

/// Compute Peak Signal-to-Noise Ratio between two surfaces
/// Returns infinity if surfaces are identical, 0 if sizes differ
inline double compute_psnr(const Surface& a, const Surface& b)
{
    if (a.get_w() != b.get_w() || a.get_h() != b.get_h())
        return 0.0;

    int w = a.get_w();
    int h = a.get_h();
    if (w == 0 || h == 0)
        return 0.0;

    double mse = 0.0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Color ca = a[y][x];
            Color cb = b[y][x];
            double dr = ca.get_r() - cb.get_r();
            double dg = ca.get_g() - cb.get_g();
            double db = ca.get_b() - cb.get_b();
            double da = ca.get_a() - cb.get_a();
            mse += (dr*dr + dg*dg + db*db + da*da) / 4.0;
        }
    }
    mse /= (w * h);

    if (mse < 1e-10)
        return std::numeric_limits<double>::infinity();

    return 10.0 * std::log10(1.0 / mse);
}

/// Check if two surfaces are visually similar (PSNR > threshold)
/// Default threshold: 30 dB
inline bool surfaces_match(const Surface& a, const Surface& b, double min_psnr = 30.0)
{
    return compute_psnr(a, b) >= min_psnr;
}

/// Get path to a test fixture file
inline std::string fixture_path(const std::string& filename)
{
    return "fixtures/" + filename;
}

/// Check if a file exists
inline bool file_exists(const std::string& path)
{
    std::ifstream f(path);
    return f.good();
}

} // namespace test
} // namespace synfig

#endif // SYNFIG_TEST_UTILS_H
