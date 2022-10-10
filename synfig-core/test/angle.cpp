/* === S Y N F I G ========================================================= */
/*! \file angle.cpp
**  \brief Test Angle class
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
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

#include <synfig/angle.h>
#include <synfig/bezier.h>

#include "test_base.h"

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === C L A S S E S ======================================================= */

void test_angle_dist_between_bigger_and_smaller_angle()
{
	float dist = Angle::deg(Angle::deg(330).dist(Angle::deg(30))).get();
	ASSERT_EQUAL(300, floor(dist+0.5))
}

void test_angle_dist_between_smaller_and_bigger_angle()
{
	float dist = Angle::deg(Angle::deg(30).dist(Angle::deg(330))).get();
	ASSERT_EQUAL(-300, floor(dist+0.5))
}

void test_angle_dist_between_positive_and_negative_angle()
{
	float dist = Angle::deg(Angle::deg(30).dist(Angle::deg(-30))).get();
	ASSERT_EQUAL(60, floor(dist+0.5))
}

void test_angle_dist_between_negative_and_positive_angle()
{
	float dist = Angle::deg(Angle::deg(-30).dist(Angle::deg(30))).get();
	ASSERT_EQUAL(-60, floor(dist+0.5))
}

void test_angle_dist_almost_minus_180d()
{
	float dist = Angle::deg(Angle::deg(20).dist(Angle::deg(195))).get();
	ASSERT_EQUAL(-175, floor(dist+0.5))
}

void test_angle_dist_little_less_than_minus_180d_do_not_change_signal()
{
	float dist = Angle::deg(Angle::deg(20).dist(Angle::deg(205))).get();
	ASSERT_EQUAL(-185, floor(dist+0.5))
}

void test_angle_dist_for_multiple_rotations_1()
{
	for(int i=-1000;i<1000;i++)
	{
		float dist=Angle::deg(Angle::deg(20+i+360).dist(Angle::deg(205+i-360))).get();
		ASSERT_EQUAL(535, floor(dist+0.5))
	}
}

void test_angle_dist_for_multiple_rotations_2()
{
	for(int i=-1000;i<1000;i++)
	{
		float dist=Angle::deg(Angle::deg(20+i-360).dist(Angle::deg(195+i+360))).get();
		ASSERT_EQUAL(-895, floor(dist+0.5))
	}
}

void test_angle_affine_combo_and_hermite()
{
	Angle a(Angle::deg(-2005));
	Angle b(Angle::deg(200));

	affine_combo<Angle> combo;

	hermite<Angle> hermie(a,b,b.dist(a),b.dist(a));

	for(float f=0;f<1.001;f+=0.1)
	{
		printf("@%f--affine_combo: %f hermie: %f\n",Angle::deg(f).get(),Angle::deg(combo(a,b,f)).get(),Angle::deg(hermie(f)).get());
	}
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	TEST_SUITE_BEGIN()
	TEST_FUNCTION(test_angle_dist_between_bigger_and_smaller_angle)
	TEST_FUNCTION(test_angle_dist_between_smaller_and_bigger_angle)
	TEST_FUNCTION(test_angle_dist_between_positive_and_negative_angle)
	TEST_FUNCTION(test_angle_dist_between_negative_and_positive_angle)
	TEST_FUNCTION(test_angle_dist_almost_minus_180d)
	TEST_FUNCTION(test_angle_dist_little_less_than_minus_180d_do_not_change_signal)
	TEST_FUNCTION(test_angle_dist_for_multiple_rotations_1)
	TEST_FUNCTION(test_angle_dist_for_multiple_rotations_2)
	TEST_FUNCTION(test_angle_affine_combo_and_hermite)
	TEST_SUITE_END()

	return tst_exit_status;
}
