/*! ========================================================================
** Extended Template and Library Test Suite
** Angle Class Test
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <stdio.h>
#include <ETL/angle>
#include <ETL/clock>
#include <ETL/bezier>
#include <ETL/hermite>

/* === M A C R O S ========================================================= */

using namespace std;
using namespace etl;

/* === C L A S S E S ======================================================= */

template <class Angle>
void angle_cos_speed_test(void)
{
	Angle a,b,c,d;
	float tmp,tmp2;

	for(tmp=-1.0;tmp<1.0;tmp+=0.000002)
	{
		a=(typename Angle::cos)(tmp);
		b=(typename Angle::cos)(tmp);
		c=(typename Angle::cos)(tmp);
		d=(typename Angle::cos)(tmp);
		tmp2=((typename Angle::cos)(a)).get();
		tmp2=((typename Angle::cos)(b)).get();
		tmp2=((typename Angle::cos)(c)).get();
		tmp2=((typename Angle::cos)(d)).get();
	}
	if (tmp2 == 0) return; // disable unused warning
}
template <class Angle>
void angle_sin_speed_test(void)
{
	Angle a,b,c,d;
	float tmp,tmp2;

	for(tmp=-1.0;tmp<1.0;tmp+=0.000002)
	{
		a=(typename Angle::sin)(tmp);
		b=(typename Angle::sin)(tmp);
		c=(typename Angle::sin)(tmp);
		d=(typename Angle::sin)(tmp);
		tmp2=((typename Angle::sin)(a)).get();
		tmp2=((typename Angle::sin)(b)).get();
		tmp2=((typename Angle::sin)(c)).get();
		tmp2=((typename Angle::sin)(d)).get();
	}
	if (tmp2 == 0) return; // disable unused warning
}
template <class Angle>
void angle_tan_speed_test(void)
{
	Angle a,b,c,d;
	float tmp,tmp2;

	for(tmp=-1.0;tmp<1.0;tmp+=0.000002)
	{
		a=(typename Angle::tan)(tmp);
		b=(typename Angle::tan)(tmp);
		c=(typename Angle::tan)(tmp);
		d=(typename Angle::tan)(tmp);
		tmp2=((typename Angle::tan)(a)).get();
		tmp2=((typename Angle::tan)(b)).get();
		tmp2=((typename Angle::tan)(c)).get();
		tmp2=((typename Angle::tan)(d)).get();
	}
	if (tmp2 == 0) return; // disable unused warning
}
template <class Angle, class mytan>
void angle_atan2_speed_test(void)
{
	Angle a,b,c;
	float x,y;

	for(y=-10.0;y<10.0;y+=0.05)
		for(x=-10.0;x<10.0;x+=0.05)
		{
			a=mytan(y,x);
			a=mytan(x,y);
			b=mytan(y,x);
			b=mytan(x,y);
			c=mytan(y,x);
			c=mytan(x,y);
			a=mytan(y,x);
			a=mytan(x,y);
			b=mytan(y,x);
			b=mytan(x,y);
			c=mytan(y,x);
			c=mytan(x,y);
		}
}

int angle_test()
{
	int ret=0;
	float dist;

	dist=angle::deg(angle::deg(330).dist(angle::deg(30))).get();
	printf("angle: angular difference between 330deg and 30deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=300)
	{
		printf("angle: error: should be 300deg!\n");
		ret++;
	}

	dist=angle::deg(angle::deg(30).dist(angle::deg(330))).get();
	printf("angle: angular difference between 30deg and 330deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-300)
	{
		printf("angle: error: should be -300deg!\n");
		ret++;
	}

	dist=angle::deg(angle::deg(30).dist(angle::deg(-30))).get();
	printf("angle: angular difference between 30deg and -30deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=60)
	{
		printf("angle: error: should be 60deg!\n");
		ret++;
	}

	dist=angle::deg(angle::deg(-30).dist(angle::deg(30))).get();
	printf("angle: angular difference between -30deg and 30deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-60)
	{
		printf("angle: error: should be -60deg!\n");
		ret++;
	}

	dist=angle::deg(angle::deg(20).dist(angle::deg(195))).get();
	printf("angle: angular difference between 20deg and 195deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-175)
	{
		printf("angle: error: should be -175deg!\n");
		ret++;
	}

	dist=angle::deg(angle::deg(20).dist(angle::deg(205))).get();
	printf("angle: angular difference between 20deg and 205deg is %0.1fdeg\n",dist);
	if(floor(dist+0.5)!=-185)
	{
		printf("angle: error: should be -185deg!\n");
		ret++;
	}

	int i;

	for(i=-1000;i<1000;i++)
	{
		dist=angle::deg(angle::deg(20+i+360).dist(angle::deg(205+i-360))).get();
		if(floor(dist+0.5)!=535)
		{
			printf("angle: error: Badness at %d!\n",i);
			ret++;
		}

	}

	for(i=-1000;i<1000;i++)
	{
		dist=angle::deg(angle::deg(20+i-360).dist(angle::deg(195+i+360))).get();
		if(floor(dist+0.5)!=-895)
		{
			printf("angle: error: Badness at %d!\n",i);
			ret++;
		}

	}



	{
		float f;
		angle a(angle::deg(-2005));
		angle b(angle::deg(200));

		affine_combo<angle> combo;

		hermite<angle> hermie(a,b,b.dist(a),b.dist(a));

		for(f=0;f<1.001;f+=0.1)
		{
			printf("@%f--affine_combo: %f hermie: %f\n",angle::deg(f).get(),angle::deg(combo(a,b,f)).get(),angle::deg(hermie(f)).get());
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
