/* === S Y N F I G ========================================================= */
/*! \file gtest_angle.cpp
**  \brief Test Angle class (Google Test)
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

#include <gtest/gtest.h>

#include <synfig/angle.h>
#include <synfig/bezier.h>

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === T E S T S =========================================================== */

TEST(AngleTest, DistBetweenBiggerAndSmallerAngle)
{
	float dist = Angle::deg(Angle::deg(330).dist(Angle::deg(30))).get();
	EXPECT_EQ(300, floor(dist+0.5));
}

TEST(AngleTest, DistBetweenSmallerAndBiggerAngle)
{
	float dist = Angle::deg(Angle::deg(30).dist(Angle::deg(330))).get();
	EXPECT_EQ(-300, floor(dist+0.5));
}

TEST(AngleTest, DistBetweenPositiveAndNegativeAngle)
{
	float dist = Angle::deg(Angle::deg(30).dist(Angle::deg(-30))).get();
	EXPECT_EQ(60, floor(dist+0.5));
}

TEST(AngleTest, DistBetweenNegativeAndPositiveAngle)
{
	float dist = Angle::deg(Angle::deg(-30).dist(Angle::deg(30))).get();
	EXPECT_EQ(-60, floor(dist+0.5));
}

TEST(AngleTest, DistAlmostMinus180d)
{
	float dist = Angle::deg(Angle::deg(20).dist(Angle::deg(195))).get();
	EXPECT_EQ(-175, floor(dist+0.5));
}

TEST(AngleTest, DistLittleLessThanMinus180dDoNotChangeSignal)
{
	float dist = Angle::deg(Angle::deg(20).dist(Angle::deg(205))).get();
	EXPECT_EQ(-185, floor(dist+0.5));
}

TEST(AngleTest, DistForMultipleRotations1)
{
	for(int i=-1000;i<1000;i++)
	{
		float dist=Angle::deg(Angle::deg(20+i+360).dist(Angle::deg(205+i-360))).get();
		EXPECT_EQ(535, floor(dist+0.5));
	}
}

TEST(AngleTest, DistForMultipleRotations2)
{
	for(int i=-1000;i<1000;i++)
	{
		float dist=Angle::deg(Angle::deg(20+i-360).dist(Angle::deg(195+i+360))).get();
		EXPECT_EQ(-895, floor(dist+0.5));
	}
}

TEST(AngleTest, AffineComboAndHermite)
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
