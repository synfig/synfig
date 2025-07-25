/* === S Y N F I G ========================================================= */
/*! \file angle.cpp
**  \brief Test Angle class
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2025 Synfig Contributors
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

#include "../advancedline.h"

#include "../test/test_base.h"

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === C L A S S E S ======================================================= */


/* === F U N C T I O N S =================================================== */

void
test_adding_point_with_replace_action_does_include_if_none_exists_at_same_position()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED, AdvancedLine::AddAction::REPLACE_IF_EXISTS);
	ASSERT_EQUAL(1, line.size());
	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);
}

void
test_adding_point_with_append_action_does_include_if_none_exists_at_same_position()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED, AdvancedLine::AddAction::APPEND);
	ASSERT_EQUAL(1, line.size());
	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);
}

void
test_adding_point_with_prepend_action_does_include_if_none_exists_at_same_position()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED, AdvancedLine::AddAction::PREPEND);
	ASSERT_EQUAL(1, line.size());
	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);
}

void
test_adding_point_with_replace_action_does_replace_previous_point_if_it_exists_at_same_position()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);

	line.add(0.1, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED, AdvancedLine::AddAction::REPLACE_IF_EXISTS);
	ASSERT_EQUAL(1, line.size());
	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(3.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.begin()->second.side1);
}

void
test_adding_point_with_append_action_changes_the_outgoing_side_of_the_existent_point_at_same_position()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);

	line.add(0.1, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED, AdvancedLine::AddAction::APPEND);
	ASSERT_EQUAL(1, line.size());
	ASSERT_EQUAL(0.1, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

void
test_adding_point_with_prepend_action_changes_the_incoming_side_of_the_existent_point_at_same_position()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);

	line.add(0.1, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED, AdvancedLine::AddAction::PREPEND);
	ASSERT_EQUAL(1, line.size());
	ASSERT_EQUAL(0.1, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.rbegin()->second.side1);
}

void
test_trunc_left_before_the_line_starting_point_does_not_change_anything()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);
	ASSERT_EQUAL(2, line.size());

	line.trunc_left(0.05, WidthPoint::TYPE_FLAT);
	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

void
test_trunc_left_after_the_line_starting_point()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_INTERPOLATE);
	line.add(0.5, 2., WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INNER_ROUNDED);
	ASSERT_EQUAL(2, line.size());

	line.calc_tangents(0);
	line.trunc_left(0.2, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(2, line.size());

	ASSERT_EQUAL(0.2, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_FLAT, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INTERPOLATE, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(2.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INTERPOLATE, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

// void
// test_adding_point_with_prepend_action_changes_the_incoming_side_of_the_existent_point_at_same_position()
// {
// 	AdvancedLine line;

// 	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);

// 	line.add(0.1, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED, AdvancedLine::AddAction::PREPEND);
// 	ASSERT_EQUAL(1, line.size());
// 	ASSERT_EQUAL(0.1, line.rbegin()->first);
// 	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
// 	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
// 	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.rbegin()->second.side1);
// }

void
test_cutting_before_the_line_starting_point_does_not_change_anything()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);

	line.cut(0.05, 0.08, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(2, line.size());

	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

void
test_cutting_after_the_line_ending_point_does_not_change_anything()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);

	line.cut(0.6, 0.8, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(2, line.size());

	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

void
test_cutting_left_at_the_line_starting_point_does_not_change_anything_but_the_side_type()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);

	line.cut(0.05, 0.1, WidthPoint::TYPE_PEAK, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(2, line.size());

	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_FLAT, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

void
test_cutting_with_the_line_starting_point_in_the_middle_changes_the_starting_point_and_the_side_type()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_INTERPOLATE);
	line.add(0.5, 3., WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INNER_ROUNDED);

	line.calc_tangents(0);
	line.cut(0.05, 0.3, WidthPoint::TYPE_PEAK, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(2, line.size());

	ASSERT_EQUAL(0.3, line.begin()->first);
	ASSERT_APPROX_EQUAL(2.5, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_FLAT, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INTERPOLATE, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INTERPOLATE, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

void
test_cutting_with_the_isolated_starting_point_in_the_middle_removes_it_and_changes_the_starting_point()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);

	line.calc_tangents(0);
	line.cut(0.05, 0.3, WidthPoint::TYPE_PEAK, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(1, line.size());

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
}

// void
// test_cutting_with_the_line_starting_point_in_the_middle_changes_the_starting_point_and_the_side_type()
// {
// 	AdvancedLine line;

// 	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
// 	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);

// 	line.cut(0.05, 0.3, WidthPoint::TYPE_PEAK, WidthPoint::TYPE_FLAT);

// 	ASSERT_EQUAL(2, line.size());

// 	ASSERT_EQUAL(0.3, line.begin()->first);
// 	ASSERT_EQUAL(2.5, line.begin()->second.w);
// 	ASSERT_EQUAL(WidthPoint::TYPE_FLAT, line.begin()->second.side0);
// 	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);

// 	ASSERT_EQUAL(0.5, line.rbegin()->first);
// 	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
// 	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
// 	ASSERT_EQUAL(WidthPoint::TYPE_INNER_ROUNDED, line.rbegin()->second.side1);
// }

void
test_cutting_right_at_the_line_ending_point_does_not_change_anything_but_the_side_type()
{
	AdvancedLine line;

	line.add(0.1, 2., WidthPoint::TYPE_SQUARED, WidthPoint::TYPE_ROUNDED);
	line.add(0.5, 3., WidthPoint::TYPE_INNER_PEAK, WidthPoint::TYPE_INNER_ROUNDED);

	line.cut(0.5, 0.8, WidthPoint::TYPE_PEAK, WidthPoint::TYPE_FLAT);

	ASSERT_EQUAL(2, line.size());

	ASSERT_EQUAL(0.1, line.begin()->first);
	ASSERT_EQUAL(2.0, line.begin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_SQUARED, line.begin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_ROUNDED, line.begin()->second.side1);

	ASSERT_EQUAL(0.5, line.rbegin()->first);
	ASSERT_EQUAL(3.0, line.rbegin()->second.w);
	ASSERT_EQUAL(WidthPoint::TYPE_INNER_PEAK, line.rbegin()->second.side0);
	ASSERT_EQUAL(WidthPoint::TYPE_PEAK, line.rbegin()->second.side1);
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	TEST_SUITE_BEGIN();

	TEST_FUNCTION(test_adding_point_with_replace_action_does_include_if_none_exists_at_same_position);
	TEST_FUNCTION(test_adding_point_with_append_action_does_include_if_none_exists_at_same_position);
	TEST_FUNCTION(test_adding_point_with_prepend_action_does_include_if_none_exists_at_same_position);
	TEST_FUNCTION(test_adding_point_with_replace_action_does_replace_previous_point_if_it_exists_at_same_position);
	TEST_FUNCTION(test_adding_point_with_append_action_changes_the_outgoing_side_of_the_existent_point_at_same_position);
	TEST_FUNCTION(test_adding_point_with_prepend_action_changes_the_incoming_side_of_the_existent_point_at_same_position);

	TEST_FUNCTION(test_trunc_left_before_the_line_starting_point_does_not_change_anything);
	TEST_FUNCTION(test_trunc_left_after_the_line_starting_point);
	// TEST_FUNCTION(test_trunc_left_exactly_where_the_line_starts);

	TEST_FUNCTION(test_cutting_before_the_line_starting_point_does_not_change_anything);
	TEST_FUNCTION(test_cutting_after_the_line_ending_point_does_not_change_anything);
	TEST_FUNCTION(test_cutting_left_at_the_line_starting_point_does_not_change_anything_but_the_side_type);
	TEST_FUNCTION(test_cutting_with_the_line_starting_point_in_the_middle_changes_the_starting_point_and_the_side_type);
	TEST_FUNCTION(test_cutting_with_the_isolated_starting_point_in_the_middle_removes_it_and_changes_the_starting_point);
	TEST_FUNCTION(test_cutting_right_at_the_line_ending_point_does_not_change_anything_but_the_side_type);

	TEST_SUITE_END();

	return tst_exit_status;
}
