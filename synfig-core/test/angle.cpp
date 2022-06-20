/*! ========================================================================
** Extended Template and Library Test Suite
** Angle Class Test
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

#include <stdio.h>
#include <synfig/angle.h>
#include <ETL/hermite>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */

int angle_test()
{
	int ret=0;
	float dist;

	dist=synfig::Angle::deg(synfig::Angle::deg(330).dist(synfig::Angle::deg(30))).get();
	printf("angle: angular difference between 330deg and 30deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=300)
	{
		printf("angle: error: should be 300deg!\n");
		ret++;
	}

	dist=synfig::Angle::deg(synfig::Angle::deg(30).dist(synfig::Angle::deg(330))).get();
	printf("angle: angular difference between 30deg and 330deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-300)
	{
		printf("angle: error: should be -300deg!\n");
		ret++;
	}

	dist=synfig::Angle::deg(synfig::Angle::deg(30).dist(synfig::Angle::deg(-30))).get();
	printf("angle: angular difference between 30deg and -30deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=60)
	{
		printf("angle: error: should be 60deg!\n");
		ret++;
	}

	dist=synfig::Angle::deg(synfig::Angle::deg(-30).dist(synfig::Angle::deg(30))).get();
	printf("angle: angular difference between -30deg and 30deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-60)
	{
		printf("angle: error: should be -60deg!\n");
		ret++;
	}

	dist=synfig::Angle::deg(synfig::Angle::deg(20).dist(synfig::Angle::deg(195))).get();
	printf("angle: angular difference between 20deg and 195deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-175)
	{
		printf("angle: error: should be -175deg!\n");
		ret++;
	}

	dist=synfig::Angle::deg(synfig::Angle::deg(20).dist(synfig::Angle::deg(205))).get();
	printf("angle: angular difference between 20deg and 205deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-185)
	{
		printf("angle: error: should be -185deg!\n");
		ret++;
	}

	int i;

	for(i=-1000;i<1000;i++)
	{
		dist=synfig::Angle::deg(synfig::Angle::deg(20+i+360).dist(synfig::Angle::deg(205+i-360))).get();
		if(floor(dist+0.5)!=535)
		{
			printf("angle: error: Badness at %d!\n",i);
			ret++;
		}

	}

	for(i=-1000;i<1000;i++)
	{
		dist=synfig::Angle::deg(synfig::Angle::deg(20+i-360).dist(synfig::Angle::deg(195+i+360))).get();
		if(floor(dist+0.5)!=-895)
		{
			printf("angle: error: Badness at %d!\n",i);
			ret++;
		}

	}



	{
		float f;
		synfig::Angle a(synfig::Angle::deg(-2005));
		synfig::Angle b(synfig::Angle::deg(200));

		affine_combo<synfig::Angle> combo;

		etl::hermite<synfig::Angle> hermie(a,b,b.dist(a),b.dist(a));

		for(f=0;f<1.001;f+=0.1)
		{
			printf("@%f--affine_combo: %f hermie: %f\n",synfig::Angle::deg(f).get(),synfig::Angle::deg(combo(a,b,f)).get(),synfig::Angle::deg(hermie(f)).get());
		}

	}

	return ret;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=angle_test();

	return error;
}
