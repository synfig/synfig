/* === S Y N F I G ========================================================= */
/*!\file node.cpp
** \brief Test Node class
**
** \legal
** Copyright (c) 2022 Synfig contributors
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
** \endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <gtest/gtest.h>

#include <synfig/reference_counter.h>

/* === U S I N G =========================================================== */
/* === M A C R O S ========================================================= */
/* === T Y P E D E F S ===================================================== */
/* === C L A S S E S & S T R U C T S ======================================= */

struct ObjectNotInit
{
	synfig::ReferenceCounter ref_counter;
	ObjectNotInit()
		: ref_counter(false)
	{}
};

struct Object
{
	synfig::ReferenceCounter ref_counter;
};

/* === G L O B A L S ======================================================= */
/* === P R O C E D U R E S ================================================= */

TEST(ReferenceCounterTest, test_count_zero_if_not_initialized)
{
	ObjectNotInit obj;
	EXPECT_EQ(0, obj.ref_counter.count());
	EXPECT_FALSE(obj.ref_counter.unique())
}

TEST(ReferenceCounterTest, test_count_one_if_initialized_on_construct)
{
	Object obj;
	EXPECT_EQ(1, obj.ref_counter.count());
}

TEST(ReferenceCounterTest, test_unique_if_just_initialized_on_construct)
{
	Object obj;
	EXPECT_TRUE(obj.ref_counter.unique())
}

TEST(ReferenceCounterTest, test_count_zero_if_detached_after_construct)
{
	Object obj;
	obj.ref_counter.detach();
	EXPECT_EQ(0, obj.ref_counter.count())
}

TEST(ReferenceCounterTest, test_count_one_if_just_initialized)
{
	ObjectNotInit obj;
	obj.ref_counter.reset();
	EXPECT_EQ(1, obj.ref_counter.count());
}

TEST(ReferenceCounterTest, test_unique_if_just_initialized)
{
	ObjectNotInit obj;
	obj.ref_counter.reset();
	EXPECT_TRUE(obj.ref_counter.unique())
}

TEST(ReferenceCounterTest, test_count_zero_if_detached_after_reset)
{
	ObjectNotInit obj;
	obj.ref_counter.reset();
	obj.ref_counter.detach();
	EXPECT_EQ(0, obj.ref_counter.count())
}

TEST(ReferenceCounterTest, test_double_reset_does_not_increase_count)
{
	Object obj;
	obj.ref_counter.reset();
	obj.ref_counter.reset();
	EXPECT_EQ(1, obj.ref_counter.count())
}

TEST(ReferenceCounterTest, test_copy_constructor_increases_count)
{
	Object obj;
	Object obj_copy(obj);
	EXPECT_EQ(2, obj.ref_counter.count())
	EXPECT_EQ(2, obj_copy.ref_counter.count())
}

TEST(ReferenceCounterTest, test_copy_assignment_increases_count)
{
	Object obj;
	Object obj_copy = obj;
	EXPECT_EQ(2, obj.ref_counter.count())
	EXPECT_EQ(2, obj_copy.ref_counter.count())
}

TEST(ReferenceCounterTest, test_move_constructor_does_not_increase_count)
{
	Object obj;
	Object obj_move(std::move(obj));
	EXPECT_EQ(0, obj.ref_counter.count())
	EXPECT_EQ(1, obj_move.ref_counter.count())
}

TEST(ReferenceCounterTest, test_move_assignment_does_not_increase_count)
{
	Object obj;
	Object obj_move = std::move(obj);
	EXPECT_EQ(0, obj.ref_counter.count())
	EXPECT_EQ(1, obj_move.ref_counter.count())
}

TEST(ReferenceCounterTest, test_copy_assignment_detaches_previous_count)
{
	Object obj;
	EXPECT_TRUE(obj.ref_counter.unique());
	Object obj2;
	EXPECT_TRUE(obj2.ref_counter.unique());

	Object obj2_copy(obj2);
	EXPECT_EQ(1, obj.ref_counter.count());
	EXPECT_EQ(2, obj2.ref_counter.count());
	EXPECT_EQ(2, obj2_copy.ref_counter.count());

	obj2_copy = obj;
	EXPECT_EQ(2, obj.ref_counter.count());
	EXPECT_EQ(1, obj2.ref_counter.count());
	EXPECT_EQ(2, obj2_copy.ref_counter.count());
}

TEST(ReferenceCounterTest, test_move_assignment_detaches_previous_count)
{
	Object obj;
	EXPECT_TRUE(obj.ref_counter.unique());
	Object obj2;
	EXPECT_TRUE(obj2.ref_counter.unique());

	Object obj2_copy(obj2);
	EXPECT_TRUE(obj.ref_counter.unique());
	EXPECT_EQ(2, obj2.ref_counter.count());
	EXPECT_EQ(2, obj2_copy.ref_counter.count());

	obj2_copy = std::move(obj);
	EXPECT_EQ(0, obj.ref_counter.count());
	EXPECT_EQ(1, obj2.ref_counter.count());
	EXPECT_EQ(1, obj2_copy.ref_counter.count());
}

TEST(ReferenceCounterTest, test_does_not_change_count_if_copy_assigned_to_itself)
{
	Object obj;
	Object obj2(obj);
	EXPECT_EQ(2, obj.ref_counter.count());
	EXPECT_EQ(2, obj2.ref_counter.count());

	obj2 = obj;
	EXPECT_EQ(2, obj.ref_counter.count());
	EXPECT_EQ(2, obj2.ref_counter.count());

	obj = obj2;
	EXPECT_EQ(2, obj.ref_counter.count());
	EXPECT_EQ(2, obj2.ref_counter.count());

	obj = obj;
	EXPECT_EQ(2, obj.ref_counter.count());
	EXPECT_EQ(2, obj2.ref_counter.count());

	obj2 = obj2;
	EXPECT_EQ(2, obj.ref_counter.count());
	EXPECT_EQ(2, obj2.ref_counter.count());
}

TEST(ReferenceCounterTest, test_does_not_change_count_if_move_assigned_to_itself)
{
	Object obj;
	obj = std::move(obj);
	EXPECT_EQ(1, obj.ref_counter.count())
}

TEST(ReferenceCounterTest, test_copy_an_empty_reference_makes_it_empty_too)
{
	Object not_init_obj;
	not_init_obj.ref_counter.detach();

	EXPECT_EQ(0, not_init_obj.ref_counter.count());

	Object init_obj;

	init_obj = not_init_obj;
	EXPECT_EQ(0, init_obj.ref_counter.count());

	Object init_obj2;
	Object init_obj3(init_obj2);

	EXPECT_EQ(2, init_obj2.ref_counter.count());

	init_obj2 = not_init_obj;
	EXPECT_EQ(0, init_obj2.ref_counter.count());
}

/* === M E T H O D S ======================================================= */
