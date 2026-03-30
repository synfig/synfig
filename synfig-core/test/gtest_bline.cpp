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

#include <gtest/gtest.h>

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

class BlineTest : public ::testing::Test {
protected:
	static void SetUpTestSuite() {
		synfig::Type::subsys_init();
	}
	static void TearDownTestSuite() {
		synfig::Type::subsys_stop();
	}
};

TEST_F(BlineTest, test_bline_length_single_vertex)
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
	EXPECT_EQ(0, lengths.size());
	EXPECT_NEAR(0.0, total_length, 1e-4);

	loop = true;
	total_length = bline_length(list, loop, &lengths);
	EXPECT_EQ(1, lengths.size());
	EXPECT_NEAR(0.349854, total_length, 1e-6);
}

TEST_F(BlineTest, test_bline_length_without_loop)
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const bool loop = false;
	std::vector<Real> lengths;

	Real total_length = bline_length(list, loop, &lengths);
	EXPECT_EQ(2, lengths.size());
	EXPECT_NEAR(1.0, lengths[0], 1e-4);
	EXPECT_NEAR(1.0, lengths[1], 1e-4);
	EXPECT_NEAR(2.0, total_length, 1e-4);

	list.clear();
	fill_list_open_rectangle(list);

	total_length = bline_length(list, loop, &lengths);
	EXPECT_EQ(3, lengths.size());
	EXPECT_NEAR(2.0, lengths[0], 1e-4);
	EXPECT_NEAR(1.0, lengths[1], 1e-4);
	EXPECT_NEAR(2.0, lengths[2], 1e-4);
	EXPECT_NEAR(5.0, total_length, 1e-4);
}

TEST_F(BlineTest, test_bline_length_with_loop)
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const bool loop = true;
	std::vector<Real> lengths;

	Real total_length = bline_length(list, loop, &lengths);
	EXPECT_EQ(3, lengths.size());
	EXPECT_NEAR(1.0, lengths[0], 1e-4);
	EXPECT_NEAR(1.0, lengths[1], 1e-4);
	EXPECT_NEAR(2.0, lengths[2], 1e-4);
	EXPECT_NEAR(4.0, total_length, 1e-4);

	list.clear();
	fill_list_open_rectangle(list);

	total_length = bline_length(list, loop, &lengths);
	EXPECT_EQ(4, lengths.size());
	EXPECT_NEAR(2.0, lengths[0], 1e-4);
	EXPECT_NEAR(1.0, lengths[1], 1e-4);
	EXPECT_NEAR(2.0, lengths[2], 1e-4);
	EXPECT_NEAR(1.0, lengths[3], 1e-4);
	EXPECT_NEAR(6.0, total_length, 1e-4);
}

TEST_F(BlineTest, test_bline_std_to_hom_without_bline_loop_without_index_loop_clamps_to_zero_for_negative_position)
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	EXPECT_NEAR(0.0, synfig::std_to_hom(list,  0.0, false, false, 1e-4));
	EXPECT_NEAR(0.0, synfig::std_to_hom(list, -1.0, false, false, 1e-4));
	EXPECT_NEAR(0.0, synfig::std_to_hom(list, -0.5, false, false, 1e-4));
}

TEST_F(BlineTest, test_bline_std_to_hom_without_bline_loop_without_index_loop_clamps_to_one_for_position_greater_than_one)
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	EXPECT_NEAR(1.0, synfig::std_to_hom(list, 1.0, false, false, 1e-4));
	EXPECT_NEAR(1.0, synfig::std_to_hom(list, 2.0, false, false, 1e-4));
	EXPECT_NEAR(1.0, synfig::std_to_hom(list, 2.5, false, false, 1e-4));
}

TEST_F(BlineTest, test_bline_std_to_hom_without_bline_loop_with_index_loop_keeps_position_for_both_edges)
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	EXPECT_NEAR( 0.0, synfig::std_to_hom(list,  0.0, true, false, 1e-4));
	EXPECT_NEAR(-1.0, synfig::std_to_hom(list, -1.0, true, false, 1e-4));
	EXPECT_NEAR(1.0, synfig::std_to_hom(list, 1.0, true, false, 1e-4));
	EXPECT_NEAR(2.0, synfig::std_to_hom(list, 2.0, true, false, 1e-4));
}

TEST_F(BlineTest, test_bline_std_to_hom_without_loop)
{
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.176000, 0.370370, 0.5, 0.629630, 0.824000, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i], false, false);
		EXPECT_NEAR(expected[i], value, 1e-6);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}
}

TEST_F(BlineTest, test_bline_std_to_hom_with_loop) {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.162, 0.25, 0.375, 0.5, 0.676, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i], false, true);
		EXPECT_NEAR(expected[i], value, 1e-6);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}
}

TEST_F(BlineTest, test_bline_hom_to_std_without_loop) {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.216466, 0.306518, 0.5, 0.693482, 0.783534, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i], false, false);
		EXPECT_NEAR(expected[i], value, 1e-6);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}
}

TEST_F(BlineTest, test_bline_hom_to_std_with_loop) {
	std::vector<ValueBase> list;
	fill_list_colinear(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	const std::vector<Real> expected = {0.0, 0.237620, 0.462321, 0.666667, 0.795654, 0.855690, 1.0};
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i], false, true);
		EXPECT_NEAR(expected[i], value, 1e-6);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}
}

TEST_F(BlineTest, test_calc_vertex) {
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
	EXPECT_NEAR((Vector(-2.219527)[0], (-0.653505)[0], 2e-6);
	EXPECT_NEAR((Vector(-2.219527)[1], (-0.653505)[1], 2e-6), vertex)

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.231653));
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(-1.650917)[0], (-0.224988)[0], 2e-6);
	EXPECT_NEAR((Vector(-1.650917)[1], (-0.224988)[1], 2e-6), vertex)

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.361874));
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(-1.010658)[0], (0.076790)[0], 2e-6);
	EXPECT_NEAR((Vector(-1.010658)[1], (0.076790)[1], 2e-6), vertex)

	bline_calc_vertex->set_link("amount", ValueNode_Const::create(0.507960));
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(-0.260716)[0], (0.341317)[0], 2e-6);
	EXPECT_NEAR((Vector(-0.260716)[1], (0.341317)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_bline_length_for_bline_with_two_vertices_at_same_spot)
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
	EXPECT_EQ(3, lengths.size());
	EXPECT_NEAR(1.0, lengths[0], 1e-4);
	EXPECT_NEAR(0.0, lengths[1], 1e-4);
	EXPECT_NEAR(1.0, lengths[2], 1e-4);
	EXPECT_NEAR(2.0, l, 1e-4);

	loop = true;
	l = bline_length(list, loop, &lengths);
	EXPECT_EQ(4, lengths.size());
	EXPECT_NEAR(1.0, lengths[0], 1e-4);
	EXPECT_NEAR(0.0, lengths[1], 1e-4);
	EXPECT_NEAR(1.0, lengths[2], 1e-4);
	EXPECT_NEAR(2.0, lengths[3], 1e-4);
	EXPECT_NEAR(4.0, l, 1e-4);
}

TEST_F(BlineTest, test_bline_std_to_hom_without_loop_with_two_vertices_at_same_spot) {
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
		EXPECT_NEAR(expected[i], value, 1e-4);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-4);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-4);
	}
}

TEST_F(BlineTest, test_bline_std_to_hom_with_loop_with_two_vertices_at_same_spot) {
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
		EXPECT_NEAR(expected[i], value, 1e-4);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-4);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::std_to_hom(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-4);
	}
}

TEST_F(BlineTest, test_bline_hom_to_std_without_loop_with_two_vertices_at_same_spot) {
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
		EXPECT_NEAR(expected[i], value, 1e-6);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, false);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}
}

TEST_F(BlineTest, test_bline_hom_to_std_with_loop_with_two_vertices_at_same_spot) {
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
		EXPECT_NEAR(expected[i], value, 1e-6);
	}

	// check index loop
	Real index_offset = 2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}

	index_offset = -2.0;
	for (size_t i = 0; i < positions.size(); ++i) {
		Real value = synfig::hom_to_std(list, positions[i] + index_offset, true, true);
		EXPECT_NEAR(expected[i] + index_offset, value, 1e-6);
	}
}

TEST_F(BlineTest, test_calc_vertex_for_single_vertex_without_loop_returns_itself)
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
	EXPECT_NEAR((Vector(1.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.0)[1], 2e-6), vertex)

	const_amount->set_value(0.3);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.0)[1], 2e-6), vertex)

	const_amount->set_value(0.8);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.0)[1], 2e-6), vertex)

	const_amount->set_value(1.0);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.0)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_calc_vertex_for_single_vertex_with_loop_returns_itself_on_edges)
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
	EXPECT_NEAR((Vector(1.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.0)[1], 2e-6), vertex)

	const_amount->set_value(0.3);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_FALSE(synfig::approximate_not_equal(1.0, vertex[0]));
	EXPECT_FALSE(synfig::approximate_not_equal(2.0, vertex[1]));

	const_amount->set_value(0.8);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_FALSE(synfig::approximate_not_equal(1.0, vertex[0]));
	EXPECT_FALSE(synfig::approximate_not_equal(2.0, vertex[1]));

	const_amount->set_value(1.0);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.0)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_calc_vertex_for_straight_line_without_loop) {
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
	EXPECT_NEAR((Vector(0.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.0)[1], 2e-6), vertex)

	const_amount->set_value(0.1);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.20)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.20)[1], 2e-6), vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.50)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.50)[1], 2e-6), vertex)

	const_amount->set_value(0.35);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.70)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.70)[1], 2e-6), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (1.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (1.00)[1], 2e-6), vertex)

	const_amount->set_value(0.80);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (1.60)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (1.60)[1], 2e-6), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (2.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (2.0)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_calc_vertex_for_straight_line_with_loop) {
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
	EXPECT_NEAR((Vector(0.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.0)[1], 2e-6), vertex)

	const_amount->set_value(0.1);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.40)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.40)[1], 2e-6), vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (1.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (1.00)[1], 2e-6), vertex)

	const_amount->set_value(0.35);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (1.40)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (1.40)[1], 2e-6), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.75);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (1.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (1.00)[1], 2e-6), vertex)

	const_amount->set_value(0.80);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.80)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.80)[1], 2e-6), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.0)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_calc_vertex_for_open_rectangle) {
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
	EXPECT_NEAR((Vector(0.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.0)[1], 2e-6), vertex)

	const_amount->set_value(0.10);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.50)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.50)[1], 2e-6), vertex)

	const_amount->set_value(0.40);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.5)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.5)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.60);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.80);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (1.00)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (1.00)[1], 2e-6), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (0.0)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_calc_vertex_for_closed_rectangle) {
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
	EXPECT_NEAR((Vector(0.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.0)[1], 2e-6), vertex)

	const_amount->set_value(0.1);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.60)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.60)[1], 2e-6), vertex)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (1.50)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (1.50)[1], 2e-6), vertex)

	const_amount->set_value(1/3.);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.40);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.4)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(0.4)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.50);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (2.00)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (2.00)[1], 2e-6), vertex)

	const_amount->set_value(0.75);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (0.50)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (0.50)[1], 2e-6), vertex)

	const_amount->set_value(5/6.);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(1.0)[0], (0.00)[0], 2e-6);
	EXPECT_NEAR((Vector(1.0)[1], (0.00)[1], 2e-6), vertex)

	const_amount->set_value(0.90);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.6)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.6)[1], (0.0)[1], 2e-6), vertex)

	const_amount->set_value(1.00);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((Vector(0.0)[0], (0.0)[0], 2e-6);
	EXPECT_NEAR((Vector(0.0)[1], (0.0)[1], 2e-6), vertex)
}

TEST_F(BlineTest, test_calc_vertex_on_vertex_exact_positions_for_looped_curve_with_two_vertices_on_same_coords) {
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
	EXPECT_NEAR((bpoints[0].vertex)[0], (vertex)[0], 2e-6);
	EXPECT_NEAR((bpoints[0].vertex)[1], (vertex)[1], 2e-6)

	const_amount->set_value(0.25);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((bpoints[1].vertex)[0], (vertex)[0], 2e-6);
	EXPECT_NEAR((bpoints[1].vertex)[1], (vertex)[1], 2e-6)

	const_amount->set_value(0.5);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((bpoints[2].vertex)[0], (vertex)[0], 2e-6);
	EXPECT_NEAR((bpoints[2].vertex)[1], (vertex)[1], 2e-6)

	const_amount->set_value(0.75);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((bpoints[3].vertex)[0], (vertex)[0], 2e-6);
	EXPECT_NEAR((bpoints[3].vertex)[1], (vertex)[1], 2e-6)

	const_amount->set_value(1.0);
	vertex = (*bline_calc_vertex)(Time()).get(Vector());
	EXPECT_NEAR((bpoints[0].vertex)[0], (vertex)[0], 2e-6);
	EXPECT_NEAR((bpoints[0].vertex)[1], (vertex)[1], 2e-6)
}
