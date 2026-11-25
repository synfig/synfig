/* === S Y N F I G ========================================================= */
/*! \file rendering_software_blur.cpp
**  \brief Tests for software blur rendering
*/
/* ========================================================================= */

#include <synfig/rendering/software/function/blur.h>

#include "test_base.h"

using namespace synfig;
using namespace synfig::rendering;

static void test_solid_expands_without_softening()
{
	Surface source(9, 9);
	Surface target(5, 5);
	source.clear();
	target.clear();
	source[4][4] = Color::white();

	software::Blur::blur(software::Blur::Params(
		target,
		RectInt(0, 0, 5, 5),
		source,
		VectorInt(2, 2),
		rendering::Blur::SOLID,
		Vector(2.0, 2.0),
		false,
		Color::BLEND_COMPOSITE,
		1.0 ));

	ASSERT_EQUAL(target[2][2].get_a(), 1.0);
	ASSERT_EQUAL(target[1][1].get_a(), 1.0);
	ASSERT_EQUAL(target[3][3].get_a(), 1.0);
	ASSERT_EQUAL(target[0][0].get_a(), 0.0);
	ASSERT_EQUAL(target[4][4].get_a(), 0.0);
}

int main()
{
	TEST_SUITE_BEGIN()
	TEST_FUNCTION(test_solid_expands_without_softening)
	TEST_SUITE_END()

	return tst_exit_status;
}
