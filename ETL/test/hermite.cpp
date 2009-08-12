/*! ========================================================================
** Extended Template and Library Test Suite
** Hermite Curve Test
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
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

#include <ETL/hermite>
#include <ETL/angle>
#include <ETL/clock>
#include <ETL/calculus>
#include <ETL/fixed>
#include <stdio.h>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

int basic_test(void)
{
	int ret=0;
	float f;

	hermite<float> Hermie;
	etl::clock timer;
	double t;

	Hermie.p1()=0;
	Hermie.t1()=1;
	Hermie.p2()=0;
	Hermie.t2()=1;

	Hermie.sync();

	integral<hermite<float> > inte(Hermie);


	fprintf(stderr,"integral of curve() on [0,1] = %f\n",inte(0,1.0));
	fprintf(stderr,"integral of curve() on [-1,3] = %f\n",inte(-1.0,3.0));
	Hermie.set_rs(-1.0,7.0);
	inte=integral<hermite<float> >(Hermie);
	fprintf(stderr,"integral of curve()[%f,%f] on [-1,7] = %f\n",Hermie.get_r(),Hermie.get_s(),inte(-1.0,7.0));
	fprintf(stderr,"integral of curve()[%f,%f] on [0,1] = %f\n",Hermie.get_r(),Hermie.get_s(),inte(0,1.0));
	Hermie.set_rs(0.0,1.0);


	for(f=0.0f,timer.reset();f<1.001f;f+=0.000005f)
	{
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
	}
	t=timer();

	fprintf(stderr,"time=%f milliseconds\n",t*1000);
	return ret;
}

int angle_test(void)
{
	int ret=0;
	float f;

	hermite<angle> Hermie;
	etl::clock timer;
	angle tmp;
	double t;

	Hermie.p1()=angle::degrees(0);
	Hermie.t1()=angle::degrees(45);

	Hermie.p2()=angle::degrees(-45);
	Hermie.t2()=angle::degrees(180);

	Hermie.sync();


	for(f=0.0f,timer.reset();f<1.001f;f+=0.000005f)
	{
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
	}
	t=timer();

	fprintf(stderr,"angle time=%f milliseconds\n",t*1000);

	return ret;
}

int fixed_test(void)
{
	int ret=0;
	float f;

	hermite<float,fixed> Hermie;
	etl::clock timer;
	double t;

	Hermie.p1()=0;
	Hermie.t1()=1;
	Hermie.p2()=0;
	Hermie.t2()=1;

	Hermie.sync();



	for(f=0.0f,timer.reset();f<1.001f;f+=0.005f)
	{
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
		t+=Hermie(f)+Hermie(f+0.1f);
	}
	t=timer();

	fprintf(stderr,"time=%f milliseconds\n",t*1000);
	return ret;
}


int anglefixed_test(void)
{
	int ret=0;
	float f;

	hermite<angle,fixed> Hermie;
	etl::clock timer;
	angle tmp;
	double t;

	Hermie.p1()=angle::degrees(0);
	Hermie.t1()=angle::degrees(45);

	Hermie.p2()=angle::degrees(-45);
	Hermie.t2()=angle::degrees(180);

	Hermie.sync();


	for(f=0.0f,timer.reset();f<1.001f;f+=0.0005f)
	{
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
		tmp+=Hermie(f)+Hermie(f+0.1f);
	}
	t=timer();

	fprintf(stderr,"angle fixed time=%f milliseconds\n",t*1000);

	return ret;
}

int float_intersection_test()
{
	int ret=0;

	hermite<float> curve1(0,1,0,1);
	hermite<float> curve2(-1,2,-1,-2);
	double t1,t2;
	float d;

	t1=curve1.intersect(curve2);
	t2=curve2.intersect(curve1);

	d=curve1(t1)-curve2(t2);

	fprintf(stderr,"float:Intersection difference: %f (t1=%f, t2=%f)\n",d,t1,t2);

	if(d>0.01)
	{
		fprintf(stderr,"float:FAILED INTERSECTION TEST.\n");
		ret++;
	}

	return ret;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=basic_test();
	error+=angle_test();
	error+=fixed_test();
	error+=float_intersection_test();
	return error;
}
