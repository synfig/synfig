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

/* === E N T R Y P O I N T ================================================= */

int main()
{
	TEST_SUITE_BEGIN()

	TEST_FUNCTION(test_basic);
	TEST_FUNCTION(test_float_intersection_reciprocity);

	TEST_SUITE_END()

	return tst_exit_status;
}
