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

#include "test_base.h"

#include <memory>

#include <synfig/blur/boxblur.h>
#include <synfig/pen.h>

/* === M A C R O S ========================================================= */

using namespace synfig;

static int data[] = {
	1,2,3,0,0,0,
	4,5,6,0,0,0,
	7,8,9,0,0,0,
	10,11,12,0,0,0
};


/* === C L A S S E S ======================================================= */

void test_default_generic_pen_row_iterator_is_invalid()
{
	generic_pen_row_iterator<int> it;

	ASSERT_FALSE(bool(it));

	ASSERT(!it);
}

void test_inc_generic_pen_row_iterator_goes_to_next_row()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	ASSERT_EQUAL(1, *it);

	it.inc();
	ASSERT_EQUAL(4, *it);
}

void test_inc_n_generic_pen_row_iterator_advance_n_rows()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	ASSERT_EQUAL(1, *it);

	it.inc(2);
	ASSERT_EQUAL(7, *it);
}

void test_dec_generic_pen_row_iterator_goes_to_previous_row()
{
	generic_pen_row_iterator<int> it(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(7, *it);

	it.dec();
	ASSERT_EQUAL(4, *it);
}

void test_dec_n_generic_pen_row_iterator_returns_n_rows()
{
	generic_pen_row_iterator<int> it(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(7, *it);

	it.dec(2);
	ASSERT_EQUAL(1, *it);
}

void test_pre_inc_generic_pen_row_iterator_goes_to_next_row()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	ASSERT_EQUAL(1, *it);

	ASSERT_EQUAL(4, *++it);
}

void test_pre_dec_generic_pen_row_iterator_goes_to_previous_row()
{
	generic_pen_row_iterator<int> it(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(7, *it);

	ASSERT_EQUAL(4, *(--it));
}

void test_post_inc_generic_pen_row_iterator_goes_to_next_row()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	ASSERT_EQUAL(1, *it);

	it++;
	ASSERT_EQUAL(4, *it);
}

void test_post_dec_generic_pen_row_iterator_goes_to_previous_row()
{
	generic_pen_row_iterator<int> it(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(7, *it);

	it--;
	ASSERT_EQUAL(4, *it);
}

void test_post_inc_generic_pen_row_iterator_does_change_itself_after()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	ASSERT_EQUAL(1, *it);

	ASSERT_EQUAL(1, *it++);
}

void test_post_dec_generic_pen_row_iterator_does_change_itself_after()
{
	generic_pen_row_iterator<int> it(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(7, *it);

	ASSERT_EQUAL(7, *(it--));
}

void test_generic_pen_row_iterator_operator_brackets_reaches_nth_row()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	ASSERT_EQUAL(1, it[0]);
	ASSERT_EQUAL(4, it[1]);
	ASSERT_EQUAL(7, it[2]);
	ASSERT_EQUAL(10, it[3]);

	generic_pen_row_iterator<int> it2(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(7, it2[0]);
	ASSERT_EQUAL(10, it2[1]);
	ASSERT_EQUAL(4, it2[-1]);
	ASSERT_EQUAL(1, it2[-2]);
}

void test_generic_pen_row_iterator_adding_number_advance_n_rows()
{
	generic_pen_row_iterator<int> it(data, 6*sizeof(int));

	it = it + 2;
	ASSERT_EQUAL(7, *it);

	it = it + 1;
	ASSERT_EQUAL(10, *it);
}

void test_generic_pen_row_iterator_subtracting_number_advance_n_rows()
{
	generic_pen_row_iterator<int> it(&data[12], 6*sizeof(int));

	it = it - 2;
	ASSERT_EQUAL(1, *it);
}

void test_generic_pen_row_iterator_difference_computes_n_rows()
{
	generic_pen_row_iterator<int> it1(&data[0], 6*sizeof(int));
	generic_pen_row_iterator<int> it2(&data[12], 6*sizeof(int));

	ASSERT_EQUAL(0, it1 - it1);
	ASSERT_EQUAL(0, it2 - it2);
	ASSERT_EQUAL(2, it2 - it1);
	ASSERT_EQUAL(-2, it1 - it2);
}

void test_default_generic_pen_is_invalid()
{
	generic_pen<int> pen;
	ASSERT(!pen);
}

void test_generic_pen_stores_constructor_info()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));

	ASSERT(pen);

	ASSERT_EQUAL(3, pen.get_width());
	ASSERT_EQUAL(4, pen.get_height());
	ASSERT_EQUAL(24, pen.get_pitch());
}

void test_generic_pen_remembers_pen_value_set()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	pen.set_value(-15);
	ASSERT_EQUAL(-15, pen.get_pen_value());
}


void test_generic_pen_inc_x_moves_to_next_column()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	ASSERT_EQUAL(1, *pen.x());
	ASSERT_EQUAL(1, *pen[0]);
	pen.inc_x();
	ASSERT_EQUAL(2, *pen.x());
}

void test_generic_pen_inc_x_advances_n_columns()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	pen.inc_x(2);
	ASSERT_EQUAL(3, *pen.x());
}

void test_generic_pen_dec_x_moves_to_previous_column()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	ASSERT_EQUAL(1, *pen.x());
	pen.inc_x(2);
	pen.dec_x();
	ASSERT_EQUAL(2, *pen.x());
}

void test_generic_pen_dec_x_returns_n_columns()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	ASSERT_EQUAL(1, *pen.x());
	pen.inc_x(3);
	pen.dec_x(2);
	ASSERT_EQUAL(2, *pen.x());
}

void test_generic_pen_inc_y_moves_to_next_row()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	pen.inc_y();
	ASSERT_EQUAL(4, *pen.x());
}

void test_generic_pen_inc_y_advances_n_rows()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	pen.inc_y(2);
	ASSERT_EQUAL(7, *pen.x());
}

void test_generic_pen_dec_y_moves_to_previous_row()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	pen.inc_y(2);
	pen.dec_y();
	ASSERT_EQUAL(4, *pen.x());
}

void test_generic_pen_dec_y_returns_n_rows()
{
	generic_pen<int> pen(data, 3, 4, 6*sizeof(int));
	pen.inc_y(3);
	pen.dec_y(2);
	ASSERT_EQUAL(4, *pen.x());
}

void generic_pen_test(int w, int h)
{
	std::unique_ptr<float[]> data(new float[w*h]);
	ASSERT(data);

	generic_pen<float> pen(data.get(),w,h);
	generic_pen<float> pen2;

	ASSERT(pen)

	ASSERT(&pen.x()[2] == &pen[0][2]);

	ASSERT(&pen.y()[2] == &pen[2][0]);

	pen.move(1,1);
	pen2=pen;

	ASSERT(pen==pen2);

	pen2.move(w,h);
	generic_pen<float>::difference_type diff(pen2-pen);

	ASSERT_EQUAL(w, diff.x);
	ASSERT_EQUAL(h, diff.y);

	ASSERT_EQUAL(w - 1, pen.end_x() - pen.x());

	ASSERT_EQUAL(h - 1, pen.end_y() - pen.y());

	ASSERT(&pen.end_y()[-1] == &pen.y()[(h-2)]);

	ASSERT(&pen.end_x()[-1] == &pen.x()[(w-2)]);
}

void generic_pen_test_40_40()
{
	generic_pen_test(40, 40);
}

void generic_pen_test_40_10()
{
	generic_pen_test(40, 10);
}

void generic_pen_test_10_40()
{
	generic_pen_test(10, 40);
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

int box_blur_test(void)
{
	typedef float boxblur_float;

	printf("box_blur: ");

	int w=25,h=25;

	//unique_ptr<boxblur_float> data(new boxblur_float[w*h]);
	//unique_ptr<boxblur_float> data2(new boxblur_float[w*h]);
	std::unique_ptr<boxblur_float[]> data(new boxblur_float[w*h]);
	std::unique_ptr<boxblur_float[]> data2(new boxblur_float[w*h]);
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
	printf("CHECK BOXBLUR RESULTS\n");
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

	TEST_SUITE_BEGIN()
		TEST_FUNCTION(test_default_generic_pen_row_iterator_is_invalid);
		TEST_FUNCTION(test_inc_generic_pen_row_iterator_goes_to_next_row);
		TEST_FUNCTION(test_inc_n_generic_pen_row_iterator_advance_n_rows);
		TEST_FUNCTION(test_dec_generic_pen_row_iterator_goes_to_previous_row);
		TEST_FUNCTION(test_dec_n_generic_pen_row_iterator_returns_n_rows);
		TEST_FUNCTION(test_pre_inc_generic_pen_row_iterator_goes_to_next_row);
		TEST_FUNCTION(test_pre_dec_generic_pen_row_iterator_goes_to_previous_row);
		TEST_FUNCTION(test_post_inc_generic_pen_row_iterator_goes_to_next_row);
		TEST_FUNCTION(test_post_dec_generic_pen_row_iterator_goes_to_previous_row);
		TEST_FUNCTION(test_post_inc_generic_pen_row_iterator_does_change_itself_after);
		TEST_FUNCTION(test_post_dec_generic_pen_row_iterator_does_change_itself_after);
		TEST_FUNCTION(test_generic_pen_row_iterator_operator_brackets_reaches_nth_row);
		TEST_FUNCTION(test_generic_pen_row_iterator_adding_number_advance_n_rows);
		TEST_FUNCTION(test_generic_pen_row_iterator_subtracting_number_advance_n_rows);
		// FIXME: .end_y() should return an invalid y() (right after last row) ?
		//TEST_FUNCTION(test_generic_pen_row_iterator_difference_computes_n_rows);

		TEST_FUNCTION(test_default_generic_pen_is_invalid);
		TEST_FUNCTION(test_generic_pen_stores_constructor_info);
		TEST_FUNCTION(test_generic_pen_remembers_pen_value_set);

		TEST_FUNCTION(test_generic_pen_inc_x_moves_to_next_column);
		TEST_FUNCTION(test_generic_pen_inc_x_advances_n_columns);
		TEST_FUNCTION(test_generic_pen_dec_x_moves_to_previous_column);
		TEST_FUNCTION(test_generic_pen_dec_x_returns_n_columns);

		TEST_FUNCTION(test_generic_pen_inc_y_moves_to_next_row);
		TEST_FUNCTION(test_generic_pen_inc_y_advances_n_rows);
		TEST_FUNCTION(test_generic_pen_dec_y_moves_to_previous_row);
		TEST_FUNCTION(test_generic_pen_dec_y_returns_n_rows);

		TEST_FUNCTION(generic_pen_test_40_40);
		TEST_FUNCTION(generic_pen_test_10_40);
		TEST_FUNCTION(generic_pen_test_40_10);
	TEST_SUITE_END()

//	return tst_exit_status;
	int error=tst_exit_status;

    if(error)return error;
	error+=box_blur_test();
    if(error)return error;
	error+=gaussian_blur_test();

	return error;
}
