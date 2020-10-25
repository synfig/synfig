/*! ========================================================================
** Extended Template and Library Test Suite
** Surface Class Test
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/surface>
#include <ETL/gaussian>
#include <cstdio>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

int display_pen(generic_pen<float> pen, int w, int h)
{
	int ret=0;
	int x, y;
	// print out the after pic
	for(y=0;y<h;y++,pen.inc_y())
	{
		printf("|");
		for(x=0;x<w;x++,pen.inc_x())
		{
			if(pen.get_value()>=2.0f)
				printf("#");
			else if(pen.get_value()>=1.0f)
				printf("@");
			else if(pen.get_value()>=0.8f)
				printf("%%");
			else if(pen.get_value()>=0.6f)
				printf("O");
			else if(pen.get_value()>=0.4f)
				printf(":");
			else if(pen.get_value()>=0.2f)
				printf(".");
			else if(pen.get_value()>=-0.1f)
				printf(" ");
			else
				printf("X"),ret++;
		}
		pen.dec_x(x);
		printf("|\n");
	}
	pen.dec_y(y);
	return ret;
}

void make_pattern(generic_pen<float> pen, int w, int h)
{
	int x,y;
	for(y=0;y<h;y++,pen.inc_y())
	{
		for(x=0;x<w;x++,pen.inc_x())
		{
			if( (x-y<=1 && y-x<=1) || y==h/2 || x==w/2)
				pen.put_value(2);
			else
				pen.put_value(0);
		}
		pen.dec_x(x);
	}
	pen.dec_y(y);
}

int basic_test()
{
	printf("Surface:basic_test(): Running...\n");

	int ret=0;

	surface<float> my_surface(100,100);

	gaussian_blur(my_surface.begin(),my_surface.end(),10,10);

	surface<float> my_surface2(my_surface);

	my_surface2.fill(0.5);
	my_surface2.clear();

	my_surface2=my_surface;

	my_surface2.fill(0.5);
	my_surface2.clear();

	my_surface.fill(0.5);
	my_surface.clear();

	surface<float> my_surface3;
	my_surface3.mirror(my_surface2);

	my_surface3.fill(0.5);
	my_surface3.clear();

	my_surface3=my_surface;

	my_surface3.mirror(my_surface);

	printf("Surface:basic_test(): %d errors.\n",ret);

	return ret;
}

int linear_sample_test()
{
	printf("Surface:linear_sample_test(): Running...\n");

	int ret=0;

	surface<float> my_surface(16,16);

	my_surface.fill(0.0f);

	make_pattern(my_surface.begin(),my_surface.get_w(),my_surface.get_h());

	int extra(5);
	surface<float> dest(18+extra*2,18+extra*2);

	int x,y;
	for(x=-extra;x<dest.get_w()-extra;x++)
		for(y=-extra;y<dest.get_h()-extra;y++)
		{
			dest[y+extra][x+extra]=my_surface.linear_sample(
				float(x)/float(dest.get_w()-1-extra*2)*float(my_surface.get_w()-1),
				float(y)/float(dest.get_h()-1-extra*2)*float(my_surface.get_h()-1)
			);
		}

	display_pen(dest.begin(),dest.get_w(),dest.get_h());

	printf("Surface:linear_sample_test(): %d errors.\n",ret);

	return ret;
}

int cubic_sample_test()
{
	printf("Surface:cubic_sample_test(): Running...\n");

	int ret=0;

	surface<float> my_surface(16,16);

	my_surface.fill(0.0f);

	make_pattern(my_surface.begin(),my_surface.get_w(),my_surface.get_h());

	{
		surface<float> dest(24,24);

		int x,y;
		for(x=0;x<dest.get_w();x++)
			for(y=0;y<dest.get_h();y++)
			{
				dest[y][x]=my_surface.cubic_sample(
					float(x)/float(dest.get_w()-1)*float(my_surface.get_w()-1),
					float(y)/float(dest.get_h()-1)*float(my_surface.get_h()-1)
				);
			}

		display_pen(dest.begin(),dest.get_w(),dest.get_h());
	}

	display_pen(my_surface.begin(),my_surface.get_w(),my_surface.get_h());
	{
		surface<float> dest(16,16);

		int x,y;
		for(x=0;x<dest.get_w();x++)
			for(y=0;y<dest.get_h();y++)
			{
				dest[y][x]=my_surface.cubic_sample(
					float(x)/float(dest.get_w()-1)*float(my_surface.get_w()-1),
					float(y)/float(dest.get_h()-1)*float(my_surface.get_h()-1)
				);
			}

		display_pen(dest.begin(),dest.get_w(),dest.get_h());
	}

	printf("Surface:cubic_sample_test(): %d errors.\n",ret);

	return ret;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=basic_test();
	error+=linear_sample_test();
	error+=cubic_sample_test();

	return error;
}
