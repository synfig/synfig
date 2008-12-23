/* === S Y N F I G ========================================================= */
/*!	\file bone.h
**	\brief Bone Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Carlos López & Chirs Moore
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_BONE_H
#define __SYNFIG_BONE_H

/* === H E A D E R S ======================================================= */
#include <iostream>
#include "matrix.h"
#include "uniqueid.h"
#include "string.h"
#include "guid.h"
#include <vector>
#include <ETL/handle>

/* === M A C R O S ========================================================= */
// how many hex digits of the guid string to show in debug messages
#define GUID_PREFIX_LEN 6

#define COUT_BONE(bone)													  \
	cout<<"[name]="<<bone.name_<<endl;									  \
	cout<<"[origin]="<<bone.origin_<<"[origin0]="<<bone.origin0_<<endl;	  \
	cout<<"[angle]="<<bone.angle_<<"[angle0]="<<bone.angle0_<<endl;		  \
	cout<<"[scalelx]="<<bone.scalelx_<<"[scalely]="<<bone.scalely_<<endl; \
	cout<<"[scalex]="<<bone.scalex_<<"[scaley]="<<bone.scaley_<<endl;	  \
	cout<<"[length]="<<bone.length_<<"[strength]="<<bone.strength_<<endl; \
	cout<<"[parent]="<<bone.parent_<<endl
/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig {

class ValueNode_Bone;

class Bone: public UniqueID
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:
	typedef etl::handle<Bone> Handle;
	// typedef etl::loose_handle<Bone> LooseHandle;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:
	//!This is the name of the bone
	String name_;
	//!This is the current origin of the bone relative to parent
	Point origin_;
	//!This is the origin of the bone at the setup time
	Point origin0_;
	//!This is the current angle if the bone relative to parent.
	Angle angle_;
	//!This is the angle of the bone at the setup time
	Angle angle0_;
	//!This is the current local x scale of the bone.
	Real scalelx_;
	//!This is the current local y scale of the bone.
	Real scalely_;
	//!This is the current recursive x scale of the bone.
	Real scalex_;
	//!This is the current recursive y scale of the bone.
	Real scaley_;
	//!This is the length at setup time
	Real length_;
	//!This is the strength at setup time
	Real strength_;
	//!Whether the bone is currently showing its setup position
	bool setup_;
	//!The parent bone.
	const ValueNode_Bone* parent_;

	Matrix setup_matrix_;
	Matrix animated_matrix_;

public:
	//!Default constructor
	Bone();
	//!Constructor by origin and tip
	Bone(const Point &origin, const Point &tip);
	//!Construtor by origin, legth and parent (default no parent)
	Bone(const String &name, const Point &origin, const Angle &angle, const Real &length, const Real &strength, ValueNode_Bone* p=0);
	//!Wrappers for name_
	const String& get_name()const {return name_;}
	void set_name(const String &x) {name_=x;}

	//!Wrappers for origin_ & origin0_
	const Point& get_origin()const {return origin_;}
	void set_origin(const Point &x) {origin_=x;}
	const Point& get_origin0()const {return origin0_;}
	void set_origin0(const Point &x) {origin0_=x;}

	//!Wrappers for angle_ & angle0_
	const Angle& get_angle()const {return angle_;}
	void set_angle(const Angle &x) {angle_=x;}
	const Angle& get_angle0()const {return angle0_;}
	void set_angle0(const Angle &x) {angle0_=x;}

	//!Wrapper for scalelx
	const Real& get_scalelx()const {return scalelx_;}
	void set_scalelx(const Real &x) {scalelx_=x;}

	//!Wrapper for scalely
	const Real& get_scalely()const {return scalely_;}
	void set_scalely(const Real &y) {scalely_=y;}

	//!Wrapper for scalex
	const Real& get_scalex()const {return scalex_;}
	void set_scalex(const Real &x) {scalex_=x;}

	//!Wrapper for scaley
	const Real& get_scaley()const {return scaley_;}
	void set_scaley(const Real &y) {scaley_=y;}

	//!Wrapper for length. Notice that a length of 0 is not allowed.
	const Real& get_length()const {return length_;}
	void set_length(const Real &x) {length_=x<0.00001?0.00001:x;}

	//!Wrapper for strength
	const Real& get_strength()const {return strength_;}
	void set_strength(const Real &x) {strength_=x;}

	//!Wrapper for setup
	const bool& get_setup()const {return setup_;}
	void set_setup(const bool &x) {setup_=x;}

	//!This gets the calculated tip of the bone based on
	//!tip=origin+[length,0]*Rotate(alpha)*Scalex(scalex*scalelx)
	Point get_tip();

	//!Wrapper for parent bone
	// const Bone &get_parent() {return *parent_;}
	const ValueNode_Bone* get_parent()const;
	void set_parent(const ValueNode_Bone* parent);

	void add_bone_to_map();
	Bone* find_bone_in_map(int uid);

	//!Setup Transformation matrix.
	//!This matrix applied to a setup point in global
	//!coordinates calculates the local coordinates of
	//!the point relative to the current bone.
	Matrix get_setup_matrix() const { return setup_matrix_; }
	void set_setup_matrix(Matrix x) { setup_matrix_ = x; }

	//!Animated Transformation matrix.
	//!This matrix applied to a setup point in local
	//!coordinates (the one obtained form the Setup
	//!Transformation matrix) would obtain the
	//!animated position of the point due the current
	//!bone influence
	Matrix get_animated_matrix() const { return animated_matrix_; }
	void set_animated_matrix(Matrix x) { animated_matrix_ = x; }

	Vector get_local_scale() const { return Vector(scalelx_, scalely_); }

	//!Get the string of the Bone
	//!@return String type. A string representation of the bone
	//!components.
	synfig::String get_string()const;

	bool is_root();

}; // END of class Bone

}; // END of namespace synfig
/* === E N D =============================================================== */

#endif
/*
 * Alternative to Bone *parent_
 * ======================================================================
 * I think that we can leave the bone as a simple information holder
 * and only give it the responsibility of:
 * Set and get:
 * 		-origin, origin0,
 * 		-angle, angle0,
 * 		-scalelx,
 * 		-scalely,
 * 		-scalex,
 * 		-scaley,
 * 		-length
 * 		-strength,
 * 		-ParentID: this is new: This is the UniqueID value of the parent bone.
 * 		Initially it is set to a non valid number (I think that -1 is fine)
 * 		so it means that it is a root bone. Later an external object can set it
 * 		to a valid UniqueID to mean that that's the parent ID.
 * 		parent_tree is not needed.
 * 		-Skeletons Pointer (see below)
 * Also the bone should:
 * 		-get_setup_matrix
 * 		-get_animated_matrix
 * 		-get_tip
 *
 * Then it comes the concept of ValueNode_Skeletons. The Skeletons (notice that
 * it is plural because there can be more than one root bone) is like the ValueNode_Bline,
 * a linkable value.
 * It is like a normal list of bones (like bline is a normal list of blinepoints).
 * This list of bones has just that, bones. So the skeleton is not an expandable tree with
 * a potential loop problem; it is just a list of objects.
 *
 * The ValueNode_Skeletons is responsible for:
 * 1) Calculate the complete setup matrix of a bone based on the hierarchy
 * 2) Calculate the complete animated matrix of a bone based on the hierarchy
 * 3) (Re)Parent a bone. Or (Un)Parent it
 * 4) Remove the bone from the list. It would set the parent UniqueID=-1 and the Skeletons
 *    pointer to be 0.
 * 5) Add a new bone to the list. The bone constructor would receive a Skeleton pointer and
 *    eventually a parent UniqueID besides the rest of information to fill the date (origin, etc.).
 *
 * It would look like that:
 *
 *   ValueNode_Skeletons
 *       ValueNode_Bone Bone1
 *       ValueNode_Bone Bone2
 *       ...
 *       ValueNode_Bone BoneN
 *
 * To perform the tasks 1), 2), 3) or 4) the ValueNode_Skeletons should perform a seek into the
 * list of bones by its UniqueID value. For example to calcualte the setup matrix it should
 * reconstruct the bone hierarchy from the current bone to the root parent. Due to that now,
 * it is only stored the UniqueID of the parent (and not a pointer), it is the skeletons veluenode
 * who have to perform all the job: find all the parents and multiply in the correct order its
 * matrixes. The same happen for the animated matrix.
 * For reparent it is the same. It is just a modification of the parent UniqueID.
 * Remove a bone from the list would imply remove all its children from the list. A warning should be triggered.
 * A bone that has a null pointer to Skeletons means that it is orphaned completely. Its parent UniqueID
 * must be -1 in that case. Anyway the bone like that can be used again in other skeleton. Just need to
 * insert it in the Skeletons list by modifying the Skeletons pointer and filling the proper parent UniqueID.
 * The Skeletons pointer is not an animatable ValueNode. It can be a Handle if you like. The parent
 * UniqueID can be animatable.
 * In this way every computation is slower but would be easier to define, visible to the user
 * and more consistent with the ValueNode concept.
 *
 * This variation of concept doesn't imply anything new in the ValueNode_VertexBone.
 * So the ValueNode_VertexBone should look like:
 *
 *   ValueNode_VertexBone
 *       Vertex Free
 *       Vertex Setup
 *       ValueNode_DynamicList Bone_weight_pairs
 *           BoneWeightPair
 *               ValueNode_Bone Bone
 *               Real Weight
 *
 * As well as the Bone having a pointer to the Skeletons it is possible for the VertexBone_ValueNode
 * to calculate the weighted matrixes as stated in the wiki. It just has to retrieve the
 * ValueNode_Skeleton and ask it to perform the known tasks. Later the ValueNode_VertexBone
 * would do the weight calculation.
 *
 * How does it look?
 */
