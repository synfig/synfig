#include <gtest/gtest.h>
#include "test_utils.h"
#include <synfig/surface.h>
#include <synfig/color.h>
#include <cmath>

using namespace synfig;

TEST(GoldenImageTest, IdenticalSurfacesHaveInfinitePSNR) {
    Surface a(64, 64);
    Surface b(64, 64);
    Color red(1.0f, 0.0f, 0.0f, 1.0f);
    a.fill(red);
    b.fill(red);
    double psnr = test::compute_psnr(a, b);
    EXPECT_TRUE(std::isinf(psnr));
}

TEST(GoldenImageTest, DifferentSurfacesHaveFinitePSNR) {
    Surface a(64, 64);
    Surface b(64, 64);
    a.fill(Color(1.0f, 0.0f, 0.0f, 1.0f));
    b.fill(Color(0.0f, 1.0f, 0.0f, 1.0f));
    double psnr = test::compute_psnr(a, b);
    EXPECT_GT(psnr, 0.0);
    EXPECT_LT(psnr, 10.0);
}

TEST(GoldenImageTest, SimilarSurfacesHaveHighPSNR) {
    Surface a(64, 64);
    Surface b(64, 64);
    a.fill(Color(1.0f, 0.0f, 0.0f, 1.0f));
    b.fill(Color(0.99f, 0.01f, 0.01f, 1.0f));
    double psnr = test::compute_psnr(a, b);
    EXPECT_GT(psnr, 30.0);
}

TEST(GoldenImageTest, DifferentSizeSurfacesReturnZero) {
    Surface a(64, 64);
    Surface b(32, 32);
    double psnr = test::compute_psnr(a, b);
    EXPECT_EQ(psnr, 0.0);
}

TEST(GoldenImageTest, EmptySurfacesReturnZero) {
    Surface a;
    Surface b;
    double psnr = test::compute_psnr(a, b);
    EXPECT_EQ(psnr, 0.0);
}

TEST(GoldenImageTest, SurfacesMatchHelper) {
    Surface a(64, 64);
    Surface b(64, 64);
    Color gray(0.5f, 0.5f, 0.5f, 1.0f);
    a.fill(gray);
    b.fill(gray);
    EXPECT_TRUE(test::surfaces_match(a, b));
}

TEST(GoldenImageTest, SurfacesDoNotMatchWhenVeryDifferent) {
    Surface a(64, 64);
    Surface b(64, 64);
    a.fill(Color(1.0f, 0.0f, 0.0f, 1.0f));
    b.fill(Color(0.0f, 0.0f, 1.0f, 1.0f));
    EXPECT_FALSE(test::surfaces_match(a, b));
}

TEST(GoldenImageTest, FixturePathHelper) {
    std::string path = test::fixture_path("gradient.sif");
    EXPECT_EQ(path, "fixtures/gradient.sif");
}
