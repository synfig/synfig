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
	cout<<"[origin]="<<bone.origin_<<endl;	                              \
	cout<<"[angle]="<<bone.angle_<<endl;		                          \
	cout<<"[scalelx]="<<bone.scalelx_<<endl; \
	cout<<"[scalex]="<<bone.scalex_<<endl;	  \
	cout<<"[length]="<<bone.length_<<endl;         \
	cout<<"[width]="<<bone.width_<<endl; \
	cout<<"[tipwidth]="<<bone.tipwidth_<<endl; \
	cout<<"[depth]="<<bone.depth_<<endl; \
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

	struct Shape {
		Vector p0;
		Real r0;
		Vector p1;
		Real r1;
		inline Shape():
			r0(0.0), r1(0.0) { }
		inline Shape(const Vector &p0, Real r0, const Vector &p1, Real r1):
			p0(p0), r0(r0), p1(p1), r1(r1) { }
	};

	// typedef etl::loose_handle<Bone> LooseHandle;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:
	//!This is the name of the bone
	String name_;
	//!This is the current origin of the bone relative to parent
	Point origin_;
	//!This is the current angle if the bone relative to parent.
	Angle angle_;
	//!This is the current local x scale of the bone.
	Real scalelx_;
	//!This is the current recursive x scale of the bone.
	Real scalex_;
	//!This is the current recursive y scale of the bone.
	Real length_;
	//!This is the width of bone at its origin
	Real width_;
	//!This is the width of bone at its tip
	Real tipwidth_;
	//!This is the z-depth of bone
	Real depth_;
	//!The parent bone.
	const ValueNode_Bone* parent_;

	Matrix animated_matrix_;

public:
	//!Default constructor
	Bone();
	//!Constructor by origin and tip
	Bone(const Point &origin, const Point &tip);
	//!Constructor by origin, length and parent (default no parent)
	Bone(const String &name, const Point &origin, const Angle &angle, const Real &length, ValueNode_Bone* p=0);
	//!Wrappers for name_
	const String& get_name()const {return name_;}
	void set_name(const String &x) {name_=x;}

	//!Wrappers for origin_
	const Point& get_origin()const {return origin_;}
	void set_origin(const Point &x) {origin_=x;}

	//!Wrappers for angle_
	const Angle& get_angle()const {return angle_;}
	void set_angle(const Angle &x) {angle_=x;}

	//!Wrapper for scalelx
	const Real& get_scalelx()const {return scalelx_;}
	void set_scalelx(const Real &x) {scalelx_=x;}

	//!Wrapper for scalex
	const Real& get_scalex()const {return scalex_;}
	void set_scalex(const Real &x) {scalex_=x;}

	//!Wrapper for length. Notice that a length of 0 is not allowed.
	const Real& get_length()const {return length_;}
	void set_length(const Real &x) {length_=x<0.00001?0.00001:x;}

	//!Wrapper for width
	const Real& get_width()const {return width_;}
	void set_width(const Real &x) {width_=x;}
	
	//!Wrapper for tipwidth
	const Real& get_tipwidth()const {return tipwidth_;}
	void set_tipwidth(const Real &x) {tipwidth_=x;}

	//!Wrapper for depth
	const Real& get_depth()const {return depth_;}
	void set_depth(const Real &x) {depth_=x;}

	//!This gets the calculated tip of the bone based on
	//!tip=origin+[length,0]*Rotate(alpha)*Scalex(scalex*scalelx)
	Point get_tip();

	//!Wrapper for parent bone
	// const Bone &get_parent() {return *parent_;}
	const ValueNode_Bone* get_parent()const;
	void set_parent(const ValueNode_Bone* parent);

	void add_bone_to_map();
	Bone* find_bone_in_map(int uid);

	//!Animated Transformation matrix.
	//!This matrix applied to a setup point in local
	//!coordinates (the one obtained form the Setup
	//!Transformation matrix) would obtain the
	//!animated position of the point due the current
	//!bone influence
	Matrix get_animated_matrix() const { return animated_matrix_; }
	void set_animated_matrix(Matrix x) { animated_matrix_ = x; }

	Vector get_local_scale() const { return Vector(scalelx_, 1.0); }

	//!Get the string of the Bone
	//!@return String type. A string representation of the bone
	//!components.
	synfig::String get_string()const;

	bool is_root()const;

	Shape get_shape() const;

	static Real distance_to_shape_center_percent(const Shape &shape, const Vector &x);
	static Real influence_function(Real distance_percent);

	static Real influence_percent(const Shape &shape, const Vector &x)
		{ return influence_function(distance_to_shape_center_percent(shape, x)); }

	Real distance_to_shape_center_percent(const Vector &x)const
		{ return distance_to_shape_center_percent(get_shape(), x); }

	Real influence_percent(const Vector &x)const
		{ return influence_percent(get_shape(), x); }

	// checks if point belongs to the range of influence of current bone
	bool have_influence_on(const Vector &x)const
		{ return distance_to_shape_center_percent(x) > 0.0; }
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
 * 		-origin,
 * 		-angle,
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
 * list of bones by its UniqueID value. For example to calculate the setup matrix it should
 * reconstruct the bone hierarchy from the current bone to the root parent. Due to that now,
 * it is only stored the UniqueID of the parent (and not a pointer), it is the skeletons veluenode
 * who have to perform all the job: find all the parents and multiply in the correct order (depth) its
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
