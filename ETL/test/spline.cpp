/*! ========================================================================
** Extended Template and Library Test Suite
** Spline Curve Test
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2010 Nikita Kitaev
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

#include <ETL/spline>
#include <ETL/angle>
#include <ETL/clock>
#include <ETL/calculus>
#include <stdio.h>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

int bspline_basic_test(void)
{
	int ret=0;
	float f;

	bspline<float> BSpline;
	etl::clock timer;
	double t;

	BSpline.cpoints().insert(BSpline.cpoints().end(), 0.0);
	BSpline.cpoints().insert(BSpline.cpoints().end(), -1.0);
	BSpline.cpoints().insert(BSpline.cpoints().end(), 0.0);
	BSpline.cpoints().insert(BSpline.cpoints().end(), 1.0);
	BSpline.cpoints().insert(BSpline.cpoints().end(), 0.0);

	BSpline.set_m(4);
	BSpline.reset_knots();

	integral<bspline<float> > inte(BSpline);


	/*
	for(f=0.0;f<1.001;f+=0.05)
		fprintf(stderr,"BSpline(%f)= %f\n",f,BSpline(f));
	*/

	fprintf(stderr,"integral of BSpline() on [0,1] = %f\n",inte(0,1.0));


	for(f=0.0f,timer.reset();f<1.001f;f+=0.000005f)
	{
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
		t+=BSpline(f)+BSpline(f+0.1f);
	}
	t=timer();

	fprintf(stderr,"BSpline time=%f milliseconds\n",t*1000);
	return ret;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=bspline_basic_test();

	return error;
}

