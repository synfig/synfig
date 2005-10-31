/* === S Y N F I G ========================================================= */
/*!	\file canvas.cpp
**	\brief Canvas Class Member Definitions
**
**	$Id: canvas.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layer.h"
#include "canvas.h"
#include <cassert>
#include "exception.h"
#include "time.h"
#include "context.h"
#include "layer_pastecanvas.h"
#include <sigc++/bind.h>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

namespace synfig { extern Canvas::Handle open_canvas(const String &filename); };

/* === M A C R O S ========================================================= */

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

Canvas::Canvas(const string &id):
	id_			(id),
	cur_time_	(0),
	is_inline_	(false),
	is_dirty_	(true),
	op_flag_	(false)
{
	_CanvasCounter::counter++;
	clear();
}

void
Canvas::on_changed()
{
	is_dirty_=true;
	Node::on_changed();
}

Canvas::~Canvas()
{
	//if(is_inline() && parent_) assert(0);
	_CanvasCounter::counter--;
	//DEBUGPOINT();
	clear();
	begin_delete();
}

Canvas::iterator
Canvas::end()
{
	return CanvasBase::end()-1;
}

Canvas::const_iterator
Canvas::end()const
{
	return CanvasBase::end()-1;
}

Canvas::reverse_iterator
Canvas::rbegin()
{
	return CanvasBase::rbegin()+1;
}

Canvas::const_reverse_iterator
Canvas::rbegin()const
{
	return CanvasBase::rbegin()+1;
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
	return *(CanvasBase::end()-1);
}

const Layer::Handle &
Canvas::back()const
{
	return *(CanvasBase::end()-1);
}

Context
Canvas::get_context()const
{
	return begin();
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
Canvas::find_layer(const Point &pos)
{
	return get_context().hit_check(pos);
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
Canvas::set_time(Time t)const
{	
	if(is_dirty_ || !get_time().is_equal(t))
	{	
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
		get_context().set_time(t);
	}
	is_dirty_=false;
}

Canvas::LooseHandle
Canvas::get_root()const
{
	return parent_?parent_->get_root().get():const_cast<synfig::Canvas *>(this);
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
Canvas::find_value_node(const String &id)
{
	return
		ValueNode::Handle::cast_const(
			const_cast<const Canvas*>(this)->find_value_node(id)
		);
}

ValueNode::ConstHandle
Canvas::find_value_node(const String &id)const
{
	if(is_inline() && parent_)
		return parent_->find_value_node(id);
		
	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos && id.find_first_of('#')==string::npos)
		return value_node_list_.find(id);

	String canvas_id(id,0,id.rfind(':'));
	String value_node_id(id,id.rfind(':')+1);
	if(canvas_id.empty())
		canvas_id=':';
	//synfig::warning("constfind:value_node_id: "+value_node_id);
	//synfig::warning("constfind:canvas_id: "+canvas_id);

	return find_canvas(canvas_id)->value_node_list_.find(value_node_id);
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
		
	return surefind_canvas(canvas_id)->value_node_list_.surefind(value_node_id);
}

void
Canvas::add_value_node(ValueNode::Handle x, const String &id)
{
	if(is_inline() && parent_)
		return parent_->add_value_node(x,id);
//		throw runtime_error("You cannot add a ValueNode to an inline Canvas");

	//DEBUGPOINT();
	if(x->is_exported())
		throw runtime_error("ValueNode is already exported");

	if(id.empty())
		throw Exception::BadLinkName("Empty ID");

	if(id.find_first_of(':',0)!=string::npos)
		throw Exception::BadLinkName("Bad character");
	
	try
	{
		//DEBUGPOINT();
		if(PlaceholderValueNode::Handle::cast_dynamic(value_node_list_.find(id)))
			throw Exception::IDNotFound("add_value_node()");
		
		//DEBUGPOINT();
		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound)
	{
		//DEBUGPOINT();
		x->set_id(id);
	
		x->set_parent_canvas(this);
	
		if(!value_node_list_.add(x))
		{
			synfig::error("Unable to add ValueNode");
			throw std::runtime_error("Unable to add ValueNode");
		}
		//DEBUGPOINT();
		
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
	catch(Exception::IDNotFound)
	{
		x->set_id(id);	

		return;
	}
}
*/

void
Canvas::remove_value_node(ValueNode::Handle x)
{
	if(is_inline() && parent_)
		return parent_->remove_value_node(x);
//		throw Exception::IDNotFound("Canvas::remove_value_node() was called from an inline canvas");
		
	if(!x)
		throw Exception::IDNotFound("Canvas::remove_value_node() was passed empty handle");
	
	if(!value_node_list_.erase(x))
		throw Exception::IDNotFound("Canvas::remove_value_node(): ValueNode was not found inside of this canvas");

	//x->set_parent_canvas(0);
	
	x->set_id("");
}


etl::handle<Canvas>
Canvas::surefind_canvas(const String &id)
{
	if(is_inline() && parent_)
		return parent_->surefind_canvas(id);
	
	if(id.empty())
		return this;
	
	// If the ID contains a "#" character, then a filename is
	// expected on the left side. 
	if(id.find_first_of('#')!=string::npos)
	{
		// If '#' is the first character, remove it
		// and attempt to parse the ID again.
		if(id[0]=='#')
			return surefind_canvas(String(id,1));
		
		//! \todo This needs alot more optimization
		String file_name(id,0,id.find_first_of('#'));
		String external_id(id,id.find_first_of('#')+1);

		file_name=unix_to_local_path(file_name);
		
		Canvas::Handle external_canvas;
		
		// If the composition is already open, then use it.
		if(externals_.count(file_name))
			external_canvas=externals_[file_name];
		else
		{
			if(is_absolute_path(file_name))
				external_canvas=open_canvas(file_name);
			else
				external_canvas=open_canvas(get_file_path()+ETL_DIRECTORY_SEPERATOR+file_name);

			if(!external_canvas)
				throw Exception::FileNotFound(file_name);
			externals_[file_name]=external_canvas;
		}
		
		return Handle::cast_const(external_canvas.constant()->find_canvas(external_id));
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
	
	// If the first character is the seperator, then 
	// this references the root canvas.
	if(id[0]==':')
		return get_root()->surefind_canvas(string(id,1));

	// Now we know that the requested Canvas is in a child
	// of this canvas. We have to find that canvas and 
	// call "find_canvas" on it, and return the result.
	
	String canvas_name=string(id,0,id.find_first_of(':'));
	
	Canvas::Handle child_canvas=surefind_canvas(canvas_name);

	return child_canvas->surefind_canvas(string(id,id.find_first_of(':')+1));
}

Canvas::Handle
Canvas::find_canvas(const String &id)
{
	return
		Canvas::Handle::cast_const(
			const_cast<const Canvas*>(this)->find_canvas(id)
		);
}

Canvas::ConstHandle
Canvas::find_canvas(const String &id)const
{
	if(is_inline() && parent_)return parent_->find_canvas(id);

	if(id.empty())
		return this;

	// If the ID contains a "#" character, then a filename is
	// expected on the left side. 
	if(id.find_first_of('#')!=string::npos)
	{
		// If '#' is the first character, remove it
		// and attempt to parse the ID again.
		if(id[0]=='#')
			return find_canvas(String(id,1));
		
		//! \todo This needs alot more optimization
		String file_name(id,0,id.find_first_of('#'));
		String external_id(id,id.find_first_of('#')+1);
		
		file_name=unix_to_local_path(file_name);
		
		Canvas::Handle external_canvas;
		
		// If the composition is already open, then use it.
		if(externals_.count(file_name))
			external_canvas=externals_[file_name];
		else
		{
			if(is_absolute_path(file_name))
				external_canvas=open_canvas(file_name);
			else
				external_canvas=open_canvas(get_file_path()+ETL_DIRECTORY_SEPERATOR+file_name);

			if(!external_canvas)
				throw Exception::FileNotFound(file_name);
			externals_[file_name]=external_canvas;
		}
		
		return Handle::cast_const(external_canvas.constant()->find_canvas(external_id));
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
	
	// If the first character is the seperator, then 
	// this references the root canvas.
	if(id.find_first_of(':')==0)
		return get_root()->find_canvas(string(id,1));

	// Now we know that the requested Canvas is in a child
	// of this canvas. We have to find that canvas and 
	// call "find_canvas" on it, and return the result.
	
	String canvas_name=string(id,0,id.find_first_of(':'));
	
	Canvas::ConstHandle child_canvas=find_canvas(canvas_name);

	return child_canvas->find_canvas(string(id,id.find_first_of(':')+1));
}


Canvas::Handle
Canvas::create()
{
	return new Canvas("Untitled");
}

void
Canvas::push_back(etl::handle<Layer> x)
{
//	DEBUGPOINT();
//	int i(x->count());
	insert(end(),x);
	//if(x->count()!=i+1)synfig::info("push_back before %d, after %d",i,x->count());
}

void
Canvas::push_front(etl::handle<Layer> x)
{
//	DEBUGPOINT();
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
	
	x->signal_added_to_group().connect(
		sigc::bind(
			sigc::mem_fun(
				*correct_canvas,
				&Canvas::add_group_pair
			),
			loose_layer
		)
	);
	x->signal_removed_from_group().connect(
		sigc::bind(
			sigc::mem_fun(
				*correct_canvas,
				&Canvas::remove_group_pair
			),
			loose_layer
		)
	);


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
Canvas::erase(Canvas::iterator iter)
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
	(*iter)->signal_added_to_group().clear();
	(*iter)->signal_removed_from_group().clear();

	if(!op_flag_)remove_child(iter->get());
		
	CanvasBase::erase(iter);
	if(!op_flag_)changed();
}

Canvas::Handle
Canvas::clone(const GUID& deriv_guid)const
{
	synfig::String name;
	if(is_inline())
		name="inline";
	else
	{
		name=get_id()+"_CLONE";
		
		throw runtime_error("Cloning of non-inline canvases is not yet suported");
	}
	
	Handle canvas(new Canvas(name));
	
	if(is_inline())
	{
		canvas->is_inline_=true;
		canvas->parent_=0;
		//canvas->set_inline(parent());
	}

	canvas->set_guid(get_guid()^deriv_guid);

	const_iterator iter;
	for(iter=begin();iter!=end();++iter)
	{
		Layer::Handle layer((*iter)->clone(deriv_guid));
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
	
	id_="inline";
	is_inline_=true;
	parent_=parent;

	// Have the parent inherit all of the group stuff

	std::map<String,std::set<etl::handle<Layer> > >::const_iterator iter;

	for(iter=group_db_.begin();iter!=group_db_.end();++iter)
	{
		parent->group_db_[iter->first].insert(iter->second.begin(),iter->second.end());
	}
	
	rend_desc()=parent->rend_desc();
}

Canvas::Handle
Canvas::create_inline(Handle parent)
{
	assert(parent);
	//if(parent->is_inline())
	//	return create_inline(parent->parent());
	
	Handle canvas(new Canvas("inline"));
	canvas->set_inline(parent);
	return canvas;
}

Canvas::Handle
Canvas::new_child_canvas()
{
	if(is_inline() && parent_)
		return parent_->new_child_canvas();
//		runtime_error("You cannot create a child Canvas in an inline Canvas");

	// Create a new canvas
	children().push_back(create());
	Canvas::Handle canvas(children().back());

	canvas->parent_=this;

	canvas->rend_desc()=rend_desc();

	return canvas;
}

Canvas::Handle
Canvas::new_child_canvas(const String &id)
{
	if(is_inline() && parent_)
		return parent_->new_child_canvas(id);
//		runtime_error("You cannot create a child Canvas in an inline Canvas");

	// Create a new canvas
	children().push_back(create());
	Canvas::Handle canvas(children().back());
	
	canvas->set_id(id);
	canvas->parent_=this;
	canvas->rend_desc()=rend_desc();

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
		find_canvas(id);
		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound)
	{
		if(child_canvas->is_inline())
			child_canvas->is_inline_=false;
		child_canvas->id_=id;
		children().push_back(child_canvas);
		child_canvas->parent_=this;
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

	child_canvas->parent_=0;
}

void
Canvas::set_file_name(const String &file_name)
{
	if(parent())
		parent()->set_file_name(file_name);
	else
	{
		file_name_=file_name;
		signal_file_name_changed_();
	}
}

sigc::signal<void>&
Canvas::signal_file_name_changed()
{
	if(parent())
		return signal_file_name_changed();
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

void
synfig::optimize_layers(Context context, Canvas::Handle op_canvas)
{
	Context iter;

	std::vector< std::pair<float,Layer::Handle> > sort_list;
	int i;
	
	// Go ahead and start romping through the canvas to paste
	for(iter=context,i=0;*iter;iter++,i++)
	{
		Layer::Handle layer=*iter;
		float z_depth(layer->get_z_depth()*1.0001+i);
		
		// If the layer isn't active, don't worry about it
		if(!layer->active())
			continue;

		// Any layer with an amount of zero is implicitly disabled.
		ValueBase value(layer->get_param("amount"));
		if(value.get_type()==ValueBase::TYPE_REAL && value.get(Real())==0)
			continue;

		Layer_PasteCanvas* paste_canvas(static_cast<Layer_PasteCanvas*>(layer.get()));
		if(layer->get_name()=="PasteCanvas" && paste_canvas->get_time_offset()==0)
		{
			Canvas::Handle sub_canvas(Canvas::create_inline(op_canvas));
			optimize_layers(paste_canvas->get_sub_canvas()->get_context(),sub_canvas);
//#define SYNFIG_OPTIMIZE_PASTE_CANVAS 1

#ifdef SYNFIG_OPTIMIZE_PASTE_CANVAS			
			Canvas::iterator sub_iter;
			// Determine if we can just remove the paste canvas
			// altogether			
			if(paste_canvas->get_blend_method()==Color::BLEND_COMPOSITE && paste_canvas->get_amount()==1.0f && paste_canvas->get_zoom()==0 && paste_canvas->get_time_offset()==0 && paste_canvas->get_origin()==Point(0,0))
			try{
				for(sub_iter=sub_canvas->begin();sub_iter!=sub_canvas->end();++sub_iter)
				{
					Layer* layer=sub_iter->get();
					
					// any layers that deform end up breaking things
					// so do things the old way if we run into anything like this
					if(!dynamic_cast<Layer_NoDeform*>(layer))
						throw int();

					ValueBase value(layer->get_param("blend_method"));
					if(value.get_type()!=ValueBase::TYPE_INTEGER || value.get(int())!=(int)Color::BLEND_COMPOSITE)
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
			}catch(int) { }
#endif
			Layer::Handle new_layer(Layer::create("PasteCanvas"));
			dynamic_cast<Layer_PasteCanvas*>(new_layer.get())->set_do_not_muck_with_time(true);
			Layer::ParamList param_list(paste_canvas->get_param_list());
			//param_list.erase("canvas");
			new_layer->set_param_list(param_list);
			dynamic_cast<Layer_PasteCanvas*>(new_layer.get())->set_sub_canvas(sub_canvas);
			dynamic_cast<Layer_PasteCanvas*>(new_layer.get())->set_do_not_muck_with_time(false);
			layer=new_layer;
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
