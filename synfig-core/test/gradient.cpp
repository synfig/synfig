/* === S Y N F I G ========================================================= */
/*! \file angle.cpp
**  \brief Test Angle class
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2025 Synfig Contributors
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <synfig/gradient.h>

#include "test_base.h"

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === C L A S S E S ======================================================= */

void
test_gradient_base_constructor_is_empty()
{
	Gradient g;
	ASSERT(g.empty());
	ASSERT_EQUAL(0, g.size());
	ASSERT(g.begin() == g.end());
	ASSERT(g.rbegin() == g.rend());
}

void
test_gradient_constructor_two_colors_keeps_the_order()
{
	Gradient g(Color::black(), Color::green());
	ASSERT_FALSE(g.empty());
	ASSERT_EQUAL(2, g.size());
	ASSERT(g.begin() != g.end());
	ASSERT(g.rbegin() != g.rend());

	ASSERT(Color::black() == g.begin()->color);
	ASSERT(Color::green() == (g.end()-1)->color);
	ASSERT_EQUAL(0., g.begin()->pos);
	ASSERT_EQUAL(1., (g.end()-1)->pos);

	g.sync();

	ASSERT(Color::black() == g.begin()->color);
	ASSERT(Color::green() == (g.end()-1)->color);
	ASSERT_EQUAL(0., g.begin()->pos);
	ASSERT_EQUAL(1., (g.end()-1)->pos);
}

void
test_gradient_constructor_three_colors_keeps_the_order()
{
	Gradient g(Color::black(), Color::green(), Color::red());
	ASSERT_FALSE(g.empty());
	ASSERT_EQUAL(3, g.size());
	ASSERT(g.begin() != g.end());
	ASSERT(g.rbegin() != g.rend());

	ASSERT(Color::black() == g.begin()->color);
	ASSERT(Color::red() == (g.end()-1)->color);
	ASSERT_EQUAL(0., g.begin()->pos);
	ASSERT_EQUAL(1., (g.end()-1)->pos);

	g.sync();

	ASSERT(Color::black() == g.begin()->color);
	ASSERT(Color::red() == (g.end()-1)->color);
	ASSERT_EQUAL(0., g.begin()->pos);
	ASSERT_EQUAL(1., (g.end()-1)->pos);
}

void
test_gradient_constructor_three_colors_let_the_second_in_the_middle()
{
	Gradient g(Color::black(), Color::green(), Color::red());
	ASSERT(Color::green() == (g.end()-2)->color);
	ASSERT_EQUAL(.5, (g.end()-2)->pos);

	g.sync();

	ASSERT(Color::green() == (g.end()-2)->color);
	ASSERT_EQUAL(.5, (g.end()-2)->pos);
}

void
test_gradient_0_color_fetches_the_empty_color_no_matter_position()
{
	Gradient g;
	ASSERT(Color() == g(0.0));
	ASSERT(Color() == g(0.3));
	ASSERT(Color() == g(0.6));
	ASSERT(Color() == g(1.0));

	ASSERT(Color() == g(-8.0));
	ASSERT(Color() == g(+8.0));
}

void
test_gradient_1_color_fetches_the_same_color_no_matter_position()
{
	{
		Gradient g1;
		g1.push_back({0.0, Color::red()});
		ASSERT(Color::red() == g1(0.0));
		ASSERT(Color::red() == g1(0.3));
		ASSERT(Color::red() == g1(0.6));
		ASSERT(Color::red() == g1(1.0));

		ASSERT(Color::red() == g1(-8.0));
		ASSERT(Color::red() == g1(+8.0));
	}

	{
		Gradient g2;
		g2.push_back({0.55, Color::cyan()});
		ASSERT(Color::cyan() == g2(0.0));
		ASSERT(Color::cyan() == g2(0.3));
		ASSERT(Color::cyan() == g2(0.6));
		ASSERT(Color::cyan() == g2(1.0));

		ASSERT(Color::cyan() == g2(-8.0));
		ASSERT(Color::cyan() == g2(+8.0));
	}
}

void
test_gradient_2_colors_fetches_the_first_color_even_if_before_first_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color::red() == g1(-0.3));
		ASSERT(Color::red() == g1(-0.6));
		ASSERT(Color::red() == g1(-1.0));
		ASSERT(Color::red() == g1(-8.0));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+3.0, Color::blue()});
		ASSERT_FALSE(Color::red() == g2(-0.3));
		ASSERT_FALSE(Color::red() == g2(-0.6));
		ASSERT_FALSE(Color::red() == g2(-1.0));
		ASSERT(Color::red() == g2(-8.0));
	}
}

void
test_gradient_2_colors_fetches_the_last_color_even_if_after_last_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color::blue() == g1(+1.3));
		ASSERT(Color::blue() == g1(+1.6));
		ASSERT(Color::blue() == g1(+2.0));
		ASSERT(Color::blue() == g1(+8.0));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+3.0, Color::blue()});
		ASSERT_FALSE(Color::blue() == g2(+1.3));
		ASSERT_FALSE(Color::blue() == g2(+1.6));
		ASSERT_FALSE(Color::blue() == g2(+2.0));
		ASSERT(Color::blue() == g2(+8.0));
	}
}

void
test_gradient_2_colors_fetches_the_first_color_for_the_exact_first_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color::red() == g1(0.0));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+3.0, Color::blue()});
		ASSERT(Color::red() == g2(-2.0));
	}
}

void
test_gradient_2_colors_fetches_the_last_color_for_the_exact_last_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color::blue() == g1(+1.0));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+3.0, Color::blue()});
		ASSERT(Color::blue() == g2(+3.0));
	}
}

void
test_gradient_2_colors_fetches_the_middle_color_for_the_exact_middle_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color(.5, 0., .5, 1.) == g1(0.5));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+2.0, Color::blue()});
		ASSERT(Color(.5, 0., .5, 1.) == g2(0.0));
	}
}

void
test_gradient_2_colors_fetches_similar_to_the_first_color_for_position_near_to_the_first_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color(.7, 0., .3, 1.) == g1(0.3));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+2.0, Color::blue()});
		ASSERT(Color(.75, 0., .25, 1.) == g2(-1.0));
	}
}

void
test_gradient_2_colors_fetches_similar_to_the_last_color_for_position_near_to_the_last_position()
{
	{
		Gradient g1(Color::red(), Color::blue());
		ASSERT(Color(.3, 0, .7, 1.) == g1(0.7));
	}

	{
		Gradient g2;
		g2.push_back({-2.0, Color::red()});
		g2.push_back({+2.0, Color::blue()});
		ASSERT(Color(.25, 0., .75, 1.) == g2(+1.0));
	}
}

void
test_gradient_3_colors_fetches_the_first_color_even_if_before_first_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT(Color::red() == g1(-0.3));
	ASSERT(Color::red() == g1(-0.6));
	ASSERT(Color::red() == g1(-1.0));
	ASSERT(Color::red() == g1(-8.0));
}

void
test_gradient_3_colors_fetches_the_last_color_even_if_after_last_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT(Color::blue() == g1(+1.3));
	ASSERT(Color::blue() == g1(+1.6));
	ASSERT(Color::blue() == g1(+2.0));
	ASSERT(Color::blue() == g1(+8.0));
}

void
test_gradient_3_colors_fetches_the_first_color_for_the_exact_first_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT(Color::red() == g1(0.0));
}

void
test_gradient_3_colors_fetches_the_last_color_for_the_exact_last_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT(Color::blue() == g1(+1.0));
}

void
test_gradient_3_colors_fetches_the_middle_color_for_the_exact_middle_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT(Color::green() == g1(0.5));
}

void
test_gradient_3_colors_fetches_similar_to_the_first_color_for_position_near_to_the_first_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT_APPROX_EQUAL(0.8f, g1(0.1).get_r());
	ASSERT_APPROX_EQUAL(0.2f, g1(0.1).get_g());
	ASSERT_APPROX_EQUAL(0.0f, g1(0.1).get_b());
	// ASSERT(Color(.8, .2, 0., 1.) == g1(0.1));
}

void
test_gradient_3_colors_fetches_similar_to_the_last_color_for_position_near_to_the_last_position()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT_APPROX_EQUAL(0.0f, g1(0.9).get_r());
	ASSERT_APPROX_EQUAL_MICRO(0.2f, g1(0.9).get_g());
	ASSERT_APPROX_EQUAL(0.8f, g1(0.9).get_b());
}

void
test_gradient_3_colors_fetches_similar_to_the_middle_color_for_position_near_to_the_middle_position_from_left()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT_APPROX_EQUAL_MICRO(0.2f, g1(0.4).get_r());
	ASSERT_APPROX_EQUAL(0.8f, g1(0.4).get_g());
	ASSERT_APPROX_EQUAL(0.f, g1(0.4).get_b());
	// ASSERT(Color(.2, .8, 0., 1.) == g1(0.4));
}

void
test_gradient_3_colors_fetches_similar_to_the_middle_color_for_position_near_to_the_middle_position_from_right()
{
	Gradient g1(Color::red(), Color::green(), Color::blue());
	ASSERT_APPROX_EQUAL(0.0f, g1(0.6).get_r());
	ASSERT_APPROX_EQUAL(0.8f, g1(0.6).get_g());
	ASSERT_APPROX_EQUAL(0.2f, g1(0.6).get_b());
	// ASSERT(Color(0, .2, .8, 1.) == g1(0.6));
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	TEST_SUITE_BEGIN()
	TEST_FUNCTION(test_gradient_base_constructor_is_empty)
	TEST_FUNCTION(test_gradient_constructor_two_colors_keeps_the_order)
	TEST_FUNCTION(test_gradient_constructor_three_colors_keeps_the_order)
	TEST_FUNCTION(test_gradient_constructor_three_colors_let_the_second_in_the_middle)
	TEST_FUNCTION(test_gradient_0_color_fetches_the_empty_color_no_matter_position)
	TEST_FUNCTION(test_gradient_1_color_fetches_the_same_color_no_matter_position)

	TEST_FUNCTION(test_gradient_2_colors_fetches_the_first_color_even_if_before_first_position)
	TEST_FUNCTION(test_gradient_2_colors_fetches_the_last_color_even_if_after_last_position)
	TEST_FUNCTION(test_gradient_2_colors_fetches_the_first_color_for_the_exact_first_position)
	TEST_FUNCTION(test_gradient_2_colors_fetches_the_last_color_for_the_exact_last_position)
	TEST_FUNCTION(test_gradient_2_colors_fetches_the_middle_color_for_the_exact_middle_position)
	TEST_FUNCTION(test_gradient_2_colors_fetches_similar_to_the_first_color_for_position_near_to_the_first_position)
	TEST_FUNCTION(test_gradient_2_colors_fetches_similar_to_the_last_color_for_position_near_to_the_last_position)

	TEST_FUNCTION(test_gradient_3_colors_fetches_the_first_color_even_if_before_first_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_the_last_color_even_if_after_last_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_the_first_color_for_the_exact_first_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_the_last_color_for_the_exact_last_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_the_middle_color_for_the_exact_middle_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_similar_to_the_first_color_for_position_near_to_the_first_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_similar_to_the_last_color_for_position_near_to_the_last_position)
	TEST_FUNCTION(test_gradient_3_colors_fetches_similar_to_the_middle_color_for_position_near_to_the_middle_position_from_left)
	TEST_FUNCTION(test_gradient_3_colors_fetches_similar_to_the_middle_color_for_position_near_to_the_middle_position_from_right)
	TEST_SUITE_END()

	return tst_exit_status;
}
