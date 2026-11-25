/* === S Y N F I G ========================================================= */
/*!	\file bone.cpp
**	\brief Bone & Smart Bone Driver Test File
**
**	\legal
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2026 Synfig Contributors
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <cmath>
#include <synfig/bone.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_bonedriver.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/vector.h>
#include <synfig/blinepoint.h>
#include <synfig/canvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

#define TEST_ASSERT(cond, msg) \
	do { \
		if (!(cond)) { \
			std::cerr << "FAILED: " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
			return 1; \
		} \
	} while (0)

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

int test_bone_basic()
{
	Bone bone("test_bone", Point(1, 2), Angle::deg(45), 5.0);
	TEST_ASSERT(bone.get_name() == "test_bone", "bone name check");
	TEST_ASSERT(std::abs(bone.get_origin()[0] - 1.0) < 1e-6, "bone origin x check");
	TEST_ASSERT(std::abs(bone.get_origin()[1] - 2.0) < 1e-6, "bone origin y check");
	TEST_ASSERT(std::abs(Angle::deg(bone.get_angle()).get() - 45.0) < 1e-5, "bone angle check");
	TEST_ASSERT(std::abs(bone.get_length() - 5.0) < 1e-6, "bone length check");
	return 0;
}

int test_smart_bone_driver_real()
{
	Canvas::Handle canvas = Canvas::create();
	Bone bone("arm_bone", Point(0, 0), Angle::deg(0), 10.0);
	ValueNode_Bone::Handle bone_node = ValueNode_Bone::create(bone, canvas);

	ValueNode_BoneDriver::Handle driver = ValueNode_BoneDriver::create(Real(10.0), canvas);
	driver->set_link("bone", bone_node);
	driver->set_link("val_min", ValueNode_Const::create(Real(10.0)));
	driver->set_link("val_max", ValueNode_Const::create(Real(50.0)));
	driver->set_link("min_angle", ValueNode_Const::create(Angle::deg(0)));
	driver->set_link("max_angle", ValueNode_Const::create(Angle::deg(90)));
	driver->set_link("clamp", ValueNode_Const::create(true));

	// At 0 deg -> 10.0
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(0)));
	Real val0 = (*driver)(0).get(Real());
	TEST_ASSERT(std::abs(val0 - 10.0) < 1e-5, "Driver at 0 deg should be 10.0");

	// At 45 deg -> 30.0
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(45)));
	Real val45 = (*driver)(0).get(Real());
	TEST_ASSERT(std::abs(val45 - 30.0) < 1e-5, "Driver at 45 deg should be 30.0");

	// At 90 deg -> 50.0
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(90)));
	Real val90 = (*driver)(0).get(Real());
	TEST_ASSERT(std::abs(val90 - 50.0) < 1e-5, "Driver at 90 deg should be 50.0");

	// At 180 deg with clamp=true -> 50.0
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(180)));
	Real val180 = (*driver)(0).get(Real());
	TEST_ASSERT(std::abs(val180 - 50.0) < 1e-5, "Driver clamped at 180 deg should be 50.0");

	// At -45 deg with clamp=true -> 10.0
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(-45)));
	Real val_neg = (*driver)(0).get(Real());
	TEST_ASSERT(std::abs(val_neg - 10.0) < 1e-5, "Driver clamped at -45 deg should be 10.0");

	return 0;
}

int test_smart_bone_driver_vector()
{
	Canvas::Handle canvas = Canvas::create();
	Bone bone("elbow_bone", Point(0, 0), Angle::deg(0), 10.0);
	ValueNode_Bone::Handle bone_node = ValueNode_Bone::create(bone, canvas);

	ValueNode_BoneDriver::Handle driver = ValueNode_BoneDriver::create(Vector(0, 0), canvas);
	driver->set_link("bone", bone_node);
	driver->set_link("val_min", ValueNode_Const::create(Vector(10, 20)));
	driver->set_link("val_max", ValueNode_Const::create(Vector(50, 100)));
	driver->set_link("min_angle", ValueNode_Const::create(Angle::deg(0)));
	driver->set_link("max_angle", ValueNode_Const::create(Angle::deg(90)));
	driver->set_link("clamp", ValueNode_Const::create(true));

	// At 45 deg -> Vector(30, 60)
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(45)));
	Vector v45 = (*driver)(0).get(Vector());
	TEST_ASSERT(std::abs(v45[0] - 30.0) < 1e-5, "Vector X at 45 deg should be 30.0");
	TEST_ASSERT(std::abs(v45[1] - 60.0) < 1e-5, "Vector Y at 45 deg should be 60.0");

	return 0;
}

int test_smart_bone_driver_bline_point()
{
	Canvas::Handle canvas = Canvas::create();
	Bone bone("joint_bone", Point(0, 0), Angle::deg(0), 10.0);
	ValueNode_Bone::Handle bone_node = ValueNode_Bone::create(bone, canvas);

	BLinePoint bp_relaxed;
	bp_relaxed.set_vertex(Point(0, 0));
	bp_relaxed.set_width(1.0);

	BLinePoint bp_bent;
	bp_bent.set_vertex(Point(5, 10));
	bp_bent.set_width(2.5);

	ValueNode_BoneDriver::Handle driver = ValueNode_BoneDriver::create(bp_relaxed, canvas);
	driver->set_link("bone", bone_node);
	driver->set_link("val_min", ValueNode_Const::create(bp_relaxed));
	driver->set_link("val_max", ValueNode_Const::create(bp_bent));
	driver->set_link("min_angle", ValueNode_Const::create(Angle::deg(0)));
	driver->set_link("max_angle", ValueNode_Const::create(Angle::deg(100)));
	driver->set_link("clamp", ValueNode_Const::create(true));

	// At 50 deg (halfway)
	bone_node->set_link("angle", ValueNode_Const::create(Angle::deg(50)));
	BLinePoint bp50 = (*driver)(0).get(BLinePoint());
	TEST_ASSERT(std::abs(bp50.get_vertex()[0] - 2.5) < 1e-5, "BLinePoint vertex X should be 2.5");
	TEST_ASSERT(std::abs(bp50.get_vertex()[1] - 5.0) < 1e-5, "BLinePoint vertex Y should be 5.0");
	TEST_ASSERT(std::abs(bp50.get_width() - 1.75) < 1e-5, "BLinePoint width should be 1.75");

	return 0;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	synfig::Type::subsys_init();

	int failures = 0;

	std::cout << "Running Bone & Smart Bone Driver tests..." << std::endl;

	try {
		std::cout << "Running test_bone_basic..." << std::endl;
		failures += test_bone_basic();
		std::cout << "test_bone_basic done. failures = " << failures << std::endl;

		std::cout << "Running test_smart_bone_driver_real..." << std::endl;
		failures += test_smart_bone_driver_real();
		std::cout << "test_smart_bone_driver_real done. failures = " << failures << std::endl;

		std::cout << "Running test_smart_bone_driver_vector..." << std::endl;
		failures += test_smart_bone_driver_vector();
		std::cout << "test_smart_bone_driver_vector done. failures = " << failures << std::endl;

		std::cout << "Running test_smart_bone_driver_bline_point..." << std::endl;
		failures += test_smart_bone_driver_bline_point();
		std::cout << "test_smart_bone_driver_bline_point done. failures = " << failures << std::endl;
	} catch (const std::exception &e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		failures++;
	} catch (...) {
		std::cerr << "Unknown exception caught!" << std::endl;
		failures++;
	}

	if (failures == 0) {
		std::cout << "All Bone & Smart Bone Driver tests PASSED!" << std::endl;
	} else {
		std::cerr << failures << " tests FAILED!" << std::endl;
	}

	return failures;
}
