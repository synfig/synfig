/* === S Y N F I G ========================================================= */
/*!	\file test_bline.cpp
**	\brief Test BLine class
**
**	\legal
**	Copyright (c) 2020 Synfig contributors
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

#include <synfig/blinepoint.h>
#include <synfig/real.h>
#include <synfig/value.h>
#include <synfig/type.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_const.h>

#include <vector>

#include "test_base.h"

using namespace synfig;

void
fill_list_colinear(std::vector<ValueBase> &list)
{
	const std::vector<Point> points {{0.0, 0.0}, {0.0, 1.0}, {0.0, 2.0}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}
}

void
fill_list_open_rectangle(std::vector<ValueBase> &list)
{
	const std::vector<Point> points {{0.0, 0.0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 0.0}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}
}

void
test_bline_length_single_vertex()
{
	std::vector<ValueBase> list;

	bool loop = false;
	std::vector<Real> lengths;
	Real total_length;

	BLinePoint p1;
	p1.set_tangent1(Point(-1,0));
	p1.set_tangent2(Point(1,0));
	list.push_back(p1);

	loop = false;
	total_length = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(0, lengths.size());
	ASSERT_APPROX_EQUAL(0.0, total_length);

	loop = true;
	total_length = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(1, lengths.size());
	ASSERT_APPROX_EQUAL_MICRO(0.349854, total_length);
}

void
test_bline_length_without_loop()
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const bool loop = false;
	std::vector<Real> lengths;

	Real total_length = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(2, lengths.size());
	ASSERT_APPROX_EQUAL(1.0, lengths[0]);
	ASSERT_APPROX_EQUAL(1.0, lengths[1]);
	ASSERT_APPROX_EQUAL(2.0, total_length);

	list.clear();
	fill_list_open_rectangle(list);

	total_length = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(3, lengths.size());
	ASSERT_APPROX_EQUAL(2.0, lengths[0]);
	ASSERT_APPROX_EQUAL(1.0, lengths[1]);
	ASSERT_APPROX_EQUAL(2.0, lengths[2]);
	ASSERT_APPROX_EQUAL(5.0, total_length);
}

void
test_bline_length_with_loop()
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const bool loop = true;
	std::vector<Real> lengths;

	Real total_length = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(3, lengths.size());
	ASSERT_APPROX_EQUAL(1.0, lengths[0]);
	ASSERT_APPROX_EQUAL(1.0, lengths[1]);
	ASSERT_APPROX_EQUAL(2.0, lengths[2]);
	ASSERT_APPROX_EQUAL(4.0, total_length);

	list.clear();
	fill_list_open_rectangle(list);

	total_length = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(4, lengths.size());
	ASSERT_APPROX_EQUAL(2.0, lengths[0]);
	ASSERT_APPROX_EQUAL(1.0, lengths[1]);
	ASSERT_APPROX_EQUAL(2.0, lengths[2]);
	ASSERT_APPROX_EQUAL(1.0, lengths[3]);
	ASSERT_APPROX_EQUAL(6.0, total_length);
}

void
test_bline_std_to_hom_without_bline_loop_without_index_loop_clamps_to_zero_for_negative_position()
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	ASSERT_APPROX_EQUAL(0.0, synfig::std_to_hom(list,  0.0, false, false));
	ASSERT_APPROX_EQUAL(0.0, synfig::std_to_hom(list, -1.0, false, false));
	ASSERT_APPROX_EQUAL(0.0, synfig::std_to_hom(list, -0.5, false, false));
}

void
test_bline_std_to_hom_without_bline_loop_without_index_loop_clamps_to_one_for_position_greater_than_one()
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	ASSERT_APPROX_EQUAL(1.0, synfig::std_to_hom(list, 1.0, false, false));
	ASSERT_APPROX_EQUAL(1.0, synfig::std_to_hom(list, 2.0, false, false));
	ASSERT_APPROX_EQUAL(1.0, synfig::std_to_hom(list, 2.5, false, false));
}

void
test_bline_std_to_hom_without_bline_loop_with_index_loop_keeps_position_for_both_edges()
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	ASSERT_APPROX_EQUAL( 0.0, synfig::std_to_hom(list,  0.0, true, false));
	ASSERT_APPROX_EQUAL(-1.0, synfig::std_to_hom(list, -1.0, true, false));
	ASSERT_APPROX_EQUAL(1.0, synfig::std_to_hom(list, 1.0, true, false));
	ASSERT_APPROX_EQUAL(2.0, synfig::std_to_hom(list, 2.0, true, false));
}

void
test_bline_std_to_hom_without_loop()
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.176000, 0.370370, 0.5, 0.629630, 0.824000, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i], false, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}
}

void test_bline_std_to_hom_with_loop() {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.162, 0.25, 0.375, 0.5, 0.676, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i], false, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}
}

void test_bline_hom_to_std_without_loop() {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.216466, 0.306518, 0.5, 0.693482, 0.783534, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i], false, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}
}

void test_bline_hom_to_std_with_loop() {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.237620, 0.462321, 0.666667, 0.795654, 0.855690, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i], false, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}
}

void
test_calc_vertex() {
	std::vector<ValueBase> list;
	BLinePoint p;
	p.set_vertex(Point(-2.342526, -1.151789));
	p.set_tangent1(Point(-1.083920, 2.628217));
	p.set_tangent2(Point(-1.083920, 2.628217));
	p.set_origin(0.500000);
	list.push_back(p);
	
	p.set_vertex(Point(0.783103, 0.572000));
	p.set_tangent1(Point(1.503124, 0.000000));
	p.set_tangent2(Point(1.503124, 0.000000));
	p.set_origin(0.500000);
	list.push_back(p);

	p.set_vertex(Point(2.317491, 0.141104));
	p.set_tangent1(Point(0.822620, -0.500000));
	p.set_tangent2(Point(0.822620, -0.500000));
	p.set_origin(0.500000);
	list.push_back(p);

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	
	bool homogeneous = true;

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", ValueNode_Const::create(false));
	bline_calc_vertex->set_link("homogeneous", ValueNode_Const::create(homogeneous));

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.1));
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(-2.219527, -0.653505), vertex)

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.231653));
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(-1.650917, -0.224988), vertex)

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.361874));
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(-1.010658, 0.076790), vertex)

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.507960));
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(-0.260716, 0.341317), vertex)
}

void
test_bline_length_for_bline_with_two_vertices_at_same_spot()
{
	std::vector<ValueBase> list;
	const std::vector<Point> points {{0,0}, {0,1}, {0,1}, {0,2}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}

	bool loop = false;
	std::vector<Real> lengths;

	Real l = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(3, lengths.size());
	ASSERT_APPROX_EQUAL(1.0, lengths[0]);
	ASSERT_APPROX_EQUAL(0.0, lengths[1]);
	ASSERT_APPROX_EQUAL(1.0, lengths[2]);
	ASSERT_APPROX_EQUAL(2.0, l);

	loop = true;
	l = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(4, lengths.size());
	ASSERT_APPROX_EQUAL(1.0, lengths[0]);
	ASSERT_APPROX_EQUAL(0.0, lengths[1]);
	ASSERT_APPROX_EQUAL(1.0, lengths[2]);
	ASSERT_APPROX_EQUAL(2.0, lengths[3]);
	ASSERT_APPROX_EQUAL(4.0, l);
}

void
test_bline_std_to_hom_without_loop_with_two_vertices_at_same_spot() {
	std::vector<ValueBase> list;
	const std::vector<Point> points {{0,0}, {0,1}, {0,1}, {0,2}, {0,3}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}

	const std::vector<Real> positions = {0.0, 0.25, 0.5, 0.75, 1.0,      /* 0.2, */ 0.4, /*0.8*/};
	const std::vector<Real> expected = {0.0, 1/3., 1/3., 2/3., 1.0, /* 0.8*1/3., */1/3., /*2/3.+0.2*1/3.*/};

	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i], false, false);
		ASSERT_APPROX_EQUAL(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL(expected[i] + index_offset, value);
	}
}

void
test_bline_std_to_hom_with_loop_with_two_vertices_at_same_spot() {
	std::vector<ValueBase> list;
	const std::vector<Point> points {{0,0}, {0,1}, {0,1}, {0,2}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}

	const std::vector<Real> positions = {0.0, 0.25, 0.5, 0.75, 1.0,      /* 0.2, */ 0.4, /*0.8*/};
	const std::vector<Real> expected = {0.0, 0.25, 0.25, 0.50, 1.0, /* 0.8*1/3., */0.25, /*2/3.+0.2*1/3.*/};

	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i], false, true);
		ASSERT_APPROX_EQUAL(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL(expected[i] + index_offset, value);
	}
}

void
test_bline_hom_to_std_without_loop_with_two_vertices_at_same_spot() {
	std::vector<ValueBase> list;
	const std::vector<Point> points {{0,0}, {0,1}, {0,1}, {0,2}, {0,3}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}

	const std::vector<Real> positions = {0.0, 1/3., 2/3., 1.0};
	const std::vector<Real> expected = {0.0, 0.25, 0.75, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i], false, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}
}

void
test_bline_hom_to_std_with_loop_with_two_vertices_at_same_spot() {
	std::vector<ValueBase> list;
	const std::vector<Point> points {{0,0}, {0,1}, {0,1}, {0,2}, {0,4}};
	for (const auto& point : points) {
		BLinePoint p;
		p.set_vertex(point);
		list.push_back(p);
	}

	const std::vector<Real> positions = {0.0, 1/8., 2/8., 4/8., 1.0};
	const std::vector<Real> expected = {0.0, 1/5., 3/5., 4/5., 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i], false, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i], value);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		ASSERT_APPROX_EQUAL_MICRO(expected[i] + index_offset, value);
	}
}

void
test_calc_vertex_for_single_vertex_without_loop_returns_itself()
{
	std::vector<ValueBase> list;
	{
		BLinePoint p;
		p.set_vertex({1.0, 2.0});
		list.push_back(p);
	}

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_true);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.0), vertex)

	const_amount->set_value(0.3);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.0), vertex)

	const_amount->set_value(0.8);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.0), vertex)

	const_amount->set_value(1.0);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.0), vertex)
}

void
test_calc_vertex_for_single_vertex_with_loop_returns_itself_on_edges()
{
	std::vector<ValueBase> list;
	{
		BLinePoint p;
		p.set_vertex({1.0, 2.0});
		p.set_tangent({1.0, -2.0});
		list.push_back(p);
	}

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_true);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.0), vertex)

	const_amount->set_value(0.3);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_FALSE(synfig::approximate_not_equal(1.0, vertex[0]));
	ASSERT_FALSE(synfig::approximate_not_equal(2.0, vertex[1]));

	const_amount->set_value(0.8);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_FALSE(synfig::approximate_not_equal(1.0, vertex[0]));
	ASSERT_FALSE(synfig::approximate_not_equal(2.0, vertex[1]));

	const_amount->set_value(1.0);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.0), vertex)
}

void
test_calc_vertex_for_straight_line_without_loop() {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_true);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.0), vertex)

	const_amount->set_value(0.1);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.20), vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.50), vertex)

	const_amount->set_value(0.35);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.70), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 1.00), vertex)

	const_amount->set_value(0.80);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 1.60), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 2.0), vertex)
}

void
test_calc_vertex_for_straight_line_with_loop() {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	bline->set_loop(true);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_true);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.0), vertex)

	const_amount->set_value(0.1);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.40), vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 1.00), vertex)

	const_amount->set_value(0.35);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 1.40), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 2.00), vertex)

	const_amount->set_value(0.75);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 1.00), vertex)

	const_amount->set_value(0.80);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.80), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.0), vertex)
}

void
test_calc_vertex_for_open_rectangle() {
	std::vector<ValueBase> list;
	fill_list_open_rectangle(list);

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_true);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.0), vertex)

	const_amount->set_value(0.10);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.50), vertex)

	const_amount->set_value(0.40);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 2.00), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.5, 2.00), vertex)

	const_amount->set_value(0.60);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.00), vertex)

	const_amount->set_value(0.80);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 1.00), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 0.0), vertex)
}

void
test_calc_vertex_for_closed_rectangle() {
	std::vector<ValueBase> list;
	fill_list_open_rectangle(list);

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	bline->set_loop(true);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_true);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.0), vertex)

	const_amount->set_value(0.1);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.60), vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 1.50), vertex)

	const_amount->set_value(1/3.);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 2.00), vertex)

	const_amount->set_value(0.40);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.4, 2.00), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 2.00), vertex)

	const_amount->set_value(0.75);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 0.50), vertex)

	const_amount->set_value(5/6.);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(1.0, 0.00), vertex)

	const_amount->set_value(0.90);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.6, 0.0), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(Vector(0.0, 0.0), vertex)
}

void
test_calc_vertex_on_vertex_exact_positions_for_looped_curve_with_two_vertices_on_same_coords() {
	std::vector<ValueBase> list;
	struct SimpleBPoint {
		Point vertex;
		Vector tangent1;
		Vector tangent2;
	};

	const std::vector<SimpleBPoint> bpoints {
		{{18.1919, 43.7512}, {0., 0.}, {0., 0.}},
		{{20.1668, 43.8746}, {0., 0.}, {0., 0.}},
		{{18.8027, 46.8822}, {-0.098746, -0.006174}, {0., 0.}},
		{{18.8027, 46.8822}, {0., 0.}, {0., 0.}}
	};
	for (const auto& item : bpoints) {
		BLinePoint p;
		p.set_vertex(item.vertex);
		p.set_tangent1(item.tangent1);
		p.set_tangent2(item.tangent2);
		p.set_origin(0.500000);
		list.push_back(p);
	}

	ValueNode_BLine* bline = ValueNode_BLine::create(list);
	bline->set_loop(true);
	ValueNode_Const* const_false = static_cast<ValueNode_Const*>(ValueNode_Const::create(false));
	// ValueNode_Const* const_true = static_cast<ValueNode_Const*>(ValueNode_Const::create(true));
	ValueNode_Const* const_amount = static_cast<ValueNode_Const*>(ValueNode_Const::create(0.1));

	ValueNode_BLineCalcVertex::Handle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
	bline_calc_vertex->set_link("bline", bline);
	bline_calc_vertex->set_link("loop", const_false);
	bline_calc_vertex->set_link("homogeneous", const_false);
	bline_calc_vertex->set_link("amount", const_amount);

	const_amount->set_value(0.0);
	Vector vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(bpoints[0].vertex, vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(bpoints[1].vertex, vertex)

	const_amount->set_value(0.5);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(bpoints[2].vertex, vertex)

	const_amount->set_value(0.75);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(bpoints[3].vertex, vertex)

	const_amount->set_value(1.0);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	ASSERT_VECTOR_APPROX_EQUAL_MICRO(bpoints[0].vertex, vertex)
}

void test_blinepoint_merged() {
	BLinePoint bp;
	bp.set_merge_tangent_both();

	// default state: merged angle, split radius
	ASSERT_FALSE(bp.get_split_tangent_both())
	ASSERT_FALSE(bp.get_split_tangent_angle())
	ASSERT_FALSE(bp.get_split_tangent_radius());
	ASSERT(bp.get_merge_tangent_both())

	bp.set_tangent(Vector{3, 0});
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent2())

	bp.set_tangent1(Vector{0, 3});
	bp.set_tangent2(Vector{1, 1});
	ASSERT_EQUAL((Vector{0, 3}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{0, 3}), bp.get_tangent2())

	bp.set_tangents(bp.get_tangent2(), bp.get_tangent1());
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, 3}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, 3}), bp.get_tangent2())
}

void test_blinepoint_split_radius() {
	BLinePoint bp;

	// this is the default state, no assignment necessary
	ASSERT_FALSE(bp.get_split_tangent_both())
	ASSERT_FALSE(bp.get_split_tangent_angle())
	ASSERT(bp.get_split_tangent_radius())
	ASSERT_FALSE(bp.get_merge_tangent_both())

	bp.set_tangent(Vector{3, 0});
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent2())

	bp.set_tangent1(Vector{5, 0});
	bp.set_tangent2(Vector{0, 1});
	ASSERT_EQUAL((Vector{5, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{1, 0}), bp.get_tangent2())

	bp.set_tangents(Vector{4, 0}, Vector{0, 2});
	ASSERT_EQUAL((Vector{4, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{2, 0}), bp.get_tangent2())

	bp.set_tangents(bp.get_tangent1(), Vector{-3, 0});
	ASSERT_EQUAL((Vector{4, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{3, 0}), bp.get_tangent2())

	bp.set_tangents(Vector{0, -5}, bp.get_tangent2());
	ASSERT_EQUAL((Vector{0, -5}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, -3}), bp.get_tangent2())

	bp.set_tangents(bp.get_tangent2(), bp.get_tangent1());
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, -3}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, -5}), bp.get_tangent2())
}

void test_blinepoint_split_angle() {
	BLinePoint bp;
	bp.set_split_tangent_angle(true);
	bp.set_split_tangent_radius(false);

	ASSERT_FALSE(bp.get_split_tangent_both())
	ASSERT(bp.get_split_tangent_angle())
	ASSERT_FALSE(bp.get_split_tangent_radius())
	ASSERT_FALSE(bp.get_merge_tangent_both())

	bp.set_tangent(Vector{3, 0});
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent2())

	bp.set_tangent1(Vector{5, 0});
	bp.set_tangent2(Vector{0, 1});
	ASSERT_EQUAL((Vector{5, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, 5}), bp.get_tangent2())

	bp.set_tangents(Vector{4, 0}, Vector{0, 2});
	ASSERT_EQUAL((Vector{4, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, 4}), bp.get_tangent2())

	bp.set_tangents(bp.get_tangent1(), Vector{-3, 0});
	ASSERT_EQUAL((Vector{4, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{-4, 0}), bp.get_tangent2())

	bp.set_tangents(Vector{0, -5}, bp.get_tangent2());
	ASSERT_EQUAL((Vector{0, -5}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{-5, 0}), bp.get_tangent2())

	bp.set_tangents(bp.get_tangent2(), bp.get_tangent1());
	ASSERT_VECTOR_APPROX_EQUAL((Vector{-5, 0}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, -5}), bp.get_tangent2())
}

void test_blinepoint_split_both() {
	BLinePoint bp;
	bp.set_split_tangent_both();

	ASSERT(bp.get_split_tangent_both())
	ASSERT(bp.get_split_tangent_angle())
	ASSERT(bp.get_split_tangent_radius())
	ASSERT_FALSE(bp.get_merge_tangent_both())

	bp.set_tangent(Vector{3, 0});
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{3, 0}), bp.get_tangent2())

	bp.set_tangent1(Vector{5, 0});
	bp.set_tangent2(Vector{0, 1});
	ASSERT_EQUAL((Vector{5, 0}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{0, 1}), bp.get_tangent2())

	bp.set_tangents(Vector{4, 0}, Vector{0, 2});
	ASSERT_EQUAL((Vector{4, 0}), bp.get_tangent1())
	ASSERT_EQUAL((Vector{0, 2}), bp.get_tangent2())

	bp.set_tangents(bp.get_tangent2(), bp.get_tangent1());
	ASSERT_VECTOR_APPROX_EQUAL((Vector{0, 2}), bp.get_tangent1())
	ASSERT_VECTOR_APPROX_EQUAL((Vector{4, 0}), bp.get_tangent2())
}

int main() {
	Type::subsys_init();

	TEST_SUITE_BEGIN();

	TEST_FUNCTION(test_bline_length_single_vertex);
	TEST_FUNCTION(test_bline_length_without_loop);
	TEST_FUNCTION(test_bline_length_with_loop);

	TEST_FUNCTION(test_bline_std_to_hom_without_bline_loop_without_index_loop_clamps_to_zero_for_negative_position);
	TEST_FUNCTION(test_bline_std_to_hom_without_bline_loop_without_index_loop_clamps_to_one_for_position_greater_than_one);
	TEST_FUNCTION(test_bline_std_to_hom_without_bline_loop_with_index_loop_keeps_position_for_both_edges);

	TEST_FUNCTION(test_bline_std_to_hom_without_loop);
	TEST_FUNCTION(test_bline_std_to_hom_with_loop);
	TEST_FUNCTION(test_bline_hom_to_std_without_loop);
	TEST_FUNCTION(test_bline_hom_to_std_with_loop);
	TEST_FUNCTION(test_calc_vertex);

	TEST_FUNCTION(test_bline_length_for_bline_with_two_vertices_at_same_spot);
	TEST_FUNCTION(test_bline_std_to_hom_without_loop_with_two_vertices_at_same_spot);
	TEST_FUNCTION(test_bline_std_to_hom_with_loop_with_two_vertices_at_same_spot);
	TEST_FUNCTION(test_bline_hom_to_std_without_loop_with_two_vertices_at_same_spot);
	TEST_FUNCTION(test_bline_hom_to_std_with_loop_with_two_vertices_at_same_spot);

	TEST_FUNCTION(test_calc_vertex_for_single_vertex_without_loop_returns_itself);
	TEST_FUNCTION(test_calc_vertex_for_single_vertex_with_loop_returns_itself_on_edges);
	TEST_FUNCTION(test_calc_vertex_for_straight_line_without_loop);
	TEST_FUNCTION(test_calc_vertex_for_straight_line_with_loop);
	TEST_FUNCTION(test_calc_vertex_for_open_rectangle);
	TEST_FUNCTION(test_calc_vertex_for_closed_rectangle);
	TEST_FUNCTION(test_calc_vertex_on_vertex_exact_positions_for_looped_curve_with_two_vertices_on_same_coords);

	TEST_FUNCTION(test_blinepoint_merged)
	TEST_FUNCTION(test_blinepoint_split_radius)
	TEST_FUNCTION(test_blinepoint_split_angle)
	TEST_FUNCTION(test_blinepoint_split_both)

	TEST_SUITE_END();

	Type::subsys_stop();

	return tst_exit_status;
}
