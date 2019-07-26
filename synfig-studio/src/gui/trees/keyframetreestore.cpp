/* === S Y N F I G ========================================================= */
/*!	\file keyframetreestore.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "trees/keyframetreestore.h"
#include <synfig/valuenode.h>
#include "iconcontroller.h"
#include <synfig/valuenodes/valuenode_timedswap.h>
#include <gtkmm/button.h>
#include <gtkmm/treerowreference.h>
#include <synfig/canvas.h>
#include <synfig/keyframe.h>
#include <time.h>
#include <cstdlib>
#include <ETL/smart_ptr>
#include <synfigapp/action.h>
#include <synfigapp/instance.h>
#include "onemoment.h"
#include <synfig/exception.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

// KeyframeTreeStore_Class KeyframeTreeStore::keyframe_tree_store_class_;

/* === C L A S S E S & S T R U C T S ======================================= */

struct _keyframe_iterator
{
	synfig::KeyframeList::iterator iter;
	int ref_count;
	int index;
};

/*
Gtk::TreeModel::iterator keyframe_iter_2_model_iter(synfig::KeyframeList::iterator iter,int index)
{
	Gtk::TreeModel::iterator ret;

	_keyframe_iterator*& data(static_cast<_keyframe_iterator*&>(ret->gobj()->user_data));
	data=new _keyframe_iterator();
	data->ref_count=1;
	data->iter=iter;
	data->index=index;

	return ret;
}
*/

synfig::KeyframeList::iterator model_iter_2_keyframe_iter(Gtk::TreeModel::iterator iter)
{
	_keyframe_iterator* data(static_cast<_keyframe_iterator*>(iter->gobj()->user_data));
	if(!data)
		throw std::runtime_error("bad data");
	return data->iter;
}

int get_index_from_model_iter(Gtk::TreeModel::iterator iter)
{
	_keyframe_iterator* data(static_cast<_keyframe_iterator*>(iter->gobj()->user_data));
	if(!data)
		throw std::runtime_error("bad data");
	return data->index;
}


/*
#ifndef TreeRowReferenceHack
class TreeRowReferenceHack
{
	GtkTreeRowReference *gobject_;
public:
	TreeRowReferenceHack():
		gobject_(0)
	{
	}

	TreeRowReferenceHack(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path):
		gobject_ ( gtk_tree_row_reference_new(model->gobj(), const_cast<GtkTreePath*>(path.gobj())) )
	{
	}

	TreeRowReferenceHack(const TreeRowReferenceHack &x):
		gobject_ ( x.gobject_?gtk_tree_row_reference_copy(x.gobject_):0 )
	{

	}

	void swap(TreeRowReferenceHack & other)
	{
		GtkTreeRowReference *const temp = gobject_;
		gobject_ = other.gobject_;
		other.gobject_ = temp;
	}

	const TreeRowReferenceHack &
	operator=(const TreeRowReferenceHack &rhs)
	{
		TreeRowReferenceHack temp (rhs);
  		swap(temp);
		return *this;
	}

	~TreeRowReferenceHack()
	{
		if(gobject_)
			gtk_tree_row_reference_free(gobject_);
	}

	Gtk::TreeModel::Path get_path() { return Gtk::TreeModel::Path(gtk_tree_row_reference_get_path(gobject_),false); }
	GtkTreeRowReference *gobj() { return gobject_; }
};
#endif
*/

/* === P R O C E D U R E S ================================================= */

void clear_iterator(GtkTreeIter* iter)
{
	iter->stamp=0;
	iter->user_data=iter->user_data2=iter->user_data3=0;
}

/* === M E T H O D S ======================================================= */


KeyframeTreeStore::KeyframeTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_):
	Glib::ObjectBase	("KeyframeTreeStore"),
	//! \todo what is going on here?  why the need for this KeyframeTreeStore_Class at all?
	// Glib::Object		(Glib::ConstructParams(keyframe_tree_store_class_.init(), (char*) 0, (char*) 0)),
	canvas_interface_	(canvas_interface_)
{
	reset_stamp();
	//reset_path_table();

	//connect some events
	canvas_interface()->signal_keyframe_added().connect(sigc::mem_fun(*this,&studio::KeyframeTreeStore::add_keyframe));
	canvas_interface()->signal_keyframe_removed().connect(sigc::mem_fun(*this,&studio::KeyframeTreeStore::remove_keyframe));
	canvas_interface()->signal_keyframe_changed().connect(sigc::mem_fun(*this,&studio::KeyframeTreeStore::change_keyframe));
	//canvas_interface()->signal_keyframe_selected().connect(sigc::mem_fun(*this,&studio::KeyframeTreeStore::select_keyframe));
}

KeyframeTreeStore::~KeyframeTreeStore()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("KeyframeTreeStore::~KeyframeTreeStore(): Deleted");
}

Glib::RefPtr<KeyframeTreeStore>
KeyframeTreeStore::create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_)
{
	KeyframeTreeStore *store(new KeyframeTreeStore(canvas_interface_));
	Glib::RefPtr<KeyframeTreeStore> ret(store);
	assert(ret);
	return ret;
}

void
KeyframeTreeStore::reset_stamp()
{
	stamp_=time(0)+reinterpret_cast<intptr_t>(this);
}

/*
void
KeyframeTreeStore::reset_path_table()
{
	Gtk::TreeModel::Children::iterator iter;
	const Gtk::TreeModel::Children children(children());
	path_table_.clear();
	for(iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeModel::Row row(*iter);
		path_table_[(Keyframe)row[model.keyframe]]=TreeRowReferenceHack(Glib::RefPtr<KeyframeTreeStore>(this),Gtk::TreePath(row));
	}
}
*/


inline bool
KeyframeTreeStore::iterator_sane(const GtkTreeIter* iter)const
{
	if(iter && iter->stamp==stamp_)
		return true;
	g_warning("KeyframeTreeStore::iterator_sane(): Bad iterator stamp");
	return false;
}

inline bool
KeyframeTreeStore::iterator_sane(const Gtk::TreeModel::iterator& iter)const
{
	return iterator_sane(iter->gobj());
}

inline void
KeyframeTreeStore::dump_iterator(const GtkTreeIter* /*gtk_iter*/, const Glib::ustring &/*name*/)const
{
#if 0
	if(!gtk_iter)
	{
		g_warning("KeyframeTreeStore::dump_iterator: \"%s\" is NULL (Root?)",name.c_str());
		return;
	}

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->user_data));

	if(gtk_iter->stamp!=stamp_ || !iter)
	{
		g_warning("KeyframeTreeStore::dump_iterator: \"%s\" is INVALID",name.c_str());
		return;
	}

	if((unsigned)iter->index>=canvas_interface()->get_canvas()->keyframe_list().size())
		g_warning("KeyframeTreeStore::dump_iterator: \"%s\"(%p) has bad index(index:%d)",name.c_str(),gtk_iter,iter->index);

	g_warning("KeyframeTreeStore::dump_iterator: \"%s\"(%p) ref:%d, index:%d, time:%s",name.c_str(),gtk_iter,iter->ref_count,iter->index,iter->iter->get_time().get_string().c_str());
#endif
}

inline void
KeyframeTreeStore::dump_iterator(const Gtk::TreeModel::iterator& iter, const Glib::ustring &name)const
{
	dump_iterator(iter->gobj(),name);
}

int
KeyframeTreeStore::time_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs)
{
	const Model model;

	_keyframe_iterator *rhs_iter(static_cast<_keyframe_iterator*>(rhs->gobj()->user_data));
	_keyframe_iterator *lhs_iter(static_cast<_keyframe_iterator*>(lhs->gobj()->user_data));

	Time diff(rhs_iter->iter->get_time()-lhs_iter->iter->get_time());
	if(diff<0)
		return -1;
	if(diff>0)
		return 1;
	return 0;
}

int
KeyframeTreeStore::description_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs)
{
	const Model model;

	_keyframe_iterator *rhs_iter(static_cast<_keyframe_iterator*>(rhs->gobj()->user_data));
	_keyframe_iterator *lhs_iter(static_cast<_keyframe_iterator*>(lhs->gobj()->user_data));

	int comp = rhs_iter->iter->get_description().compare(lhs_iter->iter->get_description());
	if (comp > 0) return 1;
	if (comp < 0) return -1;
	return 0;
}

void
KeyframeTreeStore::set_value_impl(const Gtk::TreeModel::iterator& row, int column, const Glib::ValueBase& value)
{
	if(!iterator_sane(row))
		return;

	if(column>=get_n_columns_vfunc())
	{
		g_warning("KeyframeTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(!g_value_type_compatible(G_VALUE_TYPE(value.gobj()),get_column_type_vfunc(column)))
	{
		g_warning("KeyframeTreeStore::set_value_impl: Bad value type");
		return;
	}

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(row.gobj()->user_data));

	try
	{
		if(column==model.time_delta.index())
		{
			Glib::Value<synfig::Time> x;
			g_value_init(x.gobj(),model.time.type());
			g_value_copy(value.gobj(),x.gobj());

			Time new_delta(x.get());
			if(new_delta<=Time::zero()+Time::epsilon())
			{
				// Bad value
				return;
			}

			Time old_delta((*row)[model.time_delta]);
			if(old_delta<=Time::zero()+Time::epsilon())
			{
				// Bad old delta
				return;
			}
			// row(row) on the next line is bad - don't use it, because it leaves 'row' uninitialized
			//Gtk::TreeModel::iterator row(row);
			//row++;
			//if(!row)return;

			Time change_delta(new_delta-old_delta);

			if(change_delta<=Time::zero()+Time::epsilon() &&change_delta>=Time::zero()-Time::epsilon())
			{
				// Not an error, just no change
				return;
			}

			{
				Keyframe keyframe((*row)[model.keyframe]);
				synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeSetDelta"));

				if(!action)return;

				action->set_param("canvas",canvas_interface()->get_canvas());
				action->set_param("canvas_interface",canvas_interface());
				action->set_param("keyframe",keyframe);
				action->set_param("delta",change_delta);

				canvas_interface()->get_instance()->perform_action(action);
			}

			return;
		}
		else
		if(column==model.time.index())
		{
			OneMoment one_moment;

			Glib::Value<synfig::Time> x;
			g_value_init(x.gobj(),model.time.type());
			g_value_copy(value.gobj(),x.gobj());
			synfig::Keyframe keyframe(*iter->iter);

			synfig::info("KeyframeTreeStore::set_value_impl():old_time=%s",keyframe.get_time().get_string().c_str());
			keyframe.set_time(x.get());
			synfig::info("KeyframeTreeStore::set_value_impl():new_time=%s",keyframe.get_time().get_string().c_str());

			synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeSet"));

			if(!action)
				return;

			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("keyframe",keyframe);

			canvas_interface()->get_instance()->perform_action(action);
		}
		else if(column==model.description.index())
		{
			Glib::Value<Glib::ustring> x;
			g_value_init(x.gobj(),model.description.type());
			g_value_copy(value.gobj(),x.gobj());
			synfig::Keyframe keyframe(*iter->iter);
			keyframe.set_description(x.get());

			synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeSet"));

			if(!action)
				return;

			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("keyframe",keyframe);

			canvas_interface()->get_instance()->perform_action(action);
		}
		else if(column==model.active.index())
		{
			
			synfig::Keyframe keyframe((*row)[model.keyframe]);
			
			Glib::Value<bool> x;
			g_value_init(x.gobj(),model.active.type());
			g_value_copy(value.gobj(),x.gobj());
			
			synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeToggl"));

			if(!action)
				return;
			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("keyframe",keyframe);
			action->set_param("new_status",bool(x.get()));

			canvas_interface()->get_instance()->perform_action(action);
		}
		else if(column==model.keyframe.index())
		{
			g_warning("KeyframeTreeStore::set_value_impl: This column is read-only");
		}
		else
		{
			assert(0);
		}
	}
	catch(std::exception& x)
	{
		g_warning("%s", x.what());
	}
}

Gtk::TreeModelFlags
KeyframeTreeStore::get_flags_vfunc ()
{
	return Gtk::TREE_MODEL_LIST_ONLY;
}

int
KeyframeTreeStore::get_n_columns_vfunc ()
{
	return model.size();
}

GType
KeyframeTreeStore::get_column_type_vfunc (int index)
{
	return model.types()[index];
}

bool
KeyframeTreeStore::iter_next_vfunc (const iterator& xiter, iterator& iter_next) const
{
	if(!iterator_sane(xiter)) return false;

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(xiter.gobj()->user_data));

	if(iter->iter==canvas_interface()->get_canvas()->keyframe_list().end())
		return false;

	_keyframe_iterator *next(new _keyframe_iterator());
	iter_next.gobj()->user_data=static_cast<gpointer>(next);
	next->ref_count=1;
	next->index=iter->index+1;
	next->iter=iter->iter;
	++next->iter;

	if(next->iter==canvas_interface()->get_canvas()->keyframe_list().end())
		return false;

	iter_next.gobj()->stamp=stamp_;

	return true;
}

/*
bool
KeyframeTreeStore::iter_next_vfunc (GtkTreeIter* gtk_iter)
{
	if(!iterator_sane(gtk_iter)) return false;

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->user_data));

	// If we are already at the end, then we are very invalid
	if(iter->iter==canvas_interface()->get_canvas()->keyframe_list().end())
		return false;

	++(iter->iter);

	if(iter->iter==canvas_interface()->get_canvas()->keyframe_list().end())
	{
		--(iter->iter);
		return false;
	}
	(iter->index)++;
	return true;
}

bool
KeyframeTreeStore::iter_children_vfunc (GtkTreeIter* gtk_iter, const GtkTreeIter* parent)
{
	dump_iterator(gtk_iter,"gtk_iter");
	dump_iterator(parent,"parent");

	if(!parent || !iterator_sane(parent))
	{
		clear_iterator(gtk_iter);
		return false;
	}

	_keyframe_iterator *iter(new _keyframe_iterator());
	iter->ref_count=1;
	iter->index=0;
	iter->iter=canvas_interface()->get_canvas()->keyframe_list().begin();

	gtk_iter->user_data=static_cast<gpointer>(iter);
	gtk_iter->stamp=stamp_;

	return true;
}

bool
KeyframeTreeStore::iter_has_child_vfunc (const GtkTreeIter*parent)
{
	dump_iterator(parent,"parent");

	if(parent)
		return false;

	return true;
}

int
KeyframeTreeStore::iter_n_children_vfunc (const GtkTreeIter* parent)
{
	dump_iterator(parent,"parent");

	if(parent)
		return 0;

	return canvas_interface()->get_canvas()->keyframe_list().size();
}
*/

int
KeyframeTreeStore::iter_n_root_children_vfunc () const
{
	return canvas_interface()->get_canvas()->keyframe_list().size();
}

bool
KeyframeTreeStore::iter_nth_root_child_vfunc (int n, iterator& xiter)const
{
	if(canvas_interface()->get_canvas()->keyframe_list().size()==0)
	{
		return false;
	}

	if(n<0)
	{
		g_warning("KeyframeTreeStore::iter_nth_root_child_vfunc: Out of range (negative index)");
		return false;
	}
	if(n && (unsigned)n>=canvas_interface()->get_canvas()->keyframe_list().size())
	{
		g_warning("KeyframeTreeStore::iter_nth_child_vfunc: Out of range (large index)");
		return false;
	}

	_keyframe_iterator *iter(new _keyframe_iterator());
	iter->ref_count=1;
	iter->index=n;
	iter->iter=canvas_interface()->get_canvas()->keyframe_list().begin();
	while(n--)
	{
		if(iter->iter==canvas_interface()->get_canvas()->keyframe_list().end())
		{
			g_warning("KeyframeTreeStore::iter_nth_child_vfunc: >>>BUG<<< in %s on line %d",__FILE__,__LINE__);
			delete iter;
			return false;
		}
		++iter->iter;
	}
	xiter.gobj()->user_data=static_cast<gpointer>(iter);
	xiter.gobj()->stamp=stamp_;
	return true;
}

/*
bool
KeyframeTreeStore::iter_nth_child_vfunc (GtkTreeIter* gtk_iter, const GtkTreeIter* parent, int n)
{
	dump_iterator(parent,"parent");

	if(parent)
	{
		g_warning("KeyframeTreeStore::iter_nth_child_vfunc: I am a list");
		clear_iterator(gtk_iter);
		return false;
	}



	_keyframe_iterator *iter(new _keyframe_iterator());
	iter->ref_count=1;
	iter->index=n;
	iter->iter=canvas_interface()->get_canvas()->keyframe_list().begin();
	while(n--)
	{
		if(iter->iter==canvas_interface()->get_canvas()->keyframe_list().end())
		{
			g_warning("KeyframeTreeStore::iter_nth_child_vfunc: >>>BUG<<< in %s on line %d",__FILE__,__LINE__);
			delete iter;
			clear_iterator(gtk_iter);
			return false;
		}
		++iter->iter;
	}

	gtk_iter->user_data=static_cast<gpointer>(iter);
	gtk_iter->stamp=stamp_;
	return true;
}

bool
KeyframeTreeStore::iter_parent_vfunc (GtkTreeIter* gtk_iter, const GtkTreeIter* child)
{
	dump_iterator(child,"child");
	iterator_sane(child);
	clear_iterator(gtk_iter);
	return false;
}
*/

void
KeyframeTreeStore::ref_node_vfunc (iterator& xiter)const
{
	GtkTreeIter* gtk_iter(xiter.gobj());
	if(!gtk_iter || !iterator_sane(gtk_iter)) return;

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->user_data));
	iter->ref_count++;
}

void
KeyframeTreeStore::unref_node_vfunc (iterator& xiter)const
{
	GtkTreeIter* gtk_iter(xiter.gobj());
	if(!gtk_iter || !iterator_sane(gtk_iter)) return;

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->user_data));
	iter->ref_count--;
	if(!iter->ref_count)
	{
		delete iter;

		// Make this iterator invalid
		gtk_iter->stamp=0;
	}
}

Gtk::TreeModel::Path
KeyframeTreeStore::get_path_vfunc (const iterator& gtk_iter)const
{
	Gtk::TreeModel::Path path;

	// If this is the root node, then return
	// a root path
	if(!iterator_sane(gtk_iter))
		return path;

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->gobj()->user_data));

	path.push_back(iter->index);

	return path;
}

bool
KeyframeTreeStore::get_iter_vfunc (const Gtk::TreeModel::Path& path, iterator& iter)const
{
	if(path.size()>=1)
		return iter_nth_root_child_vfunc(path.front(),iter);

	// Error case
	g_warning("KeyframeTreeStore::get_iter_vfunc(): Bad path \"%s\"",path.to_string().c_str());
	//clear_iterator(iter);
	return false;
}

bool
KeyframeTreeStore::iter_is_valid (const iterator& iter) const
{
	return iterator_sane(iter);
}

void
KeyframeTreeStore::get_value_vfunc (const Gtk::TreeModel::iterator& gtk_iter, int column, Glib::ValueBase& value)const
{
	dump_iterator(gtk_iter,"gtk_iter");
	if(!iterator_sane(gtk_iter))
		return;

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->gobj()->user_data));

	switch(column)
	{
	case 0:		// Time
	{
		Glib::Value<synfig::Time> x;
		g_value_init(x.gobj(),x.value_type());
		x.set(iter->iter->get_time());
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
		return;
	}
	case 3:		// Time Delta
	{
		Glib::Value<synfig::Time> x;
		g_value_init(x.gobj(),x.value_type());

		synfig::Keyframe prev_keyframe(*iter->iter);
		synfig::Keyframe keyframe;
		{
			KeyframeList::iterator tmp(iter->iter);
			tmp++;
			if(tmp==get_canvas()->keyframe_list().end())
			{
				x.set(Time(0));
				g_value_init(value.gobj(),x.value_type());
				g_value_copy(x.gobj(),value.gobj());
				return;
			}
			keyframe=*tmp;
		}

		Time delta(0);
		try {
			delta=keyframe.get_time()-prev_keyframe.get_time();
		}catch(...) { }
		x.set(delta);
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
		return;
	}
	case 1:		// Description
	{
		g_value_init(value.gobj(),G_TYPE_STRING);
		g_value_set_string(value.gobj(),iter->iter->get_description().c_str());
		return;
	}
	case 2:		// Keyframe
	{
		Glib::Value<synfig::Keyframe> x;
		g_value_init(x.gobj(),x.value_type());
		x.set(*iter->iter);
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
		return;
	}
	case 4:		// Active
	{
		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(iter->iter->active());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	default:
		break;
	}
}

Gtk::TreeModel::Row
KeyframeTreeStore::find_row(const synfig::Keyframe &keyframe)
{
	Gtk::TreeModel::Row row(*(children().begin()));
	dump_iterator(row,"find_row,begin");
	const GtkTreeIter *gtk_iter(row.gobj());
	if(!iterator_sane(gtk_iter))
		throw std::runtime_error(_("Unable to find Keyframe in table"));

	_keyframe_iterator *iter(static_cast<_keyframe_iterator*>(gtk_iter->user_data));

	synfig::KeyframeList &keyframe_list(canvas_interface()->get_canvas()->keyframe_list());
	if(keyframe_list.empty())
		throw std::runtime_error(_("There are no keyframes in this canvas"));

	iter->index=0;

	for(iter->iter=keyframe_list.begin();iter->iter!=keyframe_list.end() && *iter->iter!=keyframe;++iter->iter)
	{
		iter->index++;
	}
	if(iter->iter==keyframe_list.end())
		throw std::runtime_error(_("Unable to find Keyframe in table"));
	return row;
}

bool
KeyframeTreeStore::find_keyframe_path(const synfig::Keyframe &keyframe, Gtk::TreeModel::Path &path)
{
	assert(keyframe);
	Gtk::TreeRow row(find_row(keyframe));
	path = get_path(row);

	return true;
}

void
KeyframeTreeStore::add_keyframe(synfig::Keyframe keyframe)
{
	try
	{
		Gtk::TreeRow row(find_row(keyframe));
		dump_iterator(row.gobj(),"add_keyframe,row");
		Gtk::TreePath path(get_path(row));

		row_inserted(path,row);

		old_keyframe_list=get_canvas()->keyframe_list();
		//old_keyframe_list.add(keyframe);
		//old_keyframe_list.sort();
	}
	catch(std::exception &x)
	{
		g_warning("%s", x.what());
	}

	// inform that something change around time to update the canvasview time widget color
	canvas_interface()->signal_time_changed()();
}

void
KeyframeTreeStore::remove_keyframe(synfig::Keyframe keyframe)
{
	try
	{
		if(1)
		{
			// Hack: (begin) the keyframe should exist in keyframe_list,
			//     otherwise find_row() function will fail.
			//     Example: try removing keyframe from composition with only 1 kf
			// Note: To avoid the hack the KeyframeTreeStore probably should be re-implemented as ListStore --KD
			canvas_interface()->get_canvas()->keyframe_list().add(keyframe);
			
			Gtk::TreeRow row(find_row(keyframe));
			dump_iterator(row,"remove_keyframe,row");
			Gtk::TreePath path(get_path(row));
			row_deleted(path);
			
			// Hack: (end) remove added keyframe
			canvas_interface()->get_canvas()->keyframe_list().erase(keyframe);

			old_keyframe_list.erase(keyframe);
		}
		else
		{
			g_warning("KeyframeTreeStore::remove_keyframe: Keyframe not in table");
		}
	}
	catch(std::exception &x)
	{
		g_warning("%s", x.what());
	}

	// inform that something change around time to update the canvasview time widget color
	canvas_interface()->signal_time_changed()();
}

void
KeyframeTreeStore::change_keyframe(synfig::Keyframe keyframe)
{
	try
	{
		Gtk::TreeRow row(find_row(keyframe));
		
		unsigned int new_index(get_index_from_model_iter(row));
		unsigned int old_index(0);
		synfig::KeyframeList::iterator iter;
		for(old_index=0,iter=old_keyframe_list.begin();iter!=old_keyframe_list.end() && (UniqueID)*iter!=(UniqueID)keyframe;++iter,old_index++)
			;

		if(iter!=old_keyframe_list.end() && new_index!=old_index)
		{
			std::vector<int> new_order;
			for(unsigned int i=0;i<old_keyframe_list.size();i++)
			{
				new_order.push_back(i);
			}
			if(new_order.size()>new_index)
			{
				new_order.erase(new_order.begin()+new_index);
				new_order.insert(new_order.begin()+old_index,new_index);

				//new_order[old_index]=

				rows_reordered (Path(), iterator(), &new_order[0]);
			}
			
		}
		
		old_keyframe_list=get_canvas()->keyframe_list();

		row=find_row(keyframe);

		dump_iterator(row,"change_keyframe,row");
		row_changed(get_path(row),row);

		// If exist, previous row should be updated too (length value)
		if (new_index != 0)
		{
			KeyframeList::iterator keyframe_prev;
			if (get_canvas()->keyframe_list().find_prev(keyframe.get_time(), keyframe_prev, false)) {
				//synfig::Keyframe keyframe_prev = *(get_canvas()->keyframe_list().find_prev(keyframe.get_time(),false));
				Gtk::TreeRow row_prev(find_row(*keyframe_prev));
				dump_iterator(row_prev,"change_keyframe,row_prev");
				row_changed(get_path(row_prev),row_prev);
			}
		}
	}
	catch(std::exception &x)
	{
		g_warning("%s", x.what());
	}

	// inform that something change around time to update the canvasview time widget color
	canvas_interface()->signal_time_changed()();
}


