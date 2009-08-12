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
#include <ETL/fastangle>
#include <ETL/clock>
#include <ETL/bezier>
#include <ETL/hermite>

/* === M A C R O S ========================================================= */

ETL_FASTANGLE_INIT();

using namespace std;
using namespace etl;

/* === C L A S S E S ======================================================= */

int fastangle_test(void)
{
	int ret=0;
	float largest_error;

	{
		angle theta;
		fastangle theta2;
		float error;
		largest_error=0.0f;

		for(
			theta=angle::degrees(0),theta2=fastangle::degrees(0);
			theta<=angle::degrees(360);
			theta+=angle::degrees(10),theta2+=fastangle::degrees(10)
		)
		{
			error=(float)angle::sin(theta).get() -(float)fastangle::sin(theta2).get();
			/*
			fprintf(stderr,"angle: sin(%d)=%f ;\tfastangle: sin(%d)=%f ;\t diff: %f\n",
				(int)angle::degrees(theta),
				(float)angle::sin(theta),
				(int)fastangle::degrees(theta2),
				(float)fastangle::sin(theta2),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest SIN error: (+/-)%f\n",largest_error);
	if(largest_error>0.075)ret++;

	{
		angle theta;
		fastangle theta2;
		float error;
		largest_error=0.0f;

		for(
			theta=angle::degrees(0),theta2=fastangle::degrees(0);
			theta<=angle::degrees(360);
			theta+=angle::degrees(10),theta2+=fastangle::degrees(10)
		)
		{
			error=(float)angle::cos(theta).get() -(float)fastangle::cos(theta2).get();
			/*
			fprintf(stderr,"angle: cos(%d)=%f ;\tfastangle: cos(%d)=%f ;\t diff: %f\n",
				(int)angle::degrees(theta),
				(float)angle::cos(theta),
				(int)fastangle::degrees(theta2),
				(float)fastangle::cos(theta2),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest COS error: (+/-)%f\n",largest_error);
	if(largest_error>0.075)ret++;

	{
		double val;
		float error;
		largest_error=0.0f;

		for(
			val=-1.0f;
			val<1.0f;
			val+=0.01
		)
		{
			error=angle::radians(angle::sin(val)).get() -fastangle::radians(fastangle::sin(val)).get();
			/*
			fprintf(stderr,"angle: asin(%f)=%frad ;\tfastangle: asin(%f)=%frad ;\t diff: %f\n",
				val,
				(float)(angle::radians)angle::sin(val),
				val,
				(float)(fastangle::radians)fastangle::sin(val),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest ASIN error: (+/-)%frad\n",largest_error);
	if(largest_error>0.075)ret++;


	{
		double val;
		float error;
		largest_error=0.0f;

		for(
			val=-1.0f;
			val<1.0f;
			val+=0.01
		)
		{
			error=angle::radians(angle::cos(val)).get() -fastangle::radians(fastangle::cos(val)).get();
			/*
			fprintf(stderr,"angle: acos(%f)=%frad ;\tfastangle: acos(%f)=%frad ;\t diff: %f\n",
				val,
				(float)(angle::radians)angle::cos(val),
				val,
				(float)(fastangle::radians)fastangle::cos(val),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest ACOS error: (+/-)%frad\n",largest_error);
	if(largest_error>0.075)ret++;


	{
		angle theta;
		fastangle theta2;
		float error;
		largest_error=0.0f;

		for(
			theta=angle::degrees(0),theta2=fastangle::degrees(0);
			theta<angle::degrees(360);
			theta+=angle::degrees(38),theta2+=fastangle::degrees(38)
		)
		{
			error=angle::tan(theta).get() - fastangle::tan(theta2).get();
			/*
			fprintf(stderr,"angle: tan(%d)=%f ;\tfastangle: tan(%d)=%f ;\t diff: %f\n",
				(int)angle::degrees(theta),
				(float)angle::tan(theta),
				(int)fastangle::degrees(theta2),
				(float)fastangle::tan(theta2),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest TAN error: (+/-)%f\n",largest_error);
	if(largest_error>0.75)ret++;

	{
		double val;
		float error;
		largest_error=0.0f;

		for(
			val=-4.0f;
			val<4.0f;
			val+=0.1
		)
		{
			error=angle::radians(angle::tan(val)).get() -fastangle::radians(fastangle::tan(val)).get();
			/*
			fprintf(stderr,"angle: atan(%f)=%frad ;\tfastangle: atan(%f)=%frad ;\t diff: %f\n",
				val,
				(float)(angle::radians)angle::tan(val),
				val,
				(float)(fastangle::radians)fastangle::tan(val),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest ATAN error: (+/-)%frad\n",largest_error);
	if(largest_error>0.075)ret++;


	{
		angle theta;
		float error;
		largest_error=0.0f;

		for(
			theta=angle::degrees(-179);
			theta<angle::degrees(180);
			theta+=angle::degrees(5)
		)
		{
			error=angle::radians(theta).get()-fastangle::radians(fastangle::tan(angle::sin(theta).get(),angle::cos(theta).get())).get();
			/*
			fprintf(stderr,"\tfastangle: atan2(%f, %f)=%fdeg (%f) ;\t diff: %frad\n",
				(float)angle::sin(theta),
				(float)angle::cos(theta),
				(float)(fastangle::degrees)fastangle::tan(angle::sin(theta),angle::cos(theta)),
				(float)(fastangle::degrees)fastangle::tan((float)angle::sin(theta)/(float)angle::cos(theta)),
				error
				);
			*/
			if(error > largest_error)
				largest_error=error;
			if(error < -largest_error)
				largest_error=-error;

		}
	}
	printf("fastangle: Largest ATAN2 error: (+/-)%frad\n",largest_error);
	if(largest_error>0.075)ret++;

	printf("constant tests: %f==%f\n",
		(float)angle::degrees(angle::tan(1.01)).get(),
		(float)fastangle::degrees(fastangle::tan(1.01)).get());
	printf("constant tests: %f==%f\n",
		(float)angle::degrees(angle::tan(-1.0)).get(),
		(float)fastangle::degrees(fastangle::tan(-1.0)).get());

	return ret;
}

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

int fastangle_speed_test(void)
{
	int ret=0;
	float
		angle_cos_time,
		fastangle_cos_time,
		angle_tan_time,
		fastangle_tan_time,
		angle_atan2_time,
		fastangle_atan2_time,
		angle_sin_time,
		fastangle_sin_time ;

	etl::clock MyTimer;

	MyTimer.reset();
	angle_cos_speed_test<angle>();
	angle_cos_time=MyTimer();
	printf("angle: Cosine test: %f seconds\n",angle_cos_time);

	MyTimer.reset();
	angle_cos_speed_test<fastangle>();
	fastangle_cos_time=MyTimer();
	printf("fastangle: Cosine test: %f seconds\n",fastangle_cos_time);
	printf("fastangle is %.02f%% faster\n",(angle_cos_time/fastangle_cos_time)*100.0-100.0);

	MyTimer.reset();
	angle_sin_speed_test<angle>();
	angle_sin_time=MyTimer();
	printf("angle: Sine test: %f seconds\n",angle_sin_time);

	MyTimer.reset();
	angle_sin_speed_test<fastangle>();
	fastangle_sin_time=MyTimer();
	printf("fastangle: Sine test: %f seconds\n",fastangle_sin_time);
	printf("fastangle is %.02f%% faster\n",(angle_sin_time/fastangle_sin_time)*100.0-100.0);

	MyTimer.reset();
	angle_tan_speed_test<angle>();
	angle_tan_time=MyTimer();
	printf("angle: Tangent test: %f seconds\n",angle_tan_time);

	MyTimer.reset();
	angle_tan_speed_test<fastangle>();
	fastangle_tan_time=MyTimer();
	printf("fastangle: Tangent test: %f seconds\n",fastangle_tan_time);
	printf("fastangle is %.02f%% faster\n",(angle_tan_time/fastangle_tan_time)*100.0-100.0);

	MyTimer.reset();
	angle_atan2_speed_test<angle,angle::tan>();
	angle_atan2_time=MyTimer();
	printf("angle: arcTangent2 test: %f seconds\n",angle_atan2_time);

	MyTimer.reset();
	angle_atan2_speed_test<fastangle,fastangle::tan>();
	fastangle_atan2_time=MyTimer();
	printf("fastangle: arcTangent2 test: %f seconds\n",fastangle_atan2_time);
	printf("fastangle is %.02f%% faster\n",(angle_atan2_time/fastangle_atan2_time)*100.0-100.0);

	return ret;
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

	error+=fastangle_test();
	error+=fastangle_speed_test();
	error+=angle_test();

	return error;
}
