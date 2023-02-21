/* === S Y N F I G ========================================================= */
/*! \file hermite.cpp
**  \brief Hermite Curve Test
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <synfig/bezier.h>

#include "test_base.h"

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

void test_basic()
{
	hermite<float> Hermie;

	Hermie.p1()=0;
	Hermie.t1()=1;
	Hermie.p2()=2;
	Hermie.t2()=1;

	Hermie.sync();

	ASSERT_APPROX_EQUAL(0.f, Hermie[0])
	ASSERT_APPROX_EQUAL(2.f, Hermie[3])
	ASSERT_APPROX_EQUAL(0.f, Hermie.get_r())
	ASSERT_APPROX_EQUAL(1.f, Hermie.get_s())

	// TODO: replace code below with one that tests derivative()

//	integral<hermite<float> > inte(Hermie);

//	fprintf(stderr,"integral of curve() on [0,1] = %f\n",inte(0,1.0));
//	fprintf(stderr,"integral of curve() on [-1,3] = %f\n",inte(-1.0,3.0));
//	Hermie.set_rs(-1.0,7.0);
//	inte=integral<hermite<float> >(Hermie);
//	fprintf(stderr,"integral of curve()[%f,%f] on [-1,7] = %f\n",Hermie.get_r(),Hermie.get_s(),inte(-1.0,7.0));
//	fprintf(stderr,"integral of curve()[%f,%f] on [0,1] = %f\n",Hermie.get_r(),Hermie.get_s(),inte(0,1.0));
//	Hermie.set_rs(0.0,1.0);
}

void test_float_intersection_reciprocity()
{
	hermite<float> curve1(0,1,0,1);
	hermite<float> curve2(-1,2,-1,-2);
	double t1,t2;
	float d;

	t1=curve1.intersect(curve2);
	t2=curve2.intersect(curve1);

	d=curve1(t1)-curve2(t2);

	ASSERT_APPROX_EQUAL_MICRO(0.f, d);
}

void test_initial_value_for_initial_parameter()
{
	hermite<double> curve1(3,4,1,1);

	ASSERT_APPROX_EQUAL_MICRO(3., curve1(0));
}

void test_final_value_for_final_parameter()
{
	hermite<double> curve1(3,4,1,1);

	ASSERT_APPROX_EQUAL_MICRO(4., curve1(1));
}

void test_mid_value_for_mid_parameter_on_straight_line()
{
	hermite<double> curve1(3,4,0,0);

	ASSERT_APPROX_EQUAL_MICRO(3.5, curve1(0.5));
}

void test_initial_value_for_modified_initial_parameter()
{
	hermite<double> curve1(3,4,1,1);
	curve1.set_r(-1);

	ASSERT_APPROX_EQUAL_MICRO(3., curve1(-1));
}

void test_final_value_for_modified_final_parameter()
{
	hermite<double> curve1(3,4,1,1);
	curve1.set_s(2);

	ASSERT_APPROX_EQUAL_MICRO(4., curve1(2));
}

void test_mid_value_for_modified_mid_parameter_on_straight_line()
{
	hermite<double> curve1(3,4,0,0);
	curve1.set_rs(-2, 4);

	ASSERT_APPROX_EQUAL_MICRO(3.5, curve1(1));
}

void test_length_on_straight_line()
{
	hermite<double> curve1(3,4,0,0);

	ASSERT_APPROX_EQUAL_MICRO(1, curve1.length());

	curve1.p2() = 7;
	curve1.sync();
	ASSERT_APPROX_EQUAL_MICRO(4, curve1.length());

	curve1.p2() = -2;
	curve1.sync();
	ASSERT_APPROX_EQUAL_MICRO(5, curve1.length());
}

void test_length_on_straight_line_segment()
{
	hermite<double> curve1(5,6,1,1);

	ASSERT_APPROX_EQUAL_MICRO(.5, curve1.find_distance(0.25,.75));

	hermite<double> curve2(-5,6,11,11);

	ASSERT_APPROX_EQUAL_MICRO(5.5, curve2.find_distance(0.25,.75));
}

void test_subdivide_at_initial_parameter()
{
	hermite<double> curve1(3,4,0,0);
	hermite<double> s, f;
	curve1.subdivide(&s, &f, 0.);
	ASSERT_APPROX_EQUAL(0.f, s.length());
	ASSERT_APPROX_EQUAL(1.f, f.length());
}

void test_subdivide_at_final_parameter()
{
	hermite<double> curve1(3,4,0,0);
	hermite<double> s, f;
	curve1.subdivide(&s, &f, 1.);
	ASSERT_APPROX_EQUAL(1.f, s.length());
	ASSERT_APPROX_EQUAL(0.f, f.length());
}

void test_subdivide_at_midpoint()
{
	hermite<double> curve1(3,4,0,0);
	hermite<double> s, f;
	curve1.subdivide(&s, &f, .5);
	ASSERT_APPROX_EQUAL(0.5f, s.length());
	ASSERT_APPROX_EQUAL(0.5f, f.length());
}

void test_subdivide_at_some_point()
{
	hermite<double> curve1(3,4,1,1);
	bezier<double> s, f;
	const double subdiv_parameter = 0.3;
	curve1.subdivide(&s, &f, subdiv_parameter);
	ASSERT_APPROX_EQUAL(curve1.length(), s.length() + f.length());
	// keep end poinds
	ASSERT_APPROX_EQUAL(3., s[0]);
	ASSERT_APPROX_EQUAL(4., f[3]);
	// same subdivision point
	ASSERT_APPROX_EQUAL(s[3], f[0]);
	ASSERT_APPROX_EQUAL(curve1(subdiv_parameter), s[3]);
	// keep end point tangents
	hermite<double> hs(s), hf(f);
	ASSERT_APPROX_EQUAL_MICRO(.3, hs.t1());
	ASSERT_APPROX_EQUAL_MICRO(.3, hs.t2());
	ASSERT_APPROX_EQUAL_MICRO(.7, hf.t2());
	ASSERT_APPROX_EQUAL_MICRO(.7, hf.t1());
}

void test_convert_bezier_to_hermite()
{
	hermite<double> curve1(3,4,1,.5);
	bezier<double> bez(curve1);
	hermite<double> curve2(bez);

	ASSERT_EQUAL(3., curve2.P1);
	ASSERT_EQUAL(4., curve2.P2);
	ASSERT_APPROX_EQUAL(1., curve2.T1);
	ASSERT_APPROX_EQUAL(.5, curve2.T2);
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	TEST_SUITE_BEGIN()

	TEST_FUNCTION(test_basic);
	TEST_FUNCTION(test_float_intersection_reciprocity);

	TEST_FUNCTION(test_initial_value_for_initial_parameter);
	TEST_FUNCTION(test_final_value_for_final_parameter);
	TEST_FUNCTION(test_mid_value_for_mid_parameter_on_straight_line);

	TEST_FUNCTION(test_initial_value_for_modified_initial_parameter);
	TEST_FUNCTION(test_final_value_for_modified_final_parameter);
	TEST_FUNCTION(test_mid_value_for_modified_mid_parameter_on_straight_line);

	TEST_FUNCTION(test_length_on_straight_line);
	TEST_FUNCTION(test_length_on_straight_line_segment);

	TEST_FUNCTION(test_subdivide_at_initial_parameter);
	TEST_FUNCTION(test_subdivide_at_final_parameter);
	TEST_FUNCTION(test_subdivide_at_midpoint);
	TEST_FUNCTION(test_subdivide_at_some_point);

	TEST_FUNCTION(test_convert_bezier_to_hermite);

	TEST_SUITE_END()

	return tst_exit_status;
}
