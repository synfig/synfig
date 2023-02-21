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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <synfig/surface_etl.h>

#include "test_base.h"

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

void test_empty_surface_is_invalid()
{
	surface<float> my_void_surface;
	ASSERT_FALSE(my_void_surface.is_valid());
	ASSERT(!my_void_surface);

	ASSERT_EQUAL(0, my_void_surface.get_w());
	ASSERT_EQUAL(0, my_void_surface.get_h());
}

void test_new_surface_has_correct_dimensions()
{
	surface<float> my_surface(200, 100);
	ASSERT(my_surface.is_valid());
	ASSERT_FALSE(!my_surface);

	ASSERT_EQUAL(200, my_surface.get_w());
	ASSERT_EQUAL(100, my_surface.get_h());
	ASSERT(my_surface.get_pitch() >= int(200 * sizeof(float)));
}

void test_new_surface_with_negative_width_is_invalid()
{
	surface<float> my_surface(-5, 2);
	ASSERT_FALSE(my_surface.is_valid());
	ASSERT(!my_surface);
}

void test_new_surface_with_negative_height_is_invalid()
{
	surface<float> my_surface(5, -2);
	ASSERT_FALSE(my_surface.is_valid());
	ASSERT(!my_surface);
}

void test_new_surface_with_negative_dimensions_is_invalid()
{
	surface<float> my_surface(-5, -2);
	ASSERT_FALSE(my_surface.is_valid());
	ASSERT(!my_surface);
}

void test_resize_surface_has_new_dimensions()
{
	surface<char> my_surface(4, 4);
	my_surface.set_wh(5, 8);
	ASSERT_EQUAL(5, my_surface.get_w());
	ASSERT_EQUAL(8, my_surface.get_h());

	my_surface.set_wh(9, 3, 10 * sizeof(char));
	ASSERT_EQUAL(9, my_surface.get_w());
	ASSERT_EQUAL(3, my_surface.get_h());
	ASSERT_EQUAL(10, my_surface.get_pitch());
}

void test_resize_surface_does_not_delete_non_deletable_data()
{
	int* data = static_cast<int*>(malloc(sizeof(int) * 20));
	surface<int> my_surface(data, 4, 4, false);
	my_surface.set_wh(5, 8);
	ASSERT_EQUAL(5, my_surface.get_w());
	ASSERT_EQUAL(8, my_surface.get_h());
	free(data); // It must not cause double free
}

void test_resize_surface_delete_deletable_data()
{
	int* data = new int[20];
	surface<int> my_surface(data, 4, 4, true);
	my_surface.set_wh(5, 8);
	ASSERT_EQUAL(5, my_surface.get_w());
	ASSERT_EQUAL(8, my_surface.get_h());
	// Use valgrind here to check memory leak of data
}

void test_set_sample_at_start_of_the_first_row()
{
	surface<float> my_surface(3, 4);
	my_surface.clear();
	my_surface[0][0] = 32;
	ASSERT_EQUAL(32, my_surface[0][0]);
}

void test_set_sample_at_start_of_the_second_row()
{
	surface<float> my_surface(3, 4);
	my_surface.clear();
	my_surface[1][0] = 32;
	ASSERT_EQUAL(32, my_surface[1][0]);
}

void test_set_sample_at_end_of_the_last_row()
{
	surface<float> my_surface(3, 4);
	my_surface.clear();
	my_surface[2][3] = 45;
	ASSERT_EQUAL(45, my_surface[2][3]);
}

void test_set_sample_at_end_of_the_first_row()
{
	surface<float> my_surface(3, 4);
	my_surface.clear();
	my_surface[0][3] = 32;
	ASSERT_EQUAL(32, my_surface[0][3]);
}

void test_set_sample_at_start_of_the_second_row_with_non_compact_pitch()
{
	std::vector<float> data(40);
	surface<float> my_surface(data.data(), 3, 4, 10*sizeof(float), false);
	my_surface.clear();
	my_surface[1][0] = 32;
	ASSERT_EQUAL(32, my_surface[1][0]);
	ASSERT_EQUAL(32, data[10]);
}

void test_clear_surface()
{
	surface<float> my_surface(30, 40);
	my_surface.clear();
	for (int y = 0; y < 40; ++y)
		for (int x = 0; x < 30; ++x)
			ASSERT_EQUAL(0, my_surface[y][x]);
}

void test_surface_copy_constructor_from_deletable_surface()
{
	int* data = new int[12];
	surface<int> my_surface(data, 3, 4, true);
	my_surface.fill(5);
	my_surface[2][1] = 8;
	surface<int> my_surface2(my_surface);
	ASSERT_EQUAL(5, my_surface2[0][0]);
	ASSERT_EQUAL(5, my_surface2[0][1]);
	ASSERT_EQUAL(5, my_surface2[0][2]);

	ASSERT_EQUAL(5, my_surface2[1][0]);
	ASSERT_EQUAL(5, my_surface2[1][1]);
	ASSERT_EQUAL(5, my_surface2[1][2]);

	ASSERT_EQUAL(5, my_surface2[2][0]);
	ASSERT_EQUAL(8, my_surface2[2][1]);
	ASSERT_EQUAL(5, my_surface2[2][2]);

	ASSERT_EQUAL(5, my_surface2[3][0]);
	ASSERT_EQUAL(5, my_surface2[3][1]);
	ASSERT_EQUAL(5, my_surface2[3][2]);
}

void test_surface_copy_constructor_from_non_deletable_surface()
{
	surface<int> my_surface(3, 4);
	my_surface.fill(5);
	my_surface[2][1] = 8;
	surface<int> my_surface2(my_surface);
	ASSERT_EQUAL(5, my_surface2[0][0]);
	ASSERT_EQUAL(5, my_surface2[0][1]);
	ASSERT_EQUAL(5, my_surface2[0][2]);

	ASSERT_EQUAL(5, my_surface2[1][0]);
	ASSERT_EQUAL(5, my_surface2[1][1]);
	ASSERT_EQUAL(5, my_surface2[1][2]);

	ASSERT_EQUAL(5, my_surface2[2][0]);
	ASSERT_EQUAL(8, my_surface2[2][1]);
	ASSERT_EQUAL(5, my_surface2[2][2]);

	ASSERT_EQUAL(5, my_surface2[3][0]);
	ASSERT_EQUAL(5, my_surface2[3][1]);
	ASSERT_EQUAL(5, my_surface2[3][2]);
}

void test_surface_copy_constructor_does_not_share_data_with_deletable_data()
{
	int* data = new int[12];
	surface<int> my_surface(data, 3, 4, true);
	my_surface.fill(5);
	surface<int> my_surface2(my_surface);
	my_surface2[1][1] = 10;
	my_surface[2][2] = -10;

	ASSERT_EQUAL( 10, my_surface2[1][1]);
	ASSERT_EQUAL(  5, my_surface[1][1]);
	ASSERT_EQUAL(-10, my_surface[2][2]);
	ASSERT_EQUAL(  5, my_surface2[2][2]);
}

void test_surface_copy_constructor_does_not_share_data_with_non_deletable_data()
{
	surface<int> my_surface(3, 4);
	my_surface.fill(5);
	surface<int> my_surface2(my_surface);
	my_surface2[1][1] = 10;
	my_surface[2][2] = -10;

	ASSERT_EQUAL( 10, my_surface2[1][1]);
	ASSERT_EQUAL(  5, my_surface[1][1]);
	ASSERT_EQUAL(-10, my_surface[2][2]);
	ASSERT_EQUAL(  5, my_surface2[2][2]);
}

void test_surface_copy_assignment_operator_from_deletable_surface()
{
	int* data = new int[12];
	surface<int> my_surface(data, 3, 4, true);
	my_surface.fill(5);
	my_surface[2][1] = 8;
	surface<int> my_surface2 = my_surface;
	ASSERT_EQUAL(5, my_surface2[0][0]);
	ASSERT_EQUAL(5, my_surface2[0][1]);
	ASSERT_EQUAL(5, my_surface2[0][2]);

	ASSERT_EQUAL(5, my_surface2[1][0]);
	ASSERT_EQUAL(5, my_surface2[1][1]);
	ASSERT_EQUAL(5, my_surface2[1][2]);

	ASSERT_EQUAL(5, my_surface2[2][0]);
	ASSERT_EQUAL(8, my_surface2[2][1]);
	ASSERT_EQUAL(5, my_surface2[2][2]);

	ASSERT_EQUAL(5, my_surface2[3][0]);
	ASSERT_EQUAL(5, my_surface2[3][1]);
	ASSERT_EQUAL(5, my_surface2[3][2]);
}

void test_surface_copy_assignment_operator_from_non_deletable_surface()
{
	surface<int> my_surface(3, 4);
	my_surface.fill(5);
	my_surface[2][1] = 8;
	surface<int> my_surface2 = my_surface;
	ASSERT_EQUAL(5, my_surface2[0][0]);
	ASSERT_EQUAL(5, my_surface2[0][1]);
	ASSERT_EQUAL(5, my_surface2[0][2]);

	ASSERT_EQUAL(5, my_surface2[1][0]);
	ASSERT_EQUAL(5, my_surface2[1][1]);
	ASSERT_EQUAL(5, my_surface2[1][2]);

	ASSERT_EQUAL(5, my_surface2[2][0]);
	ASSERT_EQUAL(8, my_surface2[2][1]);
	ASSERT_EQUAL(5, my_surface2[2][2]);

	ASSERT_EQUAL(5, my_surface2[3][0]);
	ASSERT_EQUAL(5, my_surface2[3][1]);
	ASSERT_EQUAL(5, my_surface2[3][2]);
}

void test_surface_copy_assignment_operator_does_not_share_data_with_deletable_data()
{
	int* data = new int[12];
	surface<int> my_surface(data, 3, 4, true);
	my_surface.fill(5);
	surface<int> my_surface2 = my_surface;
	my_surface2[1][1] = 10;
	my_surface[2][2] = -10;

	ASSERT_EQUAL( 10, my_surface2[1][1]);
	ASSERT_EQUAL(  5, my_surface[1][1]);
	ASSERT_EQUAL(-10, my_surface[2][2]);
	ASSERT_EQUAL(  5, my_surface2[2][2]);
}

void test_surface_copy_assignment_operator_does_not_share_data_with_non_deletable_data()
{
	surface<int> my_surface(3, 4);
	my_surface.fill(5);
	surface<int> my_surface2 = my_surface;
	my_surface2[1][1] = 10;
	my_surface[2][2] = -10;

	ASSERT_EQUAL( 10, my_surface2[1][1]);
	ASSERT_EQUAL(  5, my_surface[1][1]);
	ASSERT_EQUAL(-10, my_surface[2][2]);
	ASSERT_EQUAL(  5, my_surface2[2][2]);
}

void test_fill_all_surface_with_same_data()
{
	surface<int> my_surface(30, 3);
	my_surface.fill(98);
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 30; ++x)
			ASSERT_EQUAL(98, my_surface[y][x]);
}

void test_fill_surface_rectangle_with_same_data()
{
	surface<int> my_surface(30, 30);
	my_surface.clear();
	my_surface.fill(42, 10, 10, 5, 8);
	for (int y = 10; y < 18; ++y)
		for (int x = 10; x < 15; ++x)
			ASSERT_EQUAL(42, my_surface[y][x]);
	for (int x = 0; x < 30; ++x) {
		ASSERT_EQUAL(0, my_surface[9][x]);
		ASSERT_EQUAL(0, my_surface[18][x]);
	}
	for (int y = 0; y < 30; ++y) {
		ASSERT_EQUAL(0, my_surface[y][9]);
		ASSERT_EQUAL(0, my_surface[y][15]);
	}
}

void test_blit_to_equal_size_surface()
{
	surface<int> src_surface(4, 4);
	surface<int> dst_surface(4, 4);

	src_surface.fill(80);
	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen);

	ASSERT_EQUAL(80, dst_surface[0][0]);
	ASSERT_EQUAL(80, dst_surface[0][1]);
	ASSERT_EQUAL(80, dst_surface[0][2]);
	ASSERT_EQUAL(80, dst_surface[0][3]);

	ASSERT_EQUAL(80, dst_surface[1][0]);
	ASSERT_EQUAL(80, dst_surface[1][1]);
	ASSERT_EQUAL(80, dst_surface[1][2]);
	ASSERT_EQUAL(80, dst_surface[1][3]);

	ASSERT_EQUAL(80, dst_surface[2][0]);
	ASSERT_EQUAL(80, dst_surface[2][1]);
	ASSERT_EQUAL(80, dst_surface[2][2]);
	ASSERT_EQUAL(80, dst_surface[2][3]);

	ASSERT_EQUAL(80, dst_surface[3][0]);
	ASSERT_EQUAL(80, dst_surface[3][1]);
	ASSERT_EQUAL(80, dst_surface[3][2]);
	ASSERT_EQUAL(80, dst_surface[3][3]);
}

void test_blit_to_larger_surface()
{
	surface<int> src_surface(2, 2);
	surface<int> dst_surface(4, 4);

	src_surface.fill(-5);
	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen);

	ASSERT_EQUAL(-5, dst_surface[0][0]);
	ASSERT_EQUAL(-5, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL(-5, dst_surface[1][0]);
	ASSERT_EQUAL(-5, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

void test_blit_to_smaller_surface()
{
	surface<int> src_surface(4, 4);
	surface<int> dst_surface(2, 2);

	src_surface.fill(-8);
	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen);

	ASSERT_EQUAL(-8, dst_surface[0][0]);
	ASSERT_EQUAL(-8, dst_surface[0][1]);

	ASSERT_EQUAL(-8, dst_surface[1][0]);
	ASSERT_EQUAL(-8, dst_surface[1][1]);
}

void test_blit_region_to_equal_size_surface()
{
	int src_data[16] = {
		 1, 2, 3, 4,
		 5, 6, 7, 8,
		 9,10,11,12,
		13,14,15,16
	};
	surface<int> src_surface(src_data, 4, 4);
	surface<int> dst_surface(4, 4);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, 2, 1, 2, 2);

	ASSERT_EQUAL( 7, dst_surface[0][0]);
	ASSERT_EQUAL( 8, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL(11, dst_surface[1][0]);
	ASSERT_EQUAL(12, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

void test_blit_region_to_larger_surface()
{
	int src_data[4] = {
		1, 2,
		3, 4
	};
	surface<int> src_surface(src_data, 2, 2, false);
	surface<int> dst_surface(4, 4);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, 1, 0, 1, 1);

	ASSERT_EQUAL( 2, dst_surface[0][0]);
	ASSERT_EQUAL( 0, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL( 0, dst_surface[1][0]);
	ASSERT_EQUAL( 0, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

void test_blit_region_to_smaller_surface()
{
	int src_data[16] = {
		 1, 2, 3, 4,
		 5, 6, 7, 8,
		 9,10,11,12,
		13,14,15,16
	};
	surface<int> src_surface(src_data, 4, 4, false);
	surface<int> dst_surface(2, 2);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, 1, 2, 1, 2);

	ASSERT_EQUAL(10, dst_surface[0][0]);
	ASSERT_EQUAL( 0, dst_surface[0][1]);

	ASSERT_EQUAL(14, dst_surface[1][0]);
	ASSERT_EQUAL( 0, dst_surface[1][1]);
}

void test_surface_blitting_clamp_negative_x()
{
	int src_data[16] = {
		 1, 2, 3, 4,
		 5, 6, 7, 8,
		 9,10,11,12,
		13,14,15,16
	};
	surface<int> src_surface(src_data, 4, 4, false);
	surface<int> dst_surface(4, 4);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, -1, 2, 2, 2);

	ASSERT_EQUAL( 9, dst_surface[0][0]);
	ASSERT_EQUAL( 0, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL(13, dst_surface[1][0]);
	ASSERT_EQUAL( 0, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

void test_surface_blitting_clamp_negative_y()
{
	int src_data[16] = {
		 1, 2, 3, 4,
		 5, 6, 7, 8,
		 9,10,11,12,
		13,14,15,16
	};
	surface<int> src_surface(src_data, 4, 4, false);
	surface<int> dst_surface(4, 4);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, 1, -2, 2, 3);

	ASSERT_EQUAL( 2, dst_surface[0][0]);
	ASSERT_EQUAL( 3, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL( 0, dst_surface[1][0]);
	ASSERT_EQUAL( 0, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

void test_surface_blitting_clamp_larger_width()
{
	int src_data[16] = {
		 1, 2, 3, 4,
		 5, 6, 7, 8,
		 9,10,11,12,
		13,14,15,16
	};
	surface<int> src_surface(src_data, 4, 4, false);
	surface<int> dst_surface(4, 4);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, 2, 2, 5, 2);

	ASSERT_EQUAL(11, dst_surface[0][0]);
	ASSERT_EQUAL(12, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL(15, dst_surface[1][0]);
	ASSERT_EQUAL(16, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

void test_surface_blitting_clamp_larger_height()
{
	int src_data[16] = {
		 1, 2, 3, 4,
		 5, 6, 7, 8,
		 9,10,11,12,
		13,14,15,16
	};
	surface<int> src_surface(src_data, 4, 4, false);
	surface<int> dst_surface(4, 4);

	dst_surface.clear();
	auto dst_pen = dst_surface.begin();
	src_surface.blit_to(dst_pen, 1, 2, 2, 5);

	ASSERT_EQUAL(10, dst_surface[0][0]);
	ASSERT_EQUAL(11, dst_surface[0][1]);
	ASSERT_EQUAL( 0, dst_surface[0][2]);
	ASSERT_EQUAL( 0, dst_surface[0][3]);

	ASSERT_EQUAL(14, dst_surface[1][0]);
	ASSERT_EQUAL(15, dst_surface[1][1]);
	ASSERT_EQUAL( 0, dst_surface[1][2]);
	ASSERT_EQUAL( 0, dst_surface[1][3]);

	ASSERT_EQUAL( 0, dst_surface[2][0]);
	ASSERT_EQUAL( 0, dst_surface[2][1]);
	ASSERT_EQUAL( 0, dst_surface[2][2]);
	ASSERT_EQUAL( 0, dst_surface[2][3]);

	ASSERT_EQUAL( 0, dst_surface[3][0]);
	ASSERT_EQUAL( 0, dst_surface[3][1]);
	ASSERT_EQUAL( 0, dst_surface[3][2]);
	ASSERT_EQUAL( 0, dst_surface[3][3]);
}

int display_pen(synfig::generic_pen<float> pen, int w, int h)
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

void make_pattern(synfig::generic_pen<float> pen, int w, int h)
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
	TEST_SUITE_BEGIN()
		TEST_FUNCTION(test_empty_surface_is_invalid);
		TEST_FUNCTION(test_new_surface_has_correct_dimensions);
		// FIXME test failure
		//TEST_FUNCTION(test_new_surface_with_negative_width_is_invalid);
		// FIXME test failure
		//TEST_FUNCTION(test_new_surface_with_negative_height_is_invalid);
		TEST_FUNCTION(test_new_surface_with_negative_dimensions_is_invalid);

		TEST_FUNCTION(test_resize_surface_has_new_dimensions);
		TEST_FUNCTION(test_resize_surface_does_not_delete_non_deletable_data);
		// Use valgrind here to check memory leak
		TEST_FUNCTION(test_resize_surface_delete_deletable_data);

		TEST_FUNCTION(test_set_sample_at_start_of_the_first_row);
		TEST_FUNCTION(test_set_sample_at_start_of_the_second_row);
		TEST_FUNCTION(test_set_sample_at_end_of_the_last_row);
		TEST_FUNCTION(test_set_sample_at_end_of_the_first_row);
		TEST_FUNCTION(test_set_sample_at_start_of_the_second_row_with_non_compact_pitch);

		TEST_FUNCTION(test_clear_surface);

		TEST_FUNCTION(test_surface_copy_constructor_from_deletable_surface)
		TEST_FUNCTION(test_surface_copy_constructor_from_non_deletable_surface)
		TEST_FUNCTION(test_surface_copy_constructor_does_not_share_data_with_deletable_data)
		TEST_FUNCTION(test_surface_copy_constructor_does_not_share_data_with_non_deletable_data)

		TEST_FUNCTION(test_surface_copy_assignment_operator_from_deletable_surface)
		TEST_FUNCTION(test_surface_copy_assignment_operator_from_non_deletable_surface)
		TEST_FUNCTION(test_surface_copy_assignment_operator_does_not_share_data_with_deletable_data)
		TEST_FUNCTION(test_surface_copy_assignment_operator_does_not_share_data_with_non_deletable_data)

		TEST_FUNCTION(test_fill_all_surface_with_same_data);
		TEST_FUNCTION(test_fill_surface_rectangle_with_same_data);

		TEST_FUNCTION(test_blit_to_equal_size_surface);
		TEST_FUNCTION(test_blit_to_larger_surface);
		TEST_FUNCTION(test_blit_to_smaller_surface);

		TEST_FUNCTION(test_blit_region_to_equal_size_surface);
		TEST_FUNCTION(test_blit_region_to_larger_surface);
		TEST_FUNCTION(test_blit_region_to_smaller_surface);

		TEST_FUNCTION(test_surface_blitting_clamp_negative_x);
		TEST_FUNCTION(test_surface_blitting_clamp_negative_y);
		TEST_FUNCTION(test_surface_blitting_clamp_larger_width);
		TEST_FUNCTION(test_surface_blitting_clamp_larger_height);
	TEST_SUITE_END()

	linear_sample_test();
	cubic_sample_test();

	return tst_exit_status;
}
