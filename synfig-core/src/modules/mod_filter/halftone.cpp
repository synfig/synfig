/* === S Y N F I G ========================================================= */
/*!	\file halftone.cpp
**	\brief blehh
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "halftone.h"

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */


/* === P R O C E D U R E S ================================================= */

#define SQRT2	(1.414213562f)

/* === M E T H O D S ======================================================= */

float
Halftone::operator()(const Point &point, const float& luma, float supersample)const
{
	float halftone(mask(point));

	if(supersample>=0.5f)
		supersample=0.4999999999f;

	halftone=(halftone-0.5f)*(1.0f-supersample*2.0f)+0.5f;

	const float diff(halftone-luma);

	if(supersample)
	{
		const float amount(diff/(supersample*2.0f)+0.5f);

		if(amount<=0.0f+0.01f)
			return 1.0f;
		else if(amount>=1.0f-0.01f)
			return 0.0f;
		else
			return 1.0f-amount;
	}
	else
	{
		if(diff>=0)
			return 0.0f;
		else
			return 1.0f;
	}

	return 0.0f;
}

float
Halftone::mask(synfig::Point point)const
{
	int type=param_type.get(int());
	Point origin=param_origin.get(Point());
	Vector size=param_size.get(Vector());
	Angle angle=param_angle.get(Angle());
	
	float radius1;
	float radius2;

	point-=origin;

	{
		const float	a(Angle::sin(-angle).get()),	b(Angle::cos(-angle).get());
		const float	u(point[0]),v(point[1]);

		point[0]=b*u-a*v;
		point[1]=a*u+b*v;
	}

	if(type==TYPE_STRIPE)
	{
		Point pnt(fmod(point[0],size[0]),fmod(point[1],size[1]));
		while(pnt[0]<0)pnt[0]+=abs(size[0]);
		while(pnt[1]<0)pnt[1]+=abs(size[1]);

		float x(pnt[1]/size[1]);
		if(x>0.5)x=1.0-x;
		x*=2;
		return x;
	}

	{
		Point pnt(fmod(point[0],size[0]),fmod(point[1],size[1]));
		while(pnt[0]<0)pnt[0]+=abs(size[0]);
		while(pnt[1]<0)pnt[1]+=abs(size[1]);
		pnt-=Vector(size[0]*0.5,size[1]*0.5);
		pnt*=2.0;
		pnt[0]/=size[0];
		pnt[1]/=size[1];

		radius1=pnt.mag()/SQRT2;
		radius1*=radius1;
	}
	if(type==TYPE_DARKONLIGHT || type== TYPE_LIGHTONDARK)
		return radius1;

	{
		Point pnt(fmod(point[0]+size[0]*0.5,size[0]),fmod(point[1]+size[0]*0.5,size[1]));
		while(pnt[0]<0)pnt[0]+=abs(size[0]);
		while(pnt[1]<0)pnt[1]+=abs(size[1]);
		pnt-=Vector(size[0]*0.5,size[1]*0.5);
		pnt*=2.0;
		pnt[0]/=size[0];
		pnt[1]/=size[1];

		radius2=pnt.mag()/SQRT2;
		radius2*=radius2;
	}

	if(type==TYPE_DIAMOND)
	{
		//return (radius1+(1.0f-radius2))*0.5;
		float x((radius1+(1.0f-radius2))*0.5);
		//float x(((radius2-radius1)*((radius1+(1.0f-radius2))*0.5)+radius1)*2.0f);
		x-=0.5;
		x*=2.0;
		if(x<0)x=-sqrt(-x);else x=sqrt(x);
		x*=1.01f;
		x/=2.0;
		x+=0.5;
		return x;
	}

	if(type==TYPE_SYMMETRIC)
	{
		float x(((radius2-radius1)*((radius1+(1.0f-radius2))*0.5)+radius1)*2.0f);
		x-=0.5;
		x*=2.0;
		if(x<0)x=-sqrt(-x);else x=sqrt(x);
		x*=1.01f;
		x/=2.0;
		x+=0.5;
		return x;
	}
	return 0;
}
