/* === S Y N F I G ========================================================= */
/*!	\file distance.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <ETL/stringf>

#include "distance.h"
#include "renddesc.h"
#include "general.h"
#include <synfig/localization.h>
#include <ctype.h>
#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define POINTS_PER_INCH		  (72.0)
#define INCHES_PER_METER	  (39.3700787402)
#define POINTS_PER_METER	  (POINTS_PER_INCH*INCHES_PER_METER)
#define CENTIMETERS_PER_METER (100.0)
#define MILLIMETERS_PER_METER (1000.0)

#define METERS_PER_UNIT		  (rend_desc.get_physical_w()/std::abs(rend_desc.get_tl()[0]-rend_desc.get_br()[0]))

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Distance::Distance(const synfig::String& str): value_(), system_()
{
	(*this)=str;
/*	int i(0);
	float val;
	int ret(strscanf(str,"%f%n",&val,&i));
	synfig::info("Distance::Distance(): ret=%d, val=%f",ret,val);

	if(ret<=0)
	{
		// Error
		synfig::error("Distance::Distance(): Bad value \"%s\"",str.c_str());
		value_=0;
	}
	else
		value_=val;

	synfig::info("Distance::Distance(): system=\"%s\"",String(str.begin()+i,str.end()).c_str());
	system_=ident_system(String(str.begin()+i,str.end()));
*/
}

Distance&
Distance::operator=(const synfig::String& str)
{
	int i(0);
	float val;
	int ret(strscanf(str,"%f%n",&val,&i));
	// synfig::info("Distance::Distance(): ret=%d, val=%f",ret,val);

	if(ret<=0)
	{
		// Error
		synfig::error("Distance::Distance(): Bad value \"%s\"",str.c_str());
		return *this;
	}
	else
		value_=val;

	synfig::String sys(str.begin()+i,str.end());

	if(sys.size())
		system_=ident_system(sys);
	else
	// this would throw BadSystem if system_ was not previously provided
		sys=system_name(system_);
	return *this;
}

synfig::String
Distance::get_string(int digits)const
{
	digits=std::min(9, std::max(0,digits));
	String fmt(strprintf("%%.%01df",digits));
	String str(strprintf(fmt.c_str(),value_));
	return strprintf("%s%s",str.c_str(),system_name(system_).c_str());
}

void
Distance::convert(Distance::System target, const RendDesc& rend_desc)
{
	value_=get(target,rend_desc);
	system_=target;
}

Real
Distance::get(Distance::System target, const RendDesc& rend_desc)const
{
	if(target==SYSTEM_UNITS)
		return units(rend_desc);
	if(target==SYSTEM_PIXELS)
		return units(rend_desc)*METERS_PER_UNIT*rend_desc.get_x_res();

	return meters_to_system(meters(rend_desc),target);
}

Real
Distance::meters()const
{
	switch(system_)
	{
		case SYSTEM_INCHES:		 return value_/INCHES_PER_METER;
		case SYSTEM_POINTS:		 return value_/POINTS_PER_METER;
		case SYSTEM_METERS:		 return value_;
		case SYSTEM_CENTIMETERS: return value_/CENTIMETERS_PER_METER;
		case SYSTEM_MILLIMETERS: return value_/MILLIMETERS_PER_METER;
		default: throw BadSystem();
	}
}

Real
Distance::meters(const RendDesc& rend_desc)const
{
	if (system_ == SYSTEM_UNITS)  return value_*METERS_PER_UNIT;
	if (system_ == SYSTEM_PIXELS) return value_/rend_desc.get_x_res();
	if (system_ > SYSTEM_PIXELS)  return meters();

	throw BadSystem();
}

Real
Distance::units(const RendDesc& rend_desc)const
{
	if(system_==SYSTEM_UNITS)
		return value_;

	Real ret;

	if(system_>SYSTEM_PIXELS)
		ret=meters();
	else
		ret=value_/rend_desc.get_x_res();

	return ret/METERS_PER_UNIT;
}


Real // (static)
Distance::meters_to_system(Real x,System target_system)
{
	switch(target_system)
	{
		case SYSTEM_INCHES:		 return x*INCHES_PER_METER;
		case SYSTEM_POINTS:		 return x*POINTS_PER_METER;
		case SYSTEM_METERS:		 return x;
		case SYSTEM_CENTIMETERS: return x*CENTIMETERS_PER_METER;
		case SYSTEM_MILLIMETERS: return x*MILLIMETERS_PER_METER;
		default: throw BadSystem();
	}
}

Distance::System // (static)
Distance::ident_system(const synfig::String& x)
{
	synfig::String str;

	// Make it all upper case, and remove white space
	for(unsigned int i=0;i<x.size();i++)if(x[i]!=' ' && x[i]!='\t')str+=toupper(x[i]);

	// If it is plural, make it singular
	if(str[str.size()-1]=='S')
		str=synfig::String(str.begin(),str.end()-1);

	if(str.empty() || str=="U" || str=="UNIT")
		return SYSTEM_UNITS;
	if(str=="PX" || str=="PIXEL")
		return SYSTEM_PIXELS;
	if(str=="PT" || str=="POINT")
		return SYSTEM_POINTS;
	if(str=="IN" || str=="\"" || str=="INCHE" || str=="INCH")
		return SYSTEM_INCHES;
	if(str=="M" || str=="METER")
		return SYSTEM_METERS;
	if(str=="CM" || str=="CENTIMETER")
		return SYSTEM_CENTIMETERS;
	if(str=="MM" || str=="MILLIMETER")
		return SYSTEM_MILLIMETERS;

	synfig::warning("Distance::ident_system(): Unknown distance system \"%s\"",x.c_str());

	return SYSTEM_UNITS;
}

synfig::String  // (static)
Distance::system_name(Distance::System system)
{
	switch(system)
	{
		case SYSTEM_UNITS:		 return "u";
		case SYSTEM_PIXELS:		 return "px";
		case SYSTEM_POINTS:		 return "pt";
		case SYSTEM_INCHES:		 return "in";
		case SYSTEM_METERS:		 return "m";
		case SYSTEM_MILLIMETERS: return "mm";
		case SYSTEM_CENTIMETERS: return "cm";

		default:				 throw BadSystem();
	}
}

synfig::String  // (static)
Distance::system_local_name(Distance::System system)
{
	switch(system)
	{
		case SYSTEM_UNITS:		return _("Units");
		case SYSTEM_PIXELS:		return _("Pixels");
		case SYSTEM_POINTS:		return _("Points");
		case SYSTEM_INCHES:		return _("Inches");
		case SYSTEM_METERS:		return _("Meters");
		case SYSTEM_MILLIMETERS:return _("Millimeters");
		case SYSTEM_CENTIMETERS:return _("Centimeters");

		default:				throw BadSystem();
	}
	return synfig::String();
}
