/* === S Y N F I G ========================================================= */
/*!	\file test_bline.cpp
**	\brief Test BLine class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2020 Synfig contributors
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

#include <synfig/blinepoint.h>
#include <synfig/real.h>
#include <synfig/value.h>
#include <synfig/type.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_const.h>

#include<synfig/general.h>

#include <vector>

#include <iostream>

using namespace etl;
using namespace synfig;

std::ostream& operator<<(std::ostream& os, const Vector& v)
{
    os << '(' << v[0] << ',' << v[1] << ')';
    return os;
}

#define ERROR_MESSAGE_TWO_VALUES(a, b) \
	std::cerr.precision(8); \
	std::cerr << __FUNCTION__ << ":" << __LINE__ << " - expected " << a << ", but got " << b << std::endl;

#define ASSERT_EQUAL(expected, value) {\
	if (expected != value) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_APPROX_EQUAL(expected, value) {\
	if (!synfig::approximate_equal(expected, value)) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected - value) > 1e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_VECTOR_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected[0] - value[0]) > 2e-6 || std::abs(expected[1] - value[1]) > 2e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

void fill_list(std::vector<ValueBase> &list) {
	BLinePoint p;
	list.push_back(p);
	p.set_vertex(Point(0.0,1.0));
	list.push_back(p);
	p.set_vertex(Point(0.0,2.0));
	list.push_back(p);
}

bool test_bline_length() {
	std::vector<ValueBase> list;
	fill_list(list);
	
	bool loop = false;
	std::vector<Real> lengths;

	Real l = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(2, lengths.size());
	ASSERT_APPROX_EQUAL(1.0, lengths[0]);
	ASSERT_APPROX_EQUAL(1.0, lengths[1]);
	ASSERT_APPROX_EQUAL(2.0, l);

	loop = true;
	l = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(3, lengths.size());
	ASSERT_APPROX_EQUAL(1.0, lengths[0]);
	ASSERT_APPROX_EQUAL(1.0, lengths[1]);
	ASSERT_APPROX_EQUAL(2.0, lengths[2]);
	ASSERT_APPROX_EQUAL(4.0, l);

	BLinePoint p1;
	p1.set_tangent1(Point(-1,0));
	p1.set_tangent2(Point(1,0));

	list.clear();
	list.push_back(p1);
	lengths.clear();
	// single point
	loop = false;
	l = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(0, lengths.size());
	ASSERT_APPROX_EQUAL(0.0, l);

	loop = true;
	l = bline_length(list, loop, &lengths);
	ASSERT_EQUAL(1, lengths.size());
	ASSERT_APPROX_EQUAL_MICRO(0.349854, l);

	return false;
}

bool test_bline_std_to_hom_without_loop() {
	std::vector<ValueBase> list;
	fill_list(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	std::vector<Real> expected = {0.0, 0.176000, 0.370370, 0.5, 0.629630, 0.824000, 1.0};
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

	return false;
}

bool test_bline_std_to_hom_with_loop() {
	std::vector<ValueBase> list;
	fill_list(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	std::vector<Real> expected = {0.0, 0.162, 0.25, 0.375, 0.5, 0.676, 1.0};
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

	return false;
}

bool test_bline_hom_to_std_without_loop() {
	std::vector<ValueBase> list;
	fill_list(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	std::vector<Real> expected = {0.0, 0.216466, 0.306518, 0.5, 0.693482, 0.783534, 1.0};
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

	return false;
}

bool test_bline_hom_to_std_with_loop() {
	std::vector<ValueBase> list;
	fill_list(list);

	const std::vector<Real> positions = {0.0, 0.2, 1/3., 0.5, 2/3., 0.8, 1.0};
	std::vector<Real> expected = {0.0, 0.237620, 0.462321, 0.666667, 0.795654, 0.855690, 1.0};
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

	return false;
}

bool test_calc_vertex() {
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

	ValueNode_BLine *bline = ValueNode_BLine::create(list);
	
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

	return false;
}


#define TEST_FUNCTION(function_name) {\
	fail = function_name(); \
	if (fail) { \
		error("%s FAILED", #function_name); \
		failures++; \
	} \
}

int main() {
	Type::subsys_init();

	int failures = 0;
	bool fail;
	bool exception_thrown = false;

	try {
		TEST_FUNCTION(test_bline_length)
		TEST_FUNCTION(test_bline_std_to_hom_without_loop)
		TEST_FUNCTION(test_bline_std_to_hom_with_loop)
		TEST_FUNCTION(test_bline_hom_to_std_without_loop)
		TEST_FUNCTION(test_bline_hom_to_std_with_loop)
		TEST_FUNCTION(test_calc_vertex)
	} catch (...) {
		error("Some exception has been thrown.");
		exception_thrown = true;
	}

	if (failures || exception_thrown)
		error("Test finished with %i errors and %i exception", failures, exception_thrown);
	else
		info("Success");

	Type::subsys_stop();

	return (failures || exception_thrown)? 1 : 0;
}
