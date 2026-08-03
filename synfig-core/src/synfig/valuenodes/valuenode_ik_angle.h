/* === S Y N F I G ========================================================= */
/*! \file valuenode_ik_angle.h
**  \brief Header file for implementation of the "Ik angle" valuenode conversion.
**
**  \legal
**  Copyright (c) 2025 ZAINAL ABID
**  Copyright (c) Synfig Contributors
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_IK_ANGLE_H
#define __SYNFIG_VALUENODE_IK_ANGLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_IK : public LinkableValueNode
{

    enum Joint {
        TWOBONE = 1,THREEBONE = 2,
    };

    enum RigType {
        ANIMAL= 1,HUMAN = 2,
    };

    enum TargetBone {
        BONE1 = 1,BONE2 = 2, BONE3 = 3,
    };
    

    ValueNode::RHandle link_pole_;
    ValueNode::RHandle link_target_;
    ValueNode::RHandle length_bone1_;
    ValueNode::RHandle length_bone2_;
    ValueNode::RHandle length_bone3_;
    ValueNode::RHandle flip_;
    ValueNode::RHandle joint_bone_;
    ValueNode::RHandle t_bone_;
    ValueNode::RHandle f_bone_;
    ValueNode::RHandle weight_;

    ValueNode_IK(const ValueBase &value);

public:
    typedef etl::handle<ValueNode_IK> Handle;
    typedef etl::handle<const ValueNode_IK> ConstHandle;

    static ValueNode_IK* create(const ValueBase& x, etl::loose_handle<Canvas> canvas=nullptr);
    virtual ~ValueNode_IK();

    virtual String get_name() const override;
    virtual String get_local_name() const override;
    static bool check_type(Type &type);

    virtual ValueBase operator()(Time t) const override;

protected:
    LinkableValueNode* create_new() const override;

    virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
    virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

    virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_IK

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif //__SYNFIG_VALUENODE_IK_ANGLE_H
