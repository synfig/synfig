/* === S Y N F I G ========================================================= */
/*! \file valuenode_maprange.cpp
**  \brief Test synfig::ValueNode_MapRange methods
**
**  \legal
**  Copyright (c) 2025 Synfig contributors
**
**  This file is part of Synfig.
**
**  Synfig is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  Synfig is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**  \endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <synfig/valuenodes/valuenode_maprange.h>

#include "test_base.h"

#include <synfig/valuenodes/valuenode_const.h>

using namespace synfig;

/* === P R O C E D U R E S ================================================= */

static void
test_int_map_range_clamp_if_below_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(int(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));
}

static void
test_int_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(int(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-14, (*map_range)(0).get(int()));
}

static void
test_int_map_range_clamp_if_above_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(int(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(10, (*map_range)(0).get(int()));
}

static void
test_int_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(int(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(14, (*map_range)(0).get(int()));
}

static void
test_int_map_range_matches_from_to_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(2));
	map_range->set_link("to_max", ValueNode_Const::create(13));
	ASSERT_EQUAL(2, (*map_range)(0).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(2, (*map_range)(0).get(int()));
}

static void
test_int_map_range_matches_from_to_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(int(5));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(2));
	map_range->set_link("to_max", ValueNode_Const::create(13));
	ASSERT_EQUAL(13, (*map_range)(0).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(13, (*map_range)(0).get(int()));
}

static void
test_int_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(int(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-2, (*map_range)(0).get(int()));
}

static void
test_int_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(int(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(5));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));
}

static void
test_int_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(int(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(5));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));
}

static void
test_int_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(int(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(5));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(10, (*map_range)(0).get(int()));

	map_range->set_link("link", ValueNode_Const::create(5));
	ASSERT_EQUAL(10, (*map_range)(0).get(int()));
}

static void
test_int_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value()
{
	auto map_range = ValueNode_MapRange::create(int(-2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(-10));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));

	map_range->set_link("link", ValueNode_Const::create(0));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));

	map_range->set_link("link", ValueNode_Const::create(2));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));

	map_range->set_link("link", ValueNode_Const::create(5));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));

	map_range->set_link("link", ValueNode_Const::create(7));
	ASSERT_EQUAL(-10, (*map_range)(0).get(int()));
}

static void
test_int_invert_map_range_works_when_below_target_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-1, map_range->get_inverse(0, -14).get(int()));
}

static void
test_int_invert_map_range_works_when_below_target_lower_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(0, map_range->get_inverse(0, -14).get(int()));
}

static void
test_int_invert_map_range_works_when_above_target_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(6, map_range->get_inverse(0, 14).get(int()));
}

static void
test_int_invert_map_range_works_when_above_target_upper_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 14).get(int()));
}

static void
test_int_invert_map_range_matches_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(0, map_range->get_inverse(0, -10).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(0, map_range->get_inverse(0, -10).get(int()));
}

static void
test_int_invert_map_range_matches_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 10).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 10).get(int()));
}

static void
test_int_invert_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(int(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(0));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(4, map_range->get_inverse(0, 6).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(4, map_range->get_inverse(0, 6).get(int()));
}

static void
test_int_invert_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(int(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(5));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT(map_range->get_inverse(0, 6).is_valid());
}

static void
test_int_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value()
{
	auto map_range = ValueNode_MapRange::create(int(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(5));
	map_range->set_link("from_max", ValueNode_Const::create(5));
	map_range->set_link("to_min", ValueNode_Const::create(-10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(5, map_range->get_inverse(0, -10).get(int()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 10).get(int()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 3).get(int()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, -15).get(int()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, +15).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(5, map_range->get_inverse(0, -15).get(int()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, +15).get(int()));
}

static void
test_int_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(int(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(-5));
	map_range->set_link("from_max", ValueNode_Const::create(+5));
	map_range->set_link("to_min", ValueNode_Const::create(+10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(-5, map_range->get_inverse(0, -15).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(-5, map_range->get_inverse(0, -15).get(int()));
}

static void
test_int_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(int(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(-5));
	map_range->set_link("from_max", ValueNode_Const::create(+5));
	map_range->set_link("to_min", ValueNode_Const::create(+10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +15).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +15).get(int()));
}

static void
test_int_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(int(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(-5));
	map_range->set_link("from_max", ValueNode_Const::create(+5));
	map_range->set_link("to_min", ValueNode_Const::create(+10));
	map_range->set_link("to_max", ValueNode_Const::create(+10));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +10).get(int()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +10).get(int()));
}






static void
test_angle_map_range_clamp_if_below_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-14), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_clamp_if_above_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(10), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(14), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_matches_from_to_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(2)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(13)));
	ASSERT_EQUAL(Angle::deg(2), (*map_range)(0).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Angle::deg(2), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_matches_from_to_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(5));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(2)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(13)));
	ASSERT_EQUAL(Angle::deg(13), (*map_range)(0).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Angle::deg(13), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-2), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(10), (*map_range)(0).get(Angle()));

	map_range->set_link("link", ValueNode_Const::create(Angle::deg(5)));
	ASSERT_EQUAL(Angle::deg(10), (*map_range)(0).get(Angle()));
}

static void
test_angle_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(-2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(-10)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));

	map_range->set_link("link", ValueNode_Const::create(Angle::deg(0)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));

	map_range->set_link("link", ValueNode_Const::create(Angle::deg(2)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));

	map_range->set_link("link", ValueNode_Const::create(Angle::deg(5)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));

	map_range->set_link("link", ValueNode_Const::create(Angle::deg(7)));
	ASSERT_EQUAL(Angle::deg(-10), (*map_range)(0).get(Angle()));
}

static void
test_angle_invert_map_range_works_when_below_target_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-1), map_range->get_inverse(0, Angle::deg(-14)).get(Angle()));
}

static void
test_angle_invert_map_range_works_when_below_target_lower_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(0), map_range->get_inverse(0, Angle::deg(-14)).get(Angle()));
}

static void
test_angle_invert_map_range_works_when_above_target_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(6), map_range->get_inverse(0, Angle::deg(14)).get(Angle()));
}

static void
test_angle_invert_map_range_works_when_above_target_upper_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(14)).get(Angle()));
}

static void
test_angle_invert_map_range_matches_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(0), map_range->get_inverse(0, Angle::deg(-10)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Angle::deg(0), map_range->get_inverse(0, Angle::deg(-10)).get(Angle()));
}

static void
test_angle_invert_map_range_matches_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(10)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(10)).get(Angle()));
}

static void
test_angle_invert_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(4), map_range->get_inverse(0, Angle::deg(6)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Angle::deg(4), map_range->get_inverse(0, Angle::deg(6)).get(Angle()));
}

static void
test_angle_invert_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT(map_range->get_inverse(0, Angle::deg(6)).is_valid());
}

static void
test_angle_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(-10)).get(Angle()));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(10)).get(Angle()));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(3)).get(Angle()));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(-15)).get(Angle()));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(+15)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(-15)).get(Angle()));
	ASSERT_EQUAL(Angle::deg(5), map_range->get_inverse(0, Angle::deg(+15)).get(Angle()));
}

static void
test_angle_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(-5), map_range->get_inverse(0, Angle::deg(-15)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Angle::deg(-5), map_range->get_inverse(0, Angle::deg(-15)).get(Angle()));
}

static void
test_angle_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(+5), map_range->get_inverse(0, Angle::deg(+15)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Angle::deg(+5), map_range->get_inverse(0, Angle::deg(+15)).get(Angle()));
}

static void
test_angle_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Angle::deg(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Angle::deg(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Angle::deg(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Angle::deg(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Angle::deg(+10)));
	ASSERT_EQUAL(Angle::deg(+5), map_range->get_inverse(0, Angle::deg(+10)).get(Angle()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Angle::deg(+5), map_range->get_inverse(0, Angle::deg(+10)).get(Angle()));
}







static void
test_real_map_range_clamp_if_below_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Real(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Real(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-14, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_clamp_if_above_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Real(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(10, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Real(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(14, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_matches_from_to_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(2)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(13)));
	ASSERT_EQUAL(2, (*map_range)(0).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(2, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_matches_from_to_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Real(5));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(2)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(13)));
	ASSERT_EQUAL(13, (*map_range)(0).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(13, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Real(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-2, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Real(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Real(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Real(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(10, (*map_range)(0).get(Real()));

	map_range->set_link("link", ValueNode_Const::create(Real(5)));
	ASSERT_EQUAL(10, (*map_range)(0).get(Real()));
}

static void
test_real_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value()
{
	auto map_range = ValueNode_MapRange::create(Real(-2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(-10)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));

	map_range->set_link("link", ValueNode_Const::create(Real(0)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));

	map_range->set_link("link", ValueNode_Const::create(Real(2)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));

	map_range->set_link("link", ValueNode_Const::create(Real(5)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));

	map_range->set_link("link", ValueNode_Const::create(Real(7)));
	ASSERT_EQUAL(-10, (*map_range)(0).get(Real()));
}

static void
test_real_invert_map_range_works_when_below_target_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-1, map_range->get_inverse(0, -14.).get(Real()));
}

static void
test_real_invert_map_range_works_when_below_target_lower_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(0, map_range->get_inverse(0, -14.).get(Real()));
}

static void
test_real_invert_map_range_works_when_above_target_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(6, map_range->get_inverse(0, 14.).get(Real()));
}

static void
test_real_invert_map_range_works_when_above_target_upper_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 14.).get(Real()));
}

static void
test_real_invert_map_range_matches_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(0, map_range->get_inverse(0, -10.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(0, map_range->get_inverse(0, -10.).get(Real()));
}

static void
test_real_invert_map_range_matches_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 10.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 10.).get(Real()));
}

static void
test_real_invert_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Real(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Real(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(4, map_range->get_inverse(0, 6.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(4, map_range->get_inverse(0, 6.).get(Real()));
}

static void
test_real_invert_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Real(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT(map_range->get_inverse(0, 6.).is_valid());
}

static void
test_real_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value()
{
	auto map_range = ValueNode_MapRange::create(Real(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(5, map_range->get_inverse(0, -10.).get(Real()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 10.).get(Real()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, 3.).get(Real()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, -15.).get(Real()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, +15.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(5, map_range->get_inverse(0, -15.).get(Real()));
	ASSERT_EQUAL(5, map_range->get_inverse(0, +15.).get(Real()));
}

static void
test_real_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Real(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(-5, map_range->get_inverse(0, -15.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(-5, map_range->get_inverse(0, -15.).get(Real()));
}

static void
test_real_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Real(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +15.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +15.).get(Real()));
}

static void
test_real_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Real(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Real(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Real(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Real(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Real(+10)));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +10.).get(Real()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(+5, map_range->get_inverse(0, +10.).get(Real()));
}







static void
test_time_map_range_clamp_if_below_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Time(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Time(-1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-14), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_clamp_if_above_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Time(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(10), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Time(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(14), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_matches_from_to_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(2)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(13)));
	ASSERT_EQUAL(Time(2), (*map_range)(0).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Time(2), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_matches_from_to_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Time(5));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(2)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(13)));
	ASSERT_EQUAL(Time(13), (*map_range)(0).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Time(13), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Time(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-2), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Time(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Time(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Time(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(10), (*map_range)(0).get(Time()));

	map_range->set_link("link", ValueNode_Const::create(Time(5)));
	ASSERT_EQUAL(Time(10), (*map_range)(0).get(Time()));
}

static void
test_time_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value()
{
	auto map_range = ValueNode_MapRange::create(Time(-2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(-10)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));

	map_range->set_link("link", ValueNode_Const::create(Time(0)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));

	map_range->set_link("link", ValueNode_Const::create(Time(2)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));

	map_range->set_link("link", ValueNode_Const::create(Time(5)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));

	map_range->set_link("link", ValueNode_Const::create(Time(7)));
	ASSERT_EQUAL(Time(-10), (*map_range)(0).get(Time()));
}

static void
test_time_invert_map_range_works_when_below_target_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-1), map_range->get_inverse(0, Time(-14)).get(Time()));
}

static void
test_time_invert_map_range_works_when_below_target_lower_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(0), map_range->get_inverse(0, Time(-14)).get(Time()));
}

static void
test_time_invert_map_range_works_when_above_target_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(6), map_range->get_inverse(0, Time(14)).get(Time()));
}

static void
test_time_invert_map_range_works_when_above_target_upper_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(14)).get(Time()));
}

static void
test_time_invert_map_range_matches_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(0), map_range->get_inverse(0, Time(-10)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Time(0), map_range->get_inverse(0, Time(-10)).get(Time()));
}

static void
test_time_invert_map_range_matches_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(10)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(10)).get(Time()));
}

static void
test_time_invert_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Time(0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Time(0)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(4), map_range->get_inverse(0, Time(6)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Time(4), map_range->get_inverse(0, Time(6)).get(Time()));
}

static void
test_time_invert_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Time(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT(map_range->get_inverse(0, Time(6)).is_valid());
}

static void
test_time_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value()
{
	auto map_range = ValueNode_MapRange::create(Time(2));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(-10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(-10)).get(Time()));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(10)).get(Time()));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(3)).get(Time()));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(-15)).get(Time()));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(+15)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(-15)).get(Time()));
	ASSERT_EQUAL(Time(5), map_range->get_inverse(0, Time(+15)).get(Time()));
}

static void
test_time_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Time(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(-5), map_range->get_inverse(0, Time(-15)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Time(-5), map_range->get_inverse(0, Time(-15)).get(Time()));
}

static void
test_time_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Time(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(+5), map_range->get_inverse(0, Time(+15)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Time(+5), map_range->get_inverse(0, Time(+15)).get(Time()));
}

static void
test_time_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Time(6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Time(-5)));
	map_range->set_link("from_max", ValueNode_Const::create(Time(+5)));
	map_range->set_link("to_min", ValueNode_Const::create(Time(+10)));
	map_range->set_link("to_max", ValueNode_Const::create(Time(+10)));
	ASSERT_EQUAL(Time(+5), map_range->get_inverse(0, Time(+10)).get(Time()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Time(+5), map_range->get_inverse(0, Time(+10)).get(Time()));
}







static void
test_vector_map_range_clamp_if_below_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Vector(-1, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Vector(-1, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-14, -13), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_clamp_if_above_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Vector(6, 7));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(10, 11), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off()
{
	auto map_range = ValueNode_MapRange::create(Vector(6, 7));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(14, 15), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_matches_from_to_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(2, 3)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(13, 14)));
	ASSERT_EQUAL(Vector(2, 3), (*map_range)(0).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Vector(2, 3), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_matches_from_to_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Vector(5, 6));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(2, 3)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(13, 14)));
	ASSERT_EQUAL(Vector(13, 14), (*map_range)(0).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Vector(13, 14), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Vector(2, 3));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-2, -1), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Vector(2, 3));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Vector(2, 3));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range()
{
	auto map_range = ValueNode_MapRange::create(Vector(6, 7));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(10, 11), (*map_range)(0).get(Vector()));

	map_range->set_link("link", ValueNode_Const::create(Vector(5, 6)));
	ASSERT_EQUAL(Vector(10, 11), (*map_range)(0).get(Vector()));
}

static void
test_vector_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value()
{
	auto map_range = ValueNode_MapRange::create(Vector(-2, -1));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(-10, -9)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));

	map_range->set_link("link", ValueNode_Const::create(Vector(0, 1)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));

	map_range->set_link("link", ValueNode_Const::create(Vector(2, 3)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));

	map_range->set_link("link", ValueNode_Const::create(Vector(5, 6)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));

	map_range->set_link("link", ValueNode_Const::create(Vector(7, 8)));
	ASSERT_EQUAL(Vector(-10, -9), (*map_range)(0).get(Vector()));
}

static void
test_vector_invert_map_range_works_when_below_target_lower_bound()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-1, 0), map_range->get_inverse(0, Vector(-14, -13)).get(Vector()));
}

static void
test_vector_invert_map_range_works_when_below_target_lower_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(0, 1), map_range->get_inverse(0, Vector(-14, -13)).get(Vector()));
}

static void
test_vector_invert_map_range_works_when_above_target_upper_bound()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(6, 7), map_range->get_inverse(0, Vector(14, 15)).get(Vector()));
}

static void
test_vector_invert_map_range_works_when_above_target_upper_bound_even_if_clamped()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(14, 15)).get(Vector()));
}

static void
test_vector_invert_map_range_matches_lower_bounds()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(0, 1), map_range->get_inverse(0, Vector(-10, -9)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Vector(0, 1), map_range->get_inverse(0, Vector(-10, -9)).get(Vector()));
}

static void
test_vector_invert_map_range_matches_upper_bounds()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(10, 11)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(10, 11)).get(Vector()));
}

static void
test_vector_invert_map_range_inside_bounds()
{
	auto map_range = ValueNode_MapRange::create(Vector(0, 0));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(true));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(0, 1)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(4, 5), map_range->get_inverse(0, Vector(6, 7)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(false));
	ASSERT_EQUAL(Vector(4, 5), map_range->get_inverse(0, Vector(6, 7)).get(Vector()));
}

static void
test_vector_invert_map_range_does_not_crash_if_source_range_is_null()
{
	auto map_range = ValueNode_MapRange::create(Vector(2, 3));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT(map_range->get_inverse(0, Vector(6, 7)).is_valid());
}

static void
test_vector_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value()
{
	auto map_range = ValueNode_MapRange::create(Vector(2, 3));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(5, 6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(-10, -9)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(-10, -9)).get(Vector()));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(10, 11)).get(Vector()));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(3, 4)).get(Vector()));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(-15, -14)).get(Vector()));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(+15, +16)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(-15, -14)).get(Vector()));
	ASSERT_EQUAL(Vector(5, 6), map_range->get_inverse(0, Vector(+15, +16)).get(Vector()));
}

static void
test_vector_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Vector(6, 7));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(-5, -4)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(+5, +6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(+10, +11)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(-5, -4), map_range->get_inverse(0, Vector(-15, -14)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Vector(-5, -4), map_range->get_inverse(0, Vector(-15, -14)).get(Vector()));
}

static void
test_vector_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Vector(6, 7));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(-5, -4)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(+5, +6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(+10, +11)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(+5, +6), map_range->get_inverse(0, Vector(+15, +16)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Vector(+5, +6), map_range->get_inverse(0, Vector(+15, +16)).get(Vector()));
}

static void
test_vector_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value()
{
	auto map_range = ValueNode_MapRange::create(Vector(6, 7));
	map_range->set_link("interpolation", ValueNode_Const::create(0));
	map_range->set_link("clamp", ValueNode_Const::create(false));
	map_range->set_link("from_min", ValueNode_Const::create(Vector(-5, -4)));
	map_range->set_link("from_max", ValueNode_Const::create(Vector(+5, +6)));
	map_range->set_link("to_min", ValueNode_Const::create(Vector(+10, +11)));
	map_range->set_link("to_max", ValueNode_Const::create(Vector(+10, +11)));
	ASSERT_EQUAL(Vector(+5, +6), map_range->get_inverse(0, Vector(+10, +11)).get(Vector()));

	map_range->set_link("clamp", ValueNode_Const::create(true));
	ASSERT_EQUAL(Vector(+5, +6), map_range->get_inverse(0, Vector(+10, +11)).get(Vector()));
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	Type::subsys_init();

	TEST_SUITE_BEGIN()

	TEST_FUNCTION(test_int_map_range_clamp_if_below_lower_bound);
	TEST_FUNCTION(test_int_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off);
	TEST_FUNCTION(test_int_map_range_clamp_if_above_upper_bound);
	TEST_FUNCTION(test_int_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off);

	TEST_FUNCTION(test_int_map_range_matches_from_to_lower_bounds);
	TEST_FUNCTION(test_int_map_range_matches_from_to_upper_bounds);

	TEST_FUNCTION(test_int_map_range_inside_bounds);

	TEST_FUNCTION(test_int_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_int_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range);
	TEST_FUNCTION(test_int_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range);

	TEST_FUNCTION(test_int_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value);

	TEST_FUNCTION(test_int_invert_map_range_works_when_below_target_lower_bound);
	TEST_FUNCTION(test_int_invert_map_range_works_when_below_target_lower_bound_even_if_clamped);
	TEST_FUNCTION(test_int_invert_map_range_works_when_above_target_upper_bound);
	TEST_FUNCTION(test_int_invert_map_range_works_when_above_target_upper_bound_even_if_clamped);

	TEST_FUNCTION(test_int_invert_map_range_matches_lower_bounds);
	TEST_FUNCTION(test_int_invert_map_range_matches_upper_bounds);

	TEST_FUNCTION(test_int_invert_map_range_inside_bounds);

	TEST_FUNCTION(test_int_invert_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_int_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value);
	TEST_FUNCTION(test_int_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value);
	TEST_FUNCTION(test_int_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value);
	TEST_FUNCTION(test_int_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value);



	TEST_FUNCTION(test_angle_map_range_clamp_if_below_lower_bound);
	TEST_FUNCTION(test_angle_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off);
	TEST_FUNCTION(test_angle_map_range_clamp_if_above_upper_bound);
	TEST_FUNCTION(test_angle_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off);

	TEST_FUNCTION(test_angle_map_range_matches_from_to_lower_bounds);
	TEST_FUNCTION(test_angle_map_range_matches_from_to_upper_bounds);

	TEST_FUNCTION(test_angle_map_range_inside_bounds);

	TEST_FUNCTION(test_angle_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_angle_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range);
	TEST_FUNCTION(test_angle_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range);

	TEST_FUNCTION(test_angle_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value);

	TEST_FUNCTION(test_angle_invert_map_range_works_when_below_target_lower_bound);
	TEST_FUNCTION(test_angle_invert_map_range_works_when_below_target_lower_bound_even_if_clamped);
	TEST_FUNCTION(test_angle_invert_map_range_works_when_above_target_upper_bound);
	TEST_FUNCTION(test_angle_invert_map_range_works_when_above_target_upper_bound_even_if_clamped);

	TEST_FUNCTION(test_angle_invert_map_range_matches_lower_bounds);
	TEST_FUNCTION(test_angle_invert_map_range_matches_upper_bounds);

	TEST_FUNCTION(test_angle_invert_map_range_inside_bounds);

	TEST_FUNCTION(test_angle_invert_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_angle_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value);
	TEST_FUNCTION(test_angle_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value);
	TEST_FUNCTION(test_angle_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value);
	TEST_FUNCTION(test_angle_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value);



	TEST_FUNCTION(test_real_map_range_clamp_if_below_lower_bound);
	TEST_FUNCTION(test_real_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off);
	TEST_FUNCTION(test_real_map_range_clamp_if_above_upper_bound);
	TEST_FUNCTION(test_real_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off);

	TEST_FUNCTION(test_real_map_range_matches_from_to_lower_bounds);
	TEST_FUNCTION(test_real_map_range_matches_from_to_upper_bounds);

	TEST_FUNCTION(test_real_map_range_inside_bounds);

	TEST_FUNCTION(test_real_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_real_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range);
	TEST_FUNCTION(test_real_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range);

	TEST_FUNCTION(test_real_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value);

	TEST_FUNCTION(test_real_invert_map_range_works_when_below_target_lower_bound);
	TEST_FUNCTION(test_real_invert_map_range_works_when_below_target_lower_bound_even_if_clamped);
	TEST_FUNCTION(test_real_invert_map_range_works_when_above_target_upper_bound);
	TEST_FUNCTION(test_real_invert_map_range_works_when_above_target_upper_bound_even_if_clamped);

	TEST_FUNCTION(test_real_invert_map_range_matches_lower_bounds);
	TEST_FUNCTION(test_real_invert_map_range_matches_upper_bounds);

	TEST_FUNCTION(test_real_invert_map_range_inside_bounds);

	TEST_FUNCTION(test_real_invert_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_real_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value);
	TEST_FUNCTION(test_real_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value);
	TEST_FUNCTION(test_real_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value);
	TEST_FUNCTION(test_real_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value);



	TEST_FUNCTION(test_time_map_range_clamp_if_below_lower_bound);
	TEST_FUNCTION(test_time_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off);
	TEST_FUNCTION(test_time_map_range_clamp_if_above_upper_bound);
	TEST_FUNCTION(test_time_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off);

	TEST_FUNCTION(test_time_map_range_matches_from_to_lower_bounds);
	TEST_FUNCTION(test_time_map_range_matches_from_to_upper_bounds);

	TEST_FUNCTION(test_time_map_range_inside_bounds);

	TEST_FUNCTION(test_time_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_time_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range);
	TEST_FUNCTION(test_time_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range);

	TEST_FUNCTION(test_time_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value);

	TEST_FUNCTION(test_time_invert_map_range_works_when_below_target_lower_bound);
	TEST_FUNCTION(test_time_invert_map_range_works_when_below_target_lower_bound_even_if_clamped);
	TEST_FUNCTION(test_time_invert_map_range_works_when_above_target_upper_bound);
	TEST_FUNCTION(test_time_invert_map_range_works_when_above_target_upper_bound_even_if_clamped);

	TEST_FUNCTION(test_time_invert_map_range_matches_lower_bounds);
	TEST_FUNCTION(test_time_invert_map_range_matches_upper_bounds);

	TEST_FUNCTION(test_time_invert_map_range_inside_bounds);

	TEST_FUNCTION(test_time_invert_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_time_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value);
	TEST_FUNCTION(test_time_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value);
	TEST_FUNCTION(test_time_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value);
	TEST_FUNCTION(test_time_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value);



	TEST_FUNCTION(test_vector_map_range_clamp_if_below_lower_bound);
	TEST_FUNCTION(test_vector_map_range_does_not_clamp_to_lower_bound_if_clamp_is_off);
	TEST_FUNCTION(test_vector_map_range_clamp_if_above_upper_bound);
	TEST_FUNCTION(test_vector_map_range_does_not_clamp_to_upper_bound_if_clamp_is_off);

	TEST_FUNCTION(test_vector_map_range_matches_from_to_lower_bounds);
	TEST_FUNCTION(test_vector_map_range_matches_from_to_upper_bounds);

	TEST_FUNCTION(test_vector_map_range_inside_bounds);

	TEST_FUNCTION(test_vector_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_vector_map_range_to_target_lower_bound_if_source_range_is_null_and_value_is_lower_than_corrupted_source_range);
	TEST_FUNCTION(test_vector_map_range_to_target_upper_bound_if_source_range_is_null_and_value_is_equals_to_or_greater_than_corrupted_source_range);

	TEST_FUNCTION(test_vector_map_range_to_single_value_if_target_range_is_null_no_matter_the_source_value);

	TEST_FUNCTION(test_vector_invert_map_range_works_when_below_target_lower_bound);
	TEST_FUNCTION(test_vector_invert_map_range_works_when_below_target_lower_bound_even_if_clamped);
	TEST_FUNCTION(test_vector_invert_map_range_works_when_above_target_upper_bound);
	TEST_FUNCTION(test_vector_invert_map_range_works_when_above_target_upper_bound_even_if_clamped);

	TEST_FUNCTION(test_vector_invert_map_range_matches_lower_bounds);
	TEST_FUNCTION(test_vector_invert_map_range_matches_upper_bounds);

	TEST_FUNCTION(test_vector_invert_map_range_inside_bounds);

	TEST_FUNCTION(test_vector_invert_map_range_does_not_crash_if_source_range_is_null);
	TEST_FUNCTION(test_vector_invert_map_range_the_single_value_if_source_range_is_null_no_matter_the_target_value);
	TEST_FUNCTION(test_vector_invert_map_range_return_source_lower_bound_if_target_range_is_null_if_target_value_is_lower_than_this_single_value);
	TEST_FUNCTION(test_vector_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_greater_than_this_single_value);
	TEST_FUNCTION(test_vector_invert_map_range_return_source_upper_bound_if_target_range_is_null_if_target_value_is_equals_to_this_single_value);



	TEST_SUITE_END()

	return tst_exit_status;
}
