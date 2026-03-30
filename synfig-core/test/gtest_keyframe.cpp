/* === S Y N F I G ========================================================= */
/*!	\file keyframe.cpp
**	\brief Test Keyframe and KeyframeList classes
**
**	\legal
**	Copyright (c) 2022 Synfig contributors
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

#include <synfig/keyframe.h>

#include <gtest/gtest.h>

using namespace synfig;

TEST(KeyframeTest, adding_single_keyframe_works)
{
	KeyframeList list;
	Keyframe kf(1.0);
	list.add(kf);
	EXPECT_EQ(1, list.size())
}

TEST(KeyframeTest, adding_two_ordered_keyframes_keeps_the_order)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf1);
	list.add(kf2);
	EXPECT_EQ(2, list.size())
	EXPECT_EQ(kf1, *list.begin());
	EXPECT_EQ(kf2, *(list.end()-1));
}

TEST(KeyframeTest, adding_two_unordered_keyframes_fixes_the_order)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf2);
	list.add(kf1);
	EXPECT_EQ(2, list.size())
	EXPECT_EQ(kf1, *list.begin());
	EXPECT_EQ(kf2, *(list.end()-1));
}

TEST(KeyframeTest, adding_one_initial_keyframe_lists_it_first)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf1);
	list.add(kf2);

	Keyframe kf3(0.5);
	list.add(kf3);

	EXPECT_EQ(3, list.size())
	EXPECT_EQ(kf3, *list.begin());
}

TEST(KeyframeTest, adding_one_intermediate_keyframe_lists_it_correctly)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf1);
	list.add(kf2);

	Keyframe kf3(2.0);
	list.add(kf3);

	EXPECT_EQ(3, list.size())
	EXPECT_EQ(kf3, *(list.begin()+1));
}

TEST(KeyframeTest, erasing_first_keyframe_works)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf1);
	list.add(kf2);

	list.erase(kf1);

	EXPECT_EQ(1, list.size())
	EXPECT_EQ(kf2, *list.begin());
}

TEST(KeyframeTest, erasing_last_keyframe_works)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf1);
	list.add(kf2);

	list.erase(kf2);

	EXPECT_EQ(1, list.size())
	EXPECT_EQ(kf1, *list.begin());
}

TEST(KeyframeTest, erasing_non_existent_keyframe_does_not_throw_exception)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(3.0);
	list.add(kf1);
	list.add(kf2);

	Keyframe kf3(7.0);

	list.erase(kf3);

	EXPECT_EQ(2, list.size())
	EXPECT_EQ(kf1, *list.begin());
}

TEST(KeyframeTest, finding_keyframe_by_uid_works)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find(kf1, iter))
	EXPECT_EQ(1., iter->get_time())

	EXPECT_TRUE(list.find(kf2, iter))
	EXPECT_EQ(2., iter->get_time())

	EXPECT_TRUE(list.find(kf3, iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_keyframe_by_uid_does_not_ignore_disabled_keyframes)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	kf1.disable();
	kf2.disable();
	kf3.disable();
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find(kf1, iter))
	EXPECT_EQ(1., iter->get_time())

	EXPECT_TRUE(list.find(kf2, iter))
	EXPECT_EQ(2., iter->get_time())

	EXPECT_TRUE(list.find(kf3, iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_non_existing_keyframe_by_uid_does_not_throw_exception)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	bool found;
	Keyframe kf0(0.0);
	EXPECT_NO_THROW(found = list.find(kf0, iter))
	EXPECT_FALSE(found)

	Keyframe kf15(1.5);
	EXPECT_NO_THROW(found = list.find(kf15, iter))
	EXPECT_FALSE(found)

	Keyframe kf25(2.5);
	EXPECT_NO_THROW(found = list.find(kf25, iter))
	EXPECT_FALSE(found)

	Keyframe kf35(3.5);
	EXPECT_NO_THROW(found = list.find(kf35, iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_keyframe_by_time_works)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find(1., iter))
	EXPECT_EQ(1., iter->get_time())

	EXPECT_TRUE(list.find(2., iter))
	EXPECT_EQ(2., iter->get_time())

	EXPECT_TRUE(list.find(3., iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_keyframe_by_time_does_not_ignore_disabled_keyframes)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	kf1.disable();
	kf2.disable();
	kf3.disable();
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find(1., iter))
	EXPECT_EQ(1., iter->get_time())

	EXPECT_TRUE(list.find(2., iter))
	EXPECT_EQ(2., iter->get_time())

	EXPECT_TRUE(list.find(3., iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_non_existing_keyframe_by_time_does_not_throw_exception)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	bool found;
	Keyframe kf0(0.0);
	EXPECT_NO_THROW(found = list.find(0., iter))
	EXPECT_FALSE(found)

	Keyframe kf15(1.5);
	EXPECT_NO_THROW(found = list.find(1.5, iter))
	EXPECT_FALSE(found)

	Keyframe kf25(2.5);
	EXPECT_NO_THROW(found = list.find(2.5, iter))
	EXPECT_FALSE(found)

	Keyframe kf35(3.5);
	EXPECT_NO_THROW(found = list.find(3.5, iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_next_keyframe_works_with_empty_keyframe_list)
{
	KeyframeList list;

	KeyframeList::iterator iter;

	bool found;
	EXPECT_NO_THROW(found = list.find_next(0., iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_next_keyframe_works_for_time_before_first_keyframe)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_next(0., iter))
	EXPECT_EQ(1., iter->get_time())
}

TEST(KeyframeTest, finding_next_keyframe_for_time_equals_first_keyframe_retrieves_the_second)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_next(1., iter))
	EXPECT_EQ(2., iter->get_time())
}

TEST(KeyframeTest, finding_next_keyframe_works_for_time_before_intermediate_keyframe)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_next(1.5, iter))
	EXPECT_EQ(2., iter->get_time())
}

TEST(KeyframeTest, finding_next_keyframe_for_time_equals_intermediate_keyframe_retrieves_the_next_one)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_next(2., iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_next_keyframe_works_for_time_before_last_keyframe)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_next(2.5, iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_next_keyframe_for_time_equals_last_keyframe_retrieves_none)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	bool found;
	EXPECT_NO_THROW(found = list.find_next(3., iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_next_keyframe_for_time_after_last_keyframe_does_not_throw_exception)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	bool found;
	EXPECT_NO_THROW(found = list.find_next(3.1, iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_previous_keyframe_works_with_empty_keyframe_list)
{
	KeyframeList list;

	KeyframeList::iterator iter;

	bool found;
	EXPECT_NO_THROW(found = list.find_prev(0., iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_previous_keyframe_for_time_before_first_keyframe_does_not_throw_exception)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	bool found;
	EXPECT_NO_THROW(found = list.find_prev(0., iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_previous_keyframe_for_time_equals_first_keyframe_retrieves_none)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	bool found;
	EXPECT_NO_THROW(found = list.find_prev(1., iter))
	EXPECT_FALSE(found)
}

TEST(KeyframeTest, finding_previous_keyframe_works_for_time_after_first_keyframe)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_prev(1.5, iter))
	EXPECT_EQ(1., iter->get_time())
}

TEST(KeyframeTest, finding_previous_keyframe_for_time_equals_intermediate_keyframe_retrieves_the_previous_one)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_prev(2., iter))
	EXPECT_EQ(1., iter->get_time())
}

TEST(KeyframeTest, finding_previous_keyframe_works_for_time_after_intermediate_keyframe)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_prev(2.5, iter))
	EXPECT_EQ(2., iter->get_time())
}

TEST(KeyframeTest, finding_previous_keyframe_for_time_equals_last_keyframe_retrieves_the_previous_one)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_prev(3., iter))
	EXPECT_EQ(2., iter->get_time())
}

TEST(KeyframeTest, finding_previous_keyframe_works_for_time_after_last_keyframe)
{
	KeyframeList list;
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	list.add(kf1);
	list.add(kf2);
	list.add(kf3);

	KeyframeList::iterator iter;

	EXPECT_TRUE(list.find_prev(3.5, iter))
	EXPECT_EQ(3., iter->get_time())
}

TEST(KeyframeTest, finding_next_keyframe_skips_disabled_keyframes)
{
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	kf1.disable();

	KeyframeList::iterator iter;

	KeyframeList list;
	list.add(kf1);

	EXPECT_FALSE(list.find_next(0., iter))

	list.add(kf2);
	list.add(kf3);

	EXPECT_TRUE(list.find_next(0., iter))
	EXPECT_EQ(2., iter->get_time())

	kf2.disable();
	list.erase(kf2);
	list.add(kf2);
	EXPECT_TRUE(list.find_next(0., iter))
	EXPECT_EQ(3., iter->get_time())

	kf3.disable();
	list.erase(kf3);
	list.add(kf3);
	EXPECT_FALSE(list.find_next(0., iter))
}

TEST(KeyframeTest, finding_previous_keyframe_skips_disabled_keyframes)
{
	Keyframe kf1(1.0);
	Keyframe kf2(2.0);
	Keyframe kf3(3.0);
	kf3.disable();

	KeyframeList::iterator iter;

	KeyframeList list;
	list.add(kf3);

	EXPECT_FALSE(list.find_prev(4., iter))

	list.add(kf1);
	list.add(kf2);

	EXPECT_TRUE(list.find_prev(4., iter))
	EXPECT_EQ(2., iter->get_time())

	kf2.disable();
	list.erase(kf2);
	list.add(kf2);
	EXPECT_TRUE(list.find_prev(4., iter))
	EXPECT_EQ(1., iter->get_time())

	kf1.disable();
	list.erase(kf1);
	list.add(kf1);
	EXPECT_FALSE(list.find_prev(4., iter))
}
