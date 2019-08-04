/* === S Y N F I G ========================================================= */
/*!	\file canvas.cpp
**	\brief Canvas Class Member Definitions
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <cassert>
#include <cstring>

#include <sigc++/bind.h>

#include "time.h"

#include "general.h"
#include <synfig/localization.h>

#include "canvas.h"
#include "context.h"
#include "exception.h"
#include "filesystemnative.h"
#include "importer.h"
#include "layer.h"
#include "loadcanvas.h"
#include "valuenode_registry.h"

#include "debug/measure.h"
#include "layers/layer_pastecanvas.h"
#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_scale.h"

#endif

using namespace synfig;
using namespace etl;
using namespace std;

namespace synfig { extern Canvas::Handle open_canvas_as(const FileSystem::Identifier &identifier, const String &as, String &errors, String &warnings); };

/* === M A C R O S ========================================================= */

//#define DEBUG_SET_TIME_MEASURE

#define ALLOW_CLONE_NON_INLINE_CANVASES

struct _CanvasCounter
{
	static int counter;
	~_CanvasCounter()
	{
		if(counter)
			synfig::error("%d canvases not yet deleted!",counter);
	}
} _canvas_counter;

int _CanvasCounter::counter(0);

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Canvas::Canvas(const String &id):
	id_			(id),
	version_	(CURRENT_CANVAS_VERSION),
	cur_time_	(0),
	is_inline_	(false),
	is_dirty_	(true),
	op_flag_	(false),
	outline_grow(0.0)
{
	identifier_.file_system = FileSystemNative::instance();
	_CanvasCounter::counter++;
	clear();
}

void
Canvas::on_changed()
{
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
		printf("%s:%d Canvas::on_changed()\n", __FILE__, __LINE__);

	is_dirty_=true;
	Node::on_changed();
}

Canvas::~Canvas()
{
	// we were having a crash where pastecanvas layers were still
	// referring to a canvas after it had been destroyed;  this code
	// will stop the pastecanvas layers from referring to the canvas
	// before the canvas is destroyed

	// the set_sub_canvas(0) ends up deleting the parent-child link,
	// which deletes the current element from the set we're iterating
	// through, so we have to make sure we've incremented the iterator
	// before we mess with the pastecanvas
	std::set<Node*>::iterator iter = parent_set.begin();
	while (iter != parent_set.end())
	{
		Layer_PasteCanvas* paste_canvas = dynamic_cast<Layer_PasteCanvas*>(*iter);
		iter++;
		if(paste_canvas)
			paste_canvas->set_sub_canvas(0);
		else
			warning("destroyed canvas has a parent that is not a pastecanvas - please report if repeatable");
	}

	//if(is_inline() && parent_) assert(0);
	_CanvasCounter::counter--;
	clear();
	begin_delete();
}

int
Canvas::indexof(const const_iterator &iter) const
{
	int index = 0;
	for(const_iterator i = begin(); i != end(); ++i, ++index)
		if (i == iter)
			return index;
	return -1;
}

Canvas::iterator
Canvas::byindex(int index)
{
	for(iterator i = begin(); i != end(); ++i, --index)
		if (!index) return i;
	return end();
}

Canvas::const_iterator
Canvas::byindex(int index) const
{
	for(const_iterator i = begin(); i != end(); ++i, --index)
		if (!index) return i;
	return end();
}

Canvas::iterator
Canvas::find_index(const etl::handle<Layer> &layer, int &index)
{
	index = -1;
	int idx = 0;
	for(iterator iter = begin(); iter != end(); ++iter, ++idx)
		if (*iter == layer) { index = idx; return iter; }
	return end();
}

Canvas::const_iterator
Canvas::find_index(const etl::handle<Layer> &layer, int &index) const
{
	index = -1;
	int idx = 0;
	for(const_iterator iter = begin(); iter != end(); ++iter, ++idx)
		if (*iter == layer) { index = idx; return iter; }
	return end();
}

Canvas::iterator
Canvas::end()
{
	Canvas::iterator i = CanvasBase::end();
	return --i;
}

Canvas::const_iterator
Canvas::end() const
{
	Canvas::const_iterator i = CanvasBase::end();
	return --i;
}

Canvas::reverse_iterator
Canvas::rbegin()
{
	Canvas::reverse_iterator i = CanvasBase::rbegin();
	return ++i;
}

Canvas::const_reverse_iterator
Canvas::rbegin()const
{
	Canvas::const_reverse_iterator i = CanvasBase::rbegin();
	return ++i;
}

int
Canvas::size()const
{
	return CanvasBase::size()-1;
}

void
Canvas::clear()
{
	while(!empty())
	{
		Layer::Handle layer(front());
		//if(layer->count()>2)synfig::info("before layer->count()=%d",layer->count());

		erase(begin());
		//if(layer->count()>1)synfig::info("after layer->count()=%d",layer->count());
	}
	//CanvasBase::clear();

	// We need to keep a blank handle at the
	// end of the image list, and acts at
	// the bottom. Without it, the layers
	// would just continue going when polled
	// for a color.
	CanvasBase::push_back(Layer::Handle());

	changed();
}

bool
Canvas::empty()const
{
	return CanvasBase::size()<=1;
}

Layer::Handle &
Canvas::back()
{
	iterator i = end();
	return *--i;
}

const Layer::Handle &
Canvas::back()const
{
	const_iterator i = end();
	return *--i;
}

IndependentContext
Canvas::get_independent_context()const
{
	return IndependentContext(begin());
}

Context
Canvas::get_context(const ContextParams &params)const
{
	return Context(get_independent_context(),params);
}

Context
Canvas::get_context(const Context &parent_context)const
{
	return get_context(parent_context.get_params());
}

Context
Canvas::get_context_sorted(const ContextParams &params, CanvasBase &out_queue) const
{
	multimap<Real, Layer::Handle> layers;
	int index = 0;
	for(const_iterator i = begin(); i != end(); ++i, ++index)
	{
		assert(*i);
		// TODO: the 1.0001 constant should be somehow user defined
		Real depth = (*i)->get_z_depth()*1.0001 + (Real)index;
		layers.insert(pair<Real, Layer::Handle>(depth, *i));
	}

	out_queue.clear();
	for(multimap<Real, Layer::Handle>::const_iterator i = layers.begin(); i != layers.end(); ++i)
		out_queue.push_back(i->second);
	out_queue.push_back(Layer::Handle());

	return Context(out_queue.begin(), params);
}

const ValueNodeList &
Canvas::value_node_list()const
{
	if(is_inline() && parent_)
		return parent_->value_node_list();
	return value_node_list_;
}

KeyframeList &
Canvas::keyframe_list()
{
	if(is_inline() && parent_)
		return parent_->keyframe_list();
	return keyframe_list_;
}

const KeyframeList &
Canvas::keyframe_list()const
{
	if(is_inline() && parent_)
		return parent_->keyframe_list();
	return keyframe_list_;
}

etl::handle<Layer>
Canvas::find_layer(const ContextParams &context_params, const Point &pos)
{
	return get_context(context_params).hit_check(pos);
}

static bool
valid_id(const String &x)
{
	static const char bad_chars[]=" :#@$^&()*";
	unsigned int i;

	if(!x.empty() && x[0]>='0' && x[0]<='9')
		return false;

	for(i=0;i<sizeof(bad_chars);i++)
		if(x.find_first_of(bad_chars[i])!=string::npos)
			return false;

	return true;
}

void
Canvas::set_id(const String &x)
{
	if(is_inline() && parent_)
		throw runtime_error("Inline Canvas cannot have an ID");

	if(!valid_id(x))
		throw runtime_error("Invalid ID");
	id_=x;
	signal_id_changed_();
}

void
Canvas::set_name(const String &x)
{
	name_=x;
	signal_meta_data_changed()("name");
	signal_meta_data_changed("name")();
}

void
Canvas::set_author(const String &x)
{
	author_=x;
	signal_meta_data_changed()("author");
	signal_meta_data_changed("author")();
}

void
Canvas::set_description(const String &x)
{
	description_=x;
	signal_meta_data_changed()("description");
	signal_meta_data_changed("description")();
}

void
Canvas::set_outline_grow(Real x)
{
	if (fabs(outline_grow - x) > 1e-8)
	{
		outline_grow = x;
		get_independent_context().set_outline_grow(outline_grow);
	}
}

Real
Canvas::get_outline_grow()const
{
	return outline_grow;
}

void
Canvas::set_time(Time t)const
{
	if(is_dirty_ || !get_time().is_equal(t))
	{
		#ifdef DEBUG_SET_TIME_MEASURE
		debug::Measure measure("Canvas::set_time", true);
		#endif

#if 0
		if(is_root())
		{
			synfig::info("is_dirty_=%d",is_dirty_);
			synfig::info("get_time()=%f",(float)get_time());
			synfig::info("t=%f",(float)t);
		}
#endif

		// ...questionable
		const_cast<Canvas&>(*this).cur_time_=t;

		is_dirty_=false;
		get_independent_context().set_time(t);
	}
	is_dirty_=false;
}

void
Canvas::load_resources(Time t)const
{
	get_independent_context().load_resources(t);
}

Canvas::LooseHandle
Canvas::get_root()const
{
	// printf("root(%lx) = ", uintptr_t(this));
	if (parent_)
		return parent_->get_root();
	// printf("%lx\n", uintptr_t(this));
	return const_cast<synfig::Canvas *>(this);
}

Canvas::LooseHandle
Canvas::get_non_inline_ancestor()const
{
	if (is_inline() && parent_)
		return parent_->get_non_inline_ancestor();
	return const_cast<synfig::Canvas *>(this);
}

int
Canvas::get_depth(etl::handle<Layer> layer)const
{
	const_iterator iter;
	int i(0);
	for(iter=begin();iter!=end();++iter,i++)
	{
		if(layer==*iter)
			return i;
	}
	return -1;
}

String
Canvas::get_relative_id(etl::loose_handle<const Canvas> x)const
{
	if(x->get_root()==this)
		return ":";
	if(is_inline() && parent_)
		return parent_->_get_relative_id(x);
	return _get_relative_id(x);
}

String
Canvas::_get_relative_id(etl::loose_handle<const Canvas> x)const
{
	if(is_inline() && parent_)
		return parent_->_get_relative_id(x);

	if(x.get()==this)
		return String();

	if(parent()==x.get())
		return get_id();

	String id;

	const Canvas* canvas=this;

	for(;!canvas->is_root();canvas=canvas->parent().get())
		id=':'+canvas->get_id()+id;

	if(x && get_root()!=x->get_root())
	{
		//String file_name=get_file_name();
		//String file_path=x->get_file_path();

		String file_name;
		if(is_absolute_path(get_file_name()))
			file_name=etl::relative_path(x->get_file_path(),get_file_name());
		else
			file_name=get_file_name();

		// If the path of X is inside of file_name,
		// then remove it.
		//if(file_name.size()>file_path.size())
		//	if(file_path==String(file_name,0,file_path.size()))
		//		file_name.erase(0,file_path.size()+1);

		id=file_name+'#'+id;
	}

	return id;
}

ValueNode::Handle
Canvas::find_value_node(const String &id, bool might_fail)
{
	return
		ValueNode::Handle::cast_const(
			const_cast<const Canvas*>(this)->find_value_node(id, might_fail)
		);
}

ValueNode::ConstHandle
Canvas::find_value_node(const String &id, bool might_fail)const
{
	if(is_inline() && parent_)
		return parent_->find_value_node(id, might_fail);

	if(id.empty())
	{
		if (!might_fail) ValueNode::breakpoint();
		throw Exception::IDNotFound("Empty ID");
	}

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos && id.find_first_of('#')==string::npos)
		return value_node_list_.find(id, might_fail);

	String canvas_id(id,0,id.rfind(':'));
	String value_node_id(id,id.rfind(':')+1);
	if(canvas_id.empty())
		canvas_id=':';
	//synfig::warning("constfind:value_node_id: "+value_node_id);
	//synfig::warning("constfind:canvas_id: "+canvas_id);

	String warnings;
	return find_canvas(canvas_id, warnings)->value_node_list_.find(value_node_id, might_fail);
}

ValueNode::Handle
Canvas::surefind_value_node(const String &id)
{
	if(is_inline() && parent_)
		return parent_->surefind_value_node(id);

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos && id.find_first_of('#')==string::npos)
		return value_node_list_.surefind(id);

	String canvas_id(id,0,id.rfind(':'));
	String value_node_id(id,id.rfind(':')+1);
	if(canvas_id.empty())
		canvas_id=':';

	String warnings;
	return surefind_canvas(canvas_id,warnings)->value_node_list_.surefind(value_node_id);
}

void
Canvas::add_value_node(ValueNode::Handle x, const String &id)
{
	if(is_inline() && parent_)
		return parent_->add_value_node(x,id);
//		throw runtime_error("You cannot add a ValueNode to an inline Canvas");

	if(x->is_exported())
		throw runtime_error("ValueNode is already exported");

	if(id.empty())
		throw Exception::BadLinkName("Empty ID");

	if(id.find_first_of(':',0)!=string::npos)
		throw Exception::BadLinkName("Bad character");

	try
	{
		if(PlaceholderValueNode::Handle::cast_dynamic(value_node_list_.find(id, true)))
			throw Exception::IDNotFound("add_value_node()");

		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound&)
	{
		x->set_id(id);

		x->set_parent_canvas(this);

		if(!value_node_list_.add(x))
		{
			synfig::error("Unable to add ValueNode");
			throw std::runtime_error("Unable to add ValueNode");
		}

		return;
	}
}

/*
void
Canvas::rename_value_node(ValueNode::Handle x, const String &id)
{
	if(id.empty())
		throw Exception::BadLinkName("Empty ID");

	if(id.find_first_of(": ",0)!=string::npos)
		throw Exception::BadLinkName("Bad character");

	try
	{
		if(PlaceholderValueNode::Handle::cast_dynamic(value_node_list_.find(id)))
			throw Exception::IDNotFound("rename_value_node");
		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound&)
	{
		x->set_id(id);

		return;
	}
}
*/

void
Canvas::remove_value_node(ValueNode::Handle x, bool might_fail)
{
	if(is_inline() && parent_)
		return parent_->remove_value_node(x, might_fail);
//		throw Exception::IDNotFound("Canvas::remove_value_node() was called from an inline canvas");

	if(!x)
	{
		if (!might_fail) ValueNode::breakpoint();
		throw Exception::IDNotFound("Canvas::remove_value_node() was passed empty handle");
	}

	if(!value_node_list_.erase(x))
	{
		if (!might_fail) ValueNode::breakpoint();
		throw Exception::IDNotFound("Canvas::remove_value_node(): ValueNode was not found inside of this canvas");
	}

	//x->set_parent_canvas(0);

	x->set_id("");
}

Canvas::Handle
Canvas::surefind_canvas(const String &id, String &warnings)
{
	if(is_inline() && parent_)
		return parent_->surefind_canvas(id,warnings);

	if(id.empty())
		return this;

	// If the ID contains a "#" character, then a filename is
	// expected on the left side.
	if(id.find_first_of('#')!=string::npos)
	{
		// If '#' is the first character, remove it
		// and attempt to parse the ID again.
		if(id[0]=='#')
			return surefind_canvas(String(id,1),warnings);

		//! \todo This needs a lot more optimization
		String file_name(id,0,id.find_first_of('#'));
		String external_id(id,id.find_first_of('#')+1);

		file_name=unix_to_local_path(file_name);

		Canvas::Handle external_canvas;

		if(!is_absolute_path(file_name))
			file_name = get_file_path()+ETL_DIRECTORY_SEPARATOR+file_name;
		// Before look up the external canvases
		// let's check if this is the current canvas
		if(get_file_name() == file_name)
			return this;
		// If the composition is already open, then use it.
		if(externals_.count(file_name))
			external_canvas=externals_[file_name];
		else
		{
			String errors;
			external_canvas=open_canvas_as(get_identifier().file_system->get_identifier(file_name), file_name, errors, warnings);
			if(!external_canvas)
				throw runtime_error(errors);
			externals_[file_name]=external_canvas;
		}

		return Handle::cast_const(external_canvas.constant()->find_canvas(external_id, warnings));
	}

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos)
	{
		Children::iterator iter;

		// Search for the image in the image list,
		// and return it if it is found
		for(iter=children().begin();iter!=children().end();iter++)
			if(id==(*iter)->get_id())
				return *iter;

		// Create a new canvas and return it
		//synfig::warning("Implicitly creating canvas named "+id);
		return new_child_canvas(id);
	}

	// If the first character is the separator, then
	// this references the root canvas.
	if(id[0]==':')
		return get_root()->surefind_canvas(string(id,1),warnings);

	// Now we know that the requested Canvas is in a child
	// of this canvas. We have to find that canvas and
	// call "find_canvas" on it, and return the result.

	String canvas_name=string(id,0,id.find_first_of(':'));

	Canvas::Handle child_canvas=surefind_canvas(canvas_name,warnings);

	return child_canvas->surefind_canvas(string(id,id.find_first_of(':')+1),warnings);
}

Canvas::Handle
Canvas::find_canvas(const String &id, String &warnings)
{
	return
		Canvas::Handle::cast_const(
			const_cast<const Canvas*>(this)->find_canvas(id, warnings)
		);
}

Canvas::ConstHandle
Canvas::find_canvas(const String &id, String &warnings)const
{
	if(is_inline() && parent_)
		return parent_->find_canvas(id, warnings);

	if(id.empty())
		return this;

	// If the ID contains a "#" character, then a filename is
	// expected on the left side.
	if(id.find_first_of('#')!=string::npos)
	{
		// If '#' is the first character, remove it
		// and attempt to parse the ID again.
		if(id[0]=='#')
			return find_canvas(String(id,1), warnings);

		//! \todo This needs a lot more optimization
		String file_name(id,0,id.find_first_of('#'));
		String external_id(id,id.find_first_of('#')+1);

		file_name=unix_to_local_path(file_name);

		Canvas::Handle external_canvas;

		if(!is_absolute_path(file_name))
			file_name = get_file_path()+ETL_DIRECTORY_SEPARATOR+file_name;

		// If the composition is already open, then use it.
		if(externals_.count(file_name))
			external_canvas=externals_[file_name];
		else
		{
			String errors, warnings;
			external_canvas=open_canvas_as(get_identifier().file_system->get_identifier(file_name), file_name, errors, warnings);
			if(!external_canvas)
				throw runtime_error(errors);
			externals_[file_name]=external_canvas;
		}

		return Handle::cast_const(external_canvas.constant()->find_canvas(external_id, warnings));
	}

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos)
	{
		Children::const_iterator iter;

		// Search for the image in the image list,
		// and return it if it is found
		for(iter=children().begin();iter!=children().end();iter++)
			if(id==(*iter)->get_id())
				return *iter;

		throw Exception::IDNotFound("Child Canvas in Parent Canvas: (child)"+id);
	}

	// If the first character is the separator, then
	// this references the root canvas.
	if(id[0]==':')
		return get_root()->find_canvas(string(id,1), warnings);

	// Now we know that the requested Canvas is in a child
	// of this canvas. We have to find that canvas and
	// call "find_canvas" on it, and return the result.

	String canvas_name=string(id,0,id.find_first_of(':'));

	Canvas::ConstHandle child_canvas=find_canvas(canvas_name, warnings);

	return child_canvas->find_canvas(string(id,id.find_first_of(':')+1), warnings);
}

Canvas::Handle
Canvas::create()
{
	return new Canvas("Untitled");
}

void
Canvas::push_back(etl::handle<Layer> x)
{
//	int i(x->count());
	insert(end(),x);
	//if(x->count()!=i+1)synfig::info("push_back before %d, after %d",i,x->count());
}

void
Canvas::push_front(etl::handle<Layer> x)
{
//	int i(x->count());
	insert(begin(),x);
	//if(x->count()!=i+1)synfig::error("push_front before %d, after %d",i,x->count());
}

void
Canvas::insert(iterator iter,etl::handle<Layer> x)
{
//	int i(x->count());
	CanvasBase::insert(iter,x);

	/*if(x->count()!=i+1)
	{
		synfig::error(__FILE__":%d: Canvas::insert(): ***FAILURE*** before %d, after %d",__LINE__,i,x->count());
		return;
		//throw runtime_error("Canvas Insertion Failed");
	}*/

	x->set_canvas(this);

	add_child(x.get());

	LooseHandle correct_canvas(this);
	//while(correct_canvas->is_inline())correct_canvas=correct_canvas->parent();
	Layer::LooseHandle loose_layer(x);

	add_connection(loose_layer,
				   sigc::connection(
					   x->signal_added_to_group().connect(
						   sigc::bind(
							   sigc::mem_fun(
								   *correct_canvas,
								   &Canvas::add_group_pair),
							   loose_layer))));
	add_connection(loose_layer,
				   sigc::connection(
					   x->signal_removed_from_group().connect(
						   sigc::bind(
							   sigc::mem_fun(
								   *correct_canvas,
								   &Canvas::remove_group_pair),
							   loose_layer))));

	if(!x->get_group().empty())
		add_group_pair(x->get_group(),x);

	changed();
}

void
Canvas::push_back_simple(etl::handle<Layer> x)
{
	CanvasBase::insert(end(),x);
	changed();
}

void
Canvas::erase(iterator iter)
{
	if(!(*iter)->get_group().empty())
		remove_group_pair((*iter)->get_group(),(*iter));

	// HACK: We really shouldn't be wiping
	// out these signals entirely. We should
	// only be removing the specific connections
	// that we made. At the moment, I'm too
	// lazy to add the code to keep track
	// of those connections, and no one else
	// is using these signals, so I'll just
	// leave these next two lines like they
	// are for now - darco 07-30-2004

	// so don't wipe them out entirely
	// - dooglus 09-21-2007
	disconnect_connections(*iter);

	if(!op_flag_)remove_child(iter->get());

	CanvasBase::erase(iter);
	if(!op_flag_)changed();
}

Canvas::Handle
Canvas::clone(const GUID& deriv_guid, bool for_export)const
{
	synfig::String name;
	if(is_inline())
		name=_("in line");
	else
	{
		name=get_id()+"_CLONE";

#ifndef ALLOW_CLONE_NON_INLINE_CANVASES
		throw runtime_error("Cloning of non-inline canvases is not yet supported");
#endif	// ALLOW_CLONE_NON_INLINE_CANVASES
	}

	Handle canvas(new Canvas(name));

	if(is_inline() && !for_export)
	{
		canvas->is_inline_=true;
		// \todo this was setting parent=0 - is there a reason for that?
		// this was causing bug 1838132, where cloning an inline canvas that contains an imported image fails
		// it was failing to ascertain the absolute pathname of the imported image, since it needs the pathname
		// of the canvas to get that, which is stored in the parent canvas
		canvas->rend_desc() = rend_desc();
		canvas->set_parent(parent());
	}

	canvas->set_guid(get_guid()^deriv_guid);

	if (canvas->parent().empty())
		canvas->set_file_name(get_file_name());

	const_iterator iter;
	for(iter=begin();iter!=end();++iter)
	{
		Layer::Handle layer((*iter)->clone(canvas, deriv_guid));
		if(layer)
		{
			assert(layer.count()==1);
			int presize(size());
			canvas->push_back(layer);
			if(!(layer.count()>1))
			{
				synfig::error("Canvas::clone(): Cloned layer insertion failure!");
				synfig::error("Canvas::clone(): \tlayer.count()=%d",layer.count());
				synfig::error("Canvas::clone(): \tlayer->get_name()=%s",layer->get_name().c_str());
				synfig::error("Canvas::clone(): \tbefore size()=%d",presize);
				synfig::error("Canvas::clone(): \tafter size()=%d",size());
			}
			assert(layer.count()>1);
		}
		else
		{
			synfig::error("Unable to clone layer");
		}
	}

	canvas->signal_group_pair_removed().clear();
	canvas->signal_group_pair_added().clear();

	return canvas;
}

void
Canvas::set_inline(LooseHandle parent)
{
	if(is_inline_ && parent_)
	{

	}

	id_=_("in line");
	is_inline_=true;

	// Have the parent inherit all of the group stuff
	std::map<String,std::set<etl::handle<Layer> > >::const_iterator iter;
	for(iter=group_db_.begin();iter!=group_db_.end();++iter)
		parent->group_db_[iter->first].insert(iter->second.begin(),iter->second.end());
	rend_desc()=parent->rend_desc();

	set_parent(parent);
}

Canvas::Handle
Canvas::create_inline(Handle parent)
{
	assert(parent);
	//if(parent->is_inline())
	//	return create_inline(parent->parent());

	Handle canvas(new Canvas(_("in line")));
	canvas->set_inline(parent);
	return canvas;
}

Canvas::Handle
Canvas::new_child_canvas()
{
	if(is_inline() && parent_)
		return parent_->new_child_canvas();

	// Create a new canvas
	children().push_back(create());
	Canvas::Handle canvas(children().back());

	canvas->rend_desc()=rend_desc();
	canvas->set_parent(this);

	return canvas;
}

Canvas::Handle
Canvas::new_child_canvas(const String &id)
{
	if(is_inline() && parent_)
		return parent_->new_child_canvas(id);

	// Create a new canvas
	children().push_back(create());
	Canvas::Handle canvas(children().back());

	canvas->set_id(id);
	canvas->rend_desc()=rend_desc();
	canvas->set_parent(this);

	return canvas;
}

Canvas::Handle
Canvas::add_child_canvas(Canvas::Handle child_canvas, const synfig::String& id)
{
	if(is_inline() && parent_)
		return parent_->add_child_canvas(child_canvas,id);

	if(child_canvas->parent() && !child_canvas->is_inline())
		throw std::runtime_error("Cannot add child canvas because it belongs to someone else!");

	if(!valid_id(id))
		throw runtime_error("Invalid ID");
	
	try
	{
		String warnings;
		find_canvas(id, warnings);
		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound&)
	{
		if(child_canvas->is_inline())
			child_canvas->is_inline_=false;
		child_canvas->id_=id;
		children().push_back(child_canvas);
		child_canvas->set_parent(this);
	}

	return child_canvas;
}

void
Canvas::remove_child_canvas(Canvas::Handle child_canvas)
{
	if(is_inline() && parent_)
		return parent_->remove_child_canvas(child_canvas);

	if(child_canvas->parent_!=this)
		throw runtime_error("Given child does not belong to me");

	if(find(children().begin(),children().end(),child_canvas)==children().end())
		throw Exception::IDNotFound(child_canvas->get_id());

	children().remove(child_canvas);
	child_canvas->set_parent(0);
}

void
Canvas::set_parent(const Canvas::LooseHandle &parent)
{
	parent_ = parent;
	on_parent_set();
}

void
Canvas::on_parent_set()
{
	// parent canvas is important field,
	// so assume that canvas replaced for layers
	for(std::list<Handle>::iterator i = children().begin(); i != children().end(); ++i)
		(*i)->on_parent_set();
	for(iterator i = begin(); *i; ++i)
		(*i)->on_canvas_set();
}

void
Canvas::set_file_name(const String &file_name_orig)
{
	String file_name = FileSystem::fix_slashes(file_name_orig);
	if(parent())
		parent()->set_file_name(file_name);
	else
	{
		String old_name(file_name_);
		file_name_=file_name;

		// when a canvas is made, its name is ""
		// then, before it's saved or even edited, it gets a name like "Synfig Animation 23", in the local language
		// we don't want to register the canvas' filename in the canvas map until it gets a real filename
		if (old_name != "")
		{
			file_name_=file_name;
			std::map<synfig::String, etl::loose_handle<Canvas> >::iterator iter;
			for(iter=get_open_canvas_map().begin();iter!=get_open_canvas_map().end();++iter)
				if(iter->second==this)
					break;
			if (iter == get_open_canvas_map().end())
				CanvasParser::register_canvas_in_map(this, file_name);
			else
				signal_file_name_changed_();
		}
	}
}

sigc::signal<void>&
Canvas::signal_file_name_changed()
{
	if(parent())
		return parent()->signal_file_name_changed();
	else
		return signal_file_name_changed_;
}

String
Canvas::get_file_name()const
{
	if(parent())
		return parent()->get_file_name();
	return file_name_;
}

String
Canvas::get_file_path()const
{
	if(parent())
		return parent()->get_file_path();
	return dirname(file_name_);
}

FileSystem::Handle
Canvas::get_file_system()const
{
	return get_identifier().file_system;
}

void
Canvas::set_identifier(const FileSystem::Identifier &identifier)
{
	identifier_ = identifier;
}

const FileSystem::Identifier&
Canvas::get_identifier()const
{
	return parent() ? parent()->get_identifier() : identifier_;
}

String
Canvas::get_meta_data(const String& key)const
{
	if(!meta_data_.count(key))
		return String();
	return meta_data_.find(key)->second;
}

void
Canvas::set_meta_data(const String& key, const String& data)
{
	if(meta_data_[key]!=data)
	{
		meta_data_[key]=data;
		signal_meta_data_changed()(key);
		signal_meta_data_changed(key)();
	}
}

void
Canvas::erase_meta_data(const String& key)
{
	if(meta_data_.count(key))
	{
		meta_data_.erase(key);
		signal_meta_data_changed()(key);
		signal_meta_data_changed(key)();
	}
}

std::list<String>
Canvas::get_meta_data_keys()const
{
	std::list<String> ret;

	std::map<String,String>::const_iterator iter;

	for(iter=meta_data_.begin();!(iter==meta_data_.end());++iter)
		ret.push_back(iter->first);

	return ret;
}

/* note - the "Motion Blur" and "Duplicate" layers need the dynamic
		  parameters of any PasteCanvas layers they loop over to be
		  maintained.  When the variables in the following function
		  refer to "motion blur", they mean either of these two
		  layers. */
void
synfig::optimize_layers(Time time, Context context, Canvas::Handle op_canvas, bool seen_motion_blur_in_parent)
{
	Context iter;

	std::vector< std::pair<float,Layer::Handle> > sort_list;
	int i, motion_blur_i=0;	// motion_blur_i is for resolving which layer comes first in the event of a z_depth tie
	float motion_blur_z_depth=0; // the z_depth of the least deep motion blur layer in this context
	bool seen_motion_blur_locally = false;
	bool motion_blurred; // the final result - is this layer blurred or not?

	// If the parent didn't cause us to already be motion blurred,
	// check whether there's a motion blur in this context,
	// and if so, calculate its z_depth.
	if (!seen_motion_blur_in_parent)
		for(iter=context,i=0;*iter;iter++,i++)
		{
			Layer::Handle layer=*iter;
			
			const float layer_visibility=context.z_depth_visibility(*layer);

			// If the layer isn't active, don't worry about it
			if(!context.active(*layer) || layer_visibility==0.0)
				continue;

			// Any layer with an amount of zero is implicitly disabled.
			ValueBase value(layer->get_param("amount"));
			if(value.get_type()==type_real && value.get(Real())==0)
				continue;

			if(layer->get_name()=="MotionBlur" || layer->get_name()=="duplicate")
			{
				float z_depth(layer->get_true_z_depth(time));

				// If we've seen a motion blur before in this context...
				if (seen_motion_blur_locally)
				{
					// ... then we're only interested in this one if it's less deep...
					if (z_depth < motion_blur_z_depth)
					{
						motion_blur_z_depth = z_depth;
						motion_blur_i = i;
					}
				}
				// ... otherwise we're always interested in it.
				else
				{
					motion_blur_z_depth = z_depth;
					motion_blur_i = i;
					seen_motion_blur_locally = true;
				}
			}
		}

	// Go ahead and start romping through the canvas to paste
	for(iter=context,i=0;*iter;iter++,i++)
	{
		Layer::Handle layer=*iter;
		float z_depth(layer->get_true_z_depth(time));
		const float layer_visibility=context.z_depth_visibility(*layer);
		//synfig::info("Visibility of %s called %s = %f", layer->get_name().c_str(), layer->get_description().c_str(), layer_visibility);

		// If the layer isn't active or isn't visible in its z depth range,
		// don't worry about it
		if(!context.active(*layer) || layer_visibility==0.0)
			continue;

		// Any layer with an amount of zero is implicitly disabled.
		ValueBase value(layer->get_param("amount"));
		if(value.get_type()==type_real && value.get(Real())==0)
			continue;

		// note: this used to include "&& paste_canvas->get_time_offset()==0", but then
		//		 time-shifted layers weren't being sorted by z-depth (bug #1806852)
		Layer_PasteCanvas* paste_canvas(dynamic_cast<Layer_PasteCanvas*>(layer.get()));
		if(paste_canvas!=NULL)
		{
			// we need to blur the sub canvas if:
			// our parent is blurred,
			// or the child is lower than a local blur,
			// or the child is at the same z_depth as a local blur, but later in the context

#if 0 // DEBUG
			if (seen_motion_blur_in_parent)					synfig::info("seen BLUR in parent\n");
			else if (seen_motion_blur_locally)
				if (z_depth > motion_blur_z_depth)			synfig::info("paste is deeper than BLUR\n");
				else if (z_depth == motion_blur_z_depth) {	synfig::info("paste is same depth as BLUR\n");
					if (i > motion_blur_i)					synfig::info("paste is physically deeper than BLUR\n");
					else									synfig::info("paste is less physically deep than BLUR\n");
				} else										synfig::info("paste is less deep than BLUR\n");
			else											synfig::info("no BLUR at all\n");
#endif	// DEBUG

			motion_blurred = (seen_motion_blur_in_parent ||
							  (seen_motion_blur_locally &&
							   (z_depth > motion_blur_z_depth ||
								(z_depth == motion_blur_z_depth && i > motion_blur_i))));

			Canvas::Handle sub_canvas(Canvas::create_inline(op_canvas));
			Canvas::Handle paste_sub_canvas = paste_canvas->get_sub_canvas();
			if(paste_sub_canvas)
			{
				ContextParams params=context.get_params();
				paste_canvas->apply_z_range_to_params(params);
				optimize_layers(time, paste_sub_canvas->get_context(params),sub_canvas,motion_blurred);
			}

// \todo: uncommenting the following breaks the rendering of at least examples/backdrop.sifz quite severely
// #define SYNFIG_OPTIMIZE_PASTE_CANVAS
#ifdef SYNFIG_OPTIMIZE_PASTE_CANVAS
			Canvas::iterator sub_iter;

			// Determine if we can just remove the paste canvas altogether
			if (paste_canvas->get_blend_method()	== Color::BLEND_COMPOSITE	&&
				paste_canvas->get_amount()			== 1.0f						&&
				paste_canvas->get_zoom()			== 0						&&
				paste_canvas->get_time_offset()		== 0						&&
				paste_canvas->get_origin()			== Point(0,0)				&&
				//paste_canvas->get_param("outline_grow").get(Real()) == 0.0
				)
				try {
					//synfig::info("outline grow is 0.0----------------");
					for(sub_iter=sub_canvas->begin();sub_iter!=sub_canvas->end();++sub_iter)
					{
						Layer* layer=sub_iter->get();

						// any layers that deform end up breaking things
						// so do things the old way if we run into anything like this
						if(!dynamic_cast<Layer_NoDeform*>(layer))
							throw int();

						ValueBase value(layer->get_param("blend_method"));
						if(value.get_type()!=type_integer || value.get(int())!=(int)Color::BLEND_COMPOSITE)
							throw int();
					}

					// It has turned out that we don't need a paste canvas
					// layer, so just go ahead and add all the layers onto
					// the current stack and be done with it
					while(sub_canvas->size())
					{
						sort_list.push_back(std::pair<float,Layer::Handle>(z_depth,sub_canvas->front()));
						//op_canvas->push_back_simple(sub_canvas->front());
						sub_canvas->pop_front();
					}
					continue;
				}
				catch(int)
				{ }
#endif	// SYNFIG_OPTIMIZE_PASTE_CANVAS

			etl::handle<Layer_PasteCanvas> new_layer =
				etl::handle<Layer_PasteCanvas>::cast_dynamic( Layer::create(paste_canvas->get_name()) );
			new_layer->set_optimized(true);
			if (motion_blurred)
			{
				Layer::DynamicParamList dynamic_param_list(paste_canvas->dynamic_param_list());
				for(Layer::DynamicParamList::const_iterator iter(dynamic_param_list.begin()); iter != dynamic_param_list.end(); ++iter)
					new_layer->connect_dynamic_param(iter->first, iter->second);
			}
			Layer::ParamList param_list(paste_canvas->get_param_list());
			//param_list.erase("canvas");
			new_layer->set_param_list(param_list);
			new_layer->set_sub_canvas(sub_canvas);
			layer=new_layer;
		}
		else					// not a PasteCanvas - does it use blend method 'Straight'?
		{
			/* when we use the 'straight' blend method, every pixel on the layer affects the layers underneath,
			 * not just the non-transparent pixels; the following workarea wraps non-pastecanvas layers in a
			 * new pastecanvas to ensure that the straight blend affects the full plane, not just the area
			 * within the layer's bounding box
			 */

			// \todo: this code probably needs modification to work properly with motionblur and duplicate
			etl::handle<Layer_Composite> composite = etl::handle<Layer_Composite>::cast_dynamic(layer);

			/* some layers (such as circle) don't touch pixels that aren't
			 * part of the circle, so they don't get blended correctly when
			 * using a straight blend.  so we encapsulate the circle, and the
			 * encapsulation layer takes care of the transparent pixels
			 * for us.  if we do that for all layers, however, then the
			 * distortion layers no longer work, since they have no
			 * context to work on.  the Layer::reads_context() method
			 * returns true for layers which need to be able to see
			 * their context.  we can't encapsulate those.
			 */

			/*
			if (composite &&
				Color::is_straight(composite->get_blend_method()) &&
				!composite->reads_context())
			{
				Canvas::Handle sub_canvas(Canvas::create_inline(op_canvas));
				// don't use clone() because it re-randomizes the seeds of any random valuenodes
				sub_canvas->push_back(composite = composite->simple_clone());
				layer = Layer::create("group");
				composite->set_description(strprintf("Wrapped clone of '%s'", composite->get_non_empty_description().c_str()));
				layer->set_description(strprintf("Group wrapper for '%s'", composite->get_non_empty_description().c_str()));
				Layer_PasteCanvas* paste_canvas(static_cast<Layer_PasteCanvas*>(layer.get()));
				paste_canvas->set_blend_method(composite->get_blend_method());
				paste_canvas->set_amount(composite->get_amount());
				sub_canvas->set_time(time); // region and outline don't calculate their bounding rects until their time is set
				composite->set_blend_method(Color::BLEND_STRAIGHT); // do this before calling set_sub_canvas(), but after set_time()
				composite->set_amount(1.0f); // after set_time()
				paste_canvas->set_sub_canvas(sub_canvas);
			}
			*/
		}
		// Alright, the layer is included in the sorted list
		// let's look if it is a composite and if it is partially visible
		etl::handle<Layer_Composite> composite = etl::handle<Layer_Composite>::cast_dynamic(layer);
		if(composite && layer_visibility < 1.0)
		{
			// Let's clone the composite layer if it is not a Paste Canvas
			// (because paste will always be new layer)
			// Oops... not always...
			//if(dynamic_cast<Layer_PasteCanvas*>(layer.get()) != NULL)
				composite = etl::handle<Layer_Composite>::cast_dynamic(composite->simple_clone());
			// Let's scale the amount parameter by the z depth visibility
			ValueNode::Handle amount;
			// First look if amount is dynamic:
			if(composite->dynamic_param_list().count("amount"))
			{
				amount=composite->dynamic_param_list().find("amount")->second;
			}
			else
			// It is normal constant parameter
			{
				amount=ValueNode_Const::create(layer->get_param("amount").get(Real()));
			}
			// Connect a ValueNode_Scale to the amount parameter with the right sub-parameters
			ValueNode::Handle value_node=ValueNodeRegistry::create("scale", ValueBase(Real()));
			ValueNode_Scale::Handle scale=ValueNode_Scale::Handle::cast_dynamic(value_node);
			scale->set_link("link", amount);
			scale->set_link("scalar", ValueNode_Const::create(layer_visibility));
			composite->connect_dynamic_param("amount", value_node);
			layer=composite;
		}
		sort_list.push_back(std::pair<float,Layer::Handle>(z_depth,layer));
		//op_canvas->push_back_simple(layer);
	}

	//sort_list.sort();
	stable_sort(sort_list.begin(),sort_list.end());
	std::vector< std::pair<float,Layer::Handle> >::iterator iter2;
	for(iter2=sort_list.begin();iter2!=sort_list.end();++iter2)
		op_canvas->push_back_simple(iter2->second);
	op_canvas->op_flag_=true;
}

void
Canvas::get_times_vfunc(Node::time_set &set) const
{
	const_iterator	i = begin(),
				iend = end();

	for(; i != iend; ++i)
	{
		const Node::time_set &tset = (*i)->get_times();
		set.insert(tset.begin(),tset.end());
	}
}

std::set<etl::handle<Layer> >
Canvas::get_layers_in_group(const String&group)
{
	if(is_inline() && parent_)
		return parent_->get_layers_in_group(group);

	if(group_db_.count(group)==0)
		return std::set<etl::handle<Layer> >();
	return group_db_.find(group)->second;
}

std::set<String>
Canvas::get_groups()const
{
	if(is_inline() && parent_)
		return parent_->get_groups();

	std::set<String> ret;
	std::map<String,std::set<etl::handle<Layer> > >::const_iterator iter;
	for(iter=group_db_.begin();iter!=group_db_.end();++iter)
		ret.insert(iter->first);
	return ret;
}

int
Canvas::get_group_count()const
{
	if(is_inline() && parent_)
		return parent_->get_group_count();

	return group_db_.size();
}

void
Canvas::add_group_pair(String group, etl::handle<Layer> layer)
{
	group_db_[group].insert(layer);
	if(group_db_[group].size()==1)
		signal_group_added()(group);
	else
		signal_group_changed()(group);

	signal_group_pair_added()(group,layer);

	if(is_inline()  && parent_)
		return parent_->add_group_pair(group,layer);
}

void
Canvas::remove_group_pair(String group, etl::handle<Layer> layer)
{
	group_db_[group].erase(layer);

	signal_group_pair_removed()(group,layer);

	if(group_db_[group].empty())
	{
		group_db_.erase(group);
		signal_group_removed()(group);
	}
	else
		signal_group_changed()(group);

	if(is_inline() && parent_)
		return parent_->remove_group_pair(group,layer);
}

void
Canvas::add_connection(etl::loose_handle<Layer> layer, sigc::connection connection)
{
	connections_[layer].push_back(connection);
}

void
Canvas::disconnect_connections(etl::loose_handle<Layer> layer)
{
	std::vector<sigc::connection>::iterator iter;
	for(iter=connections_[layer].begin();iter!=connections_[layer].end();++iter)
		iter->disconnect();
	connections_[layer].clear();
}

void
Canvas::rename_group(const String&old_name,const String&new_name)
{
	if(is_inline() && parent_)
		return parent_->rename_group(old_name,new_name);

	{
		std::map<String,std::set<etl::handle<Layer> > >::iterator iter;
		iter=group_db_.find(old_name);
		if(iter!=group_db_.end())
		for(++iter;iter!=group_db_.end() && iter->first.find(old_name)==0;iter=group_db_.find(old_name),++iter)
		{
			String name(iter->first,old_name.size(),String::npos);
			name=new_name+name;
			rename_group(iter->first,name);
		}
	}

	std::set<etl::handle<Layer> > layers(get_layers_in_group(old_name));
	std::set<etl::handle<Layer> >::iterator iter;

	for(iter=layers.begin();iter!=layers.end();++iter)
	{
		(*iter)->remove_from_group(old_name);
		(*iter)->add_to_group(new_name);
	}
}

void
Canvas::register_external_canvas(String file_name, Handle canvas)
{
	if(!is_absolute_path(file_name)) file_name = get_file_path()+ETL_DIRECTORY_SEPARATOR+file_name;
	externals_[file_name] = canvas;
}

#ifdef _DEBUG
void
Canvas::show_externals(String file, int line, String text) const
{
	printf("  .----- (externals for %lx '%s')\n  |  %s:%d %s\n", uintptr_t(this), get_name().c_str(), file.c_str(), line, text.c_str());
	std::map<String, Handle>::iterator iter;
	for (iter = externals_.begin(); iter != externals_.end(); iter++)
	{
		synfig::String first(iter->first);
		etl::loose_handle<Canvas> second(iter->second);
		printf("  |    %40s : %lx (%d)\n", first.c_str(), uintptr_t(&*second), second->count());
	}
	printf("  `-----\n\n");
}

void
Canvas::show_structure(int i) const
{
	if(i==0)
		printf("---Canvas Structure----\n");
	IndependentContext iter;
	for(iter=get_independent_context();*iter;iter++)
	{
		Layer::Handle layer=*iter;
		printf("%d: %s : %s", i, layer->get_name().c_str(), layer->get_non_empty_description().c_str());
		etl::handle<Layer_Composite> composite = etl::handle<Layer_Composite>::cast_dynamic(layer);
		if(composite)
			printf(": %d: %f", composite->get_blend_method(), composite->get_amount());
		else
			printf(": no composite");
		printf("\n");
		if(dynamic_cast<Layer_PasteCanvas*>(layer.get()) != NULL)
		{
			Layer_PasteCanvas* paste_canvas(static_cast<Layer_PasteCanvas*>(layer.get()));
			paste_canvas->get_sub_canvas()->show_structure(i+1);
		}
	}
}
#endif	// _DEBUG

// #define DEBUG_INVOKE_SVNCR

// this is only ever called from valuenode_dynamiclist.cpp and valuenode_staticlist.cpp
// the container is a ValueNode_{Static,Dynamic}List
// the content is the entry
void
Canvas::invoke_signal_value_node_child_removed(etl::handle<ValueNode> container, etl::handle<ValueNode> content)
{
	signal_value_node_child_removed()(container, content);
	Canvas::Handle canvas(this);
#ifdef DEBUG_INVOKE_SVNCR
	printf("%s:%d removed stuff from a canvas %lx with %zd parents\n", __FILE__, __LINE__, uintptr_t(canvas.get()), canvas->parent_set.size());
#endif
	for (std::set<Node*>::iterator iter = canvas->parent_set.begin(); iter != canvas->parent_set.end(); iter++)
	{
		if (dynamic_cast<Layer*>(*iter))
		{
			Layer* layer(dynamic_cast<Layer*>(*iter));
#ifdef DEBUG_INVOKE_SVNCR
			printf("it's a layer %lx\n", uintptr_t(layer));
			printf("%s:%d it's a layer with %zd parents\n", __FILE__, __LINE__, layer->parent_set.size());
#endif
			for (std::set<Node*>::iterator iter = layer->parent_set.begin(); iter != layer->parent_set.end(); iter++)
				if (dynamic_cast<Canvas*>(*iter))
				{
					Canvas* canvas(dynamic_cast<Canvas*>(*iter));
#ifdef DEBUG_INVOKE_SVNCR
					printf("it's a canvas %lx\n", uintptr_t(canvas));
#endif
					if (canvas->get_non_inline_ancestor())
					{
#ifdef DEBUG_INVOKE_SVNCR
						printf("%s:%d recursively invoking signal vn child removed on non_inline_ancestor %lx\n", __FILE__, __LINE__, uintptr_t(canvas->get_non_inline_ancestor().get()));
#endif
						canvas->get_non_inline_ancestor()->invoke_signal_value_node_child_removed(container, content);
					}
#ifdef DEBUG_INVOKE_SVNCR
					else
						printf("can't get non_inline_ancestor_canvas\n");
#endif
				}
#ifdef DEBUG_INVOKE_SVNCR
				else
					printf("not a canvas\n");
#endif
		}
#ifdef DEBUG_INVOKE_SVNCR
		else
			printf("not a layer\n");
#endif
	}
}

#if 0
void
Canvas::show_canvas_ancestry(String file, int line, String note)const
{
	printf("%s:%d %s:\n", file.c_str(), line, note.c_str());
	show_canvas_ancestry();
}

void
Canvas::show_canvas_ancestry()const
{
	String layer;
	// printf("%s:%d parent set size = %zd\n", __FILE__, __LINE__, parent_set.size());
	if (parent_set.size() == 1)
	{
		Node* node(*(parent_set.begin()));
		if (dynamic_cast<Layer*>(node))
		{
			layer = (dynamic_cast<Layer*>(node))->get_description();
		}
	}

	printf("  canvas %lx %6s parent %7lx layer %-10s id %8s name '%8s' desc '%8s'\n",
		   uintptr_t(this), is_inline_?"inline":"", uintptr_t(parent_.get()),
		   layer.c_str(),
		   get_id().c_str(), get_name().c_str(), get_description().c_str());
	if (parent_) parent_->show_canvas_ancestry();
	else printf("\n");
}
#endif

String
Canvas::get_string()const
{
	return String("Canvas: ") + get_description();
}

void
Canvas::fill_sound_processor(SoundProcessor &soundProcessor) const
{
	for(IndependentContext c = begin(); *c; ++c)
		if ((*c)->active())
			(*c)->fill_sound_processor(soundProcessor);
}
