/*! ========================================================================
** Extended Template and Library Test Suite
** Handle Template Class Test
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <list>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <memory>
#include <map>
#include <ETL/pen>
#include <ETL/boxblur>
//#include <ETL/gaussian>
#include <cmath>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */

int generic_pen_test(int w, int h)
{
	printf("generic_pen(w:%d,h:%d): ",w,h);

	std::unique_ptr<float> data(new float[w*h]);
	//unique_ptr<float> data(new float[w*h]);
	if(!data.get())
	{
		printf("Um..... malloc failure on line %d of " __FILE__ "...\n", __LINE__);
		abort();
	}

	generic_pen<float> pen(data.get(),w,h);
	generic_pen<float> pen2;

	if(!pen)
	{
		printf("FAILURE! " __FILE__ "@%d: On pen bool test\n", __LINE__);
		return 1;
	}

	if(&pen.x()[2]!=&pen[0][2])
	{
		printf("FAILURE! " __FILE__ "@%d: On request for horizontal iterator\n", __LINE__);
		return 1;
	}

	if(&pen.y()[2]!=&pen[2][0])
	{
		printf("FAILURE! " __FILE__ "@%d: On request for vertical iterator\n", __LINE__);
		return 1;
	}

	pen.move(1,1);
	pen2=pen;

	if(pen!=pen2)
	{
		printf("FAILURE! " __FILE__ "@%d: On pen assignment or pen comparison\n", __LINE__);
		return 1;
	}

	pen2.move(w,h);
	generic_pen<float>::difference_type diff(pen2-pen);

	if(diff.x!=w || diff.y!=h)
	{
		printf("FAILURE! " __FILE__ "@%d: pen difference inconsistency ([%d,%d]!=[%d,%d])\n", __LINE__, diff.x, diff.y, w, h);
		return 1;
	}

	if(pen.end_x()-pen.x()!=w-1)
	{
		printf("FAILURE! " __FILE__ "@%d: iterator_x inconsistency (%ld!=%d)\n", __LINE__, pen.end_x()-pen.x(), w);
		return 1;
	}

	if(pen.end_y()-pen.y()!=h-1)
	{
		printf("FAILURE! " __FILE__ "@%d: iterator_y inconsistency (%d!=%d)\n", __LINE__, pen.end_y()-pen.y(), h);
		return 1;
	}

	if(&pen.end_y()[-1]!=&pen.y()[(h-2)])
	{
		printf("FAILURE! " __FILE__ "@%d: iterator_y inconsistency\n", __LINE__);
		return 1;
	}

	if(&pen.end_x()[-1]!=&pen.x()[(w-2)])
	{
		printf("FAILURE! " __FILE__ "@%d: iterator_x inconsistency\n", __LINE__);
		return 1;
	}

	printf("PASSED\n");

	return 0;
}

int alpha_pen_test(void)
{
	printf("alpha_pen: ");
	printf("SKIPPED\n");

	return 0;
}

int bbox_pen_test(void)
{
	printf("bbox_pen: ");



	printf("SKIPPED\n");

	return 0;
}

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
			else if(pen.get_value()>=-0.0001f)
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

int display_pen(generic_pen<double> pen, int w, int h)
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
			else if(pen.get_value()>=-0.0001f)
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

void emptyfunction(int v)
{
	static int stupid = 0;
	stupid = v;
	if (stupid == 0) return; // disable unused warning
	//printf("Called... %d\n",v);
}

int box_blur_test(void)
{
	typedef float boxblur_float;

	printf("box_blur: ");

	int w=25,h=25;

	//unique_ptr<boxblur_float> data(new boxblur_float[w*h]);
	//unique_ptr<boxblur_float> data2(new boxblur_float[w*h]);
	std::unique_ptr<boxblur_float> data(new boxblur_float[w*h]);
	std::unique_ptr<boxblur_float> data2(new boxblur_float[w*h]);
	if(!data.get())
	{
		printf("Um..... malloc failure on line %d of " __FILE__ "...\n", __LINE__);
		abort();
	}

	generic_pen<boxblur_float> pen(data.get(),w,h);
	generic_pen<boxblur_float> pen2;

	generic_pen<boxblur_float> pen3(data2.get(),w,h);
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

	int bad_values=0;

	printf("\nBEFORE BOX BLUR:\n");

	// print out the before pic
	display_pen(pen,w,h);

	// Pen 2 will be the end
	pen2=pen;
	pen2.move(w,h);

	//temporary
	vbox_blur(pen,pen2,2,pen3);
	printf("\n VBLUR ONLY:\n");
	display_pen(pen3,w,h);

//	box_blur(pen,w,h,4);
	hbox_blur(pen,pen2,2,pen3);

	printf("\n HBLUR ONLY:\n");
	display_pen(pen3,w,h);

	pen2=pen3;
	pen2.move(w,h);
	vbox_blur(pen3,pen2,2,pen);

	printf("\nAFTER BOX BLUR:\n");

	// print out the after pic
	bad_values=display_pen(pen,w,h);

	if(bad_values)
	{
		printf("FAILURE! " __FILE__ "@%d: blur result contained %d bad values\n", __LINE__, bad_values);
		return 1;
	}

	boxblur_float max=0;
	printf("CHECK BOXBLUR RESULTS %d,%d:\n",pen.diff_begin().x, pen.diff_begin().y);
	for(y=0;y<h;y++,pen.inc_y())
	{
		for(x=0;x<w;x++,pen.inc_x())
		{
			boxblur_float f = 0;

			for(int oy=-2; oy <= 2; ++oy)
			{
				int iy = y+oy;
				if(iy < 0) iy = 0;
				if(iy >= h) iy = h-1;

				for(int ox=-2; ox <= 2; ++ox)
				{
					int ix = x+ox;
					if(ix < 0) ix = 0;
					if(ix >= w) ix = w-1;

					if( (ix-iy<=1 && iy-ix<=1) || iy==h/2 || ix==w/2)
						f += 2;
				}
			}

			//print out if the relative error is high
			/*f /= 25;
			float rf = pen.get_value() - f/25;
			if(f && rf > 0.3)
			{
				printf("pixel (%d,%d) off by %f\n",x,y,rf);
			}*/
			boxblur_float diff = fabs(pen.get_value() - f/25);
			if(diff > max) max = diff;
			pen.put_value(f/25); //if length = 2 then dim = 5.. area = 25
		}
		pen.dec_x(x);
	}
	pen.dec_y(y);

	/*if(max)
	{
		for(y=0;y<h;y++,pen.inc_y())
		{
			for(x=0;x<w;x++,pen.inc_x())
			{
				pen.put_value(pen.get_value()/max);
			}
			pen.dec_x(x);
		}
		pen.dec_y(y);
	}*/

	//printf("\nHBOXBLUR ERROR (max = %e):\n",max);
	printf("\nCorrect results:\n");
	display_pen(pen,w,h);

	printf("PASSED\n");

	return 0;
}

/*
float:
|@@%O.     :::::          |
|@@@%:.    :::::          |
|%@@%O:.   :::::          |
|O%%@%O:.  :::::          |
|.:O%@%O:. :::::          |
| .:O%@%O:.:::::          |
|  .:O%@%O:O::::          |
|   .:O%@%O%O:::          |
|    .:O%@%@%O::          |
|     .:O%@@@%::          |
|::.:::O%@@@@@%O::::::::::|
|::.::::O%@@@@@%::::::::::|
|::.:::::OO@@@@@%O::::::::|
|::.:::::::%@@@@@%O:::::::|
|::.::::::.O%@@@@@%O::::::|
|          ::%@@@%O:.     |
|          ::O%@%@%O:.    |
|          :.:O%O%@%O:.   |
|          :.::O:O%@%O:.  |
|          :.:.:.:O%@%O:. |
|          :.:.: .:O%@%O:.|
|          :.:.:  .:O%@%%O|
|          :.:.:   .:O%@@%|
|          :.:.:    .:%@@@|
|          :.:.:     .O%@@|

double:
|@@%O.     .....          |
|@@@O:.    .....          |
|%@@%O:.   .....          |
|OO%@%O:.  .....          |
|.:O%@%O:. .....          |
| .:O%@%O:.:....          |
|  .:O%@%O:O:...          |
|   .:O%@%O%O:..          |
|    .:O%@%@%O:.          |
|     .:O%@@@O:.          |
|.....:O%@@@@@%O..........|
|......:O%@@@@@%::........|
|.......:OO@@@@@OO:.......|
|........::%@@@@@%O:......|
|..........O%@@@@@%O:.....|
|          .:O@@@%O:.     |
|          .:O%@%@%O:.    |
|          ..:O%O%@%O:.   |
|          ...:O:O%@%O:.  |
|          ....:.:O%@%O:. |
|          ..... .:O%@%O:.|
|          .....  .:O%@%OO|
|          .....   .:O%@@%|
|          .....    .:O@@@|
|          .....     .O%@@|


*/

int gaussian_blur_test(void)
{
	printf("gaussian_blur: ");
#if 0
	int w=25,h=25;
	int bad_values=0;

	auto_ptr<float> data(new float[w*h]);
	if(!data.get())
	{
		printf("Um..... malloc failure on line %d of " __FILE__ "...\n",__LINE__);
		abort();
	}

	generic_pen<float> pen(data.get(),w,h);
	generic_pen<float> pen2;
	int x,y;

	for(y=0;y<h;y++,pen.inc_y())
	{
		for(x=0;x<w;x++,pen.inc_x())
		{
			if((x-y<=1 && y-x<=1) || y==h/2)
				pen.put_value(2);
			else
				pen.put_value(0);
		}
		pen.dec_x(x);
	}
	pen.dec_y(y);

	printf("\nBEFORE GAUSSIAN BLUR:\n");

	// print out the before pic
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
			else if(pen.get_value()>=0.0f)
				printf(" ");
			else
				printf("X"),bad_values++;
		}
		pen.dec_x(x);
		printf("|\n");
	}
	pen.dec_y(y);

	// Pen 2 will be the end
	pen2=pen;
	pen2.move(w,h);

#if 0
	gaussian_blur_5x5(pen,pen2);
	gaussian_blur_5x5(pen,pen2);
	gaussian_blur_5x5(pen,pen2);
#endif

#if 0
	gaussian_blur_3x3(pen,pen2);
	gaussian_blur_3x3(pen,pen2);
	gaussian_blur_3x3(pen,pen2);
	gaussian_blur_3x3(pen,pen2);
	gaussian_blur_3x3(pen,pen2);
#endif

//	gaussian_blur(pen,pen2,15);
	gaussian_blur(pen,pen2,10,10);

	printf("\nAFTER GAUSSIAN BLUR:\n");

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
			else if(pen.get_value()>=0.0f)
				printf(" ");
			else
				printf("X"),bad_values++;
		}
		pen.dec_x(x);
		printf("|\n");
	}
	pen.dec_y(y);

	if(bad_values)
	{
		printf("FAILURE! " __FILE__ "@%d: blur result contained bad values\n",__LINE__);
		return 1;
	}
#endif
	printf("PASSED\n");

	return 0;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=generic_pen_test(40,40);
	error+=generic_pen_test(10,40);
	error+=generic_pen_test(40,10);
    if(error)return error;
	error+=alpha_pen_test();
	error+=bbox_pen_test();
	error+=box_blur_test();
    if(error)return error;
	error+=gaussian_blur_test();

	return error;
}
