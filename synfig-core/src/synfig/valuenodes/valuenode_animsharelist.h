/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animsharelist.h
**	\brief Header file for implementation of the "Anim Share List" valuenode
**	conversion.
**
**	This file is part of Synfig.
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef valuenode_animsharelist_h_INCLUDED
#define valuenode_animsharelist_h_INCLUDED

/* === H E A D E R S ======================================================= */

#include "valuenode_dynamiclist.h"

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class ValueNode_AnimShareList
**	\brief This class implements a list of AnimShare items
*/
class ValueNode_AnimShareList : public ValueNode_DynamicList {
  ValueNode_AnimShareList();

public:
  typedef etl::handle<ValueNode_AnimShareList> Handle;
  typedef etl::handle<const ValueNode_AnimShareList> ConstHandle;
  typedef etl::handle<const ValueNode_AnimShareList> LooseHandle;

  static ValueNode_AnimShareList *
  create(const ValueBase &x = type_list,
         etl::loose_handle<Canvas> canvas = nullptr);
  virtual ~ValueNode_AnimShareList();

  virtual String get_name() const override;
  virtual String get_local_name() const override;
  static bool check_type(Type &type);
  void clear();
  virtual String link_local_name(int i) const override;

  virtual ValueBase operator()(Time t) const override;

  virtual ListEntry create_list_entry(int index, Time time = 0,
                                      Real origin = 0.5) override;

protected:
  LinkableValueNode *create_new() const override;
}; // END of class ValueNode_AnimShareList

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif // valuenode_animsharelist_h_INCLUDED
