/* === S Y N F I G ========================================================= */
/*!	\file layer.h
**	\brief Layer Class Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**  Copyright (c) 2011-2013 Carlos López
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

#ifndef __SYNFIG_LAYER_H
#define __SYNFIG_LAYER_H

/* === H E A D E R S ======================================================= */

#include <map>

#include <ETL/handle>

#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "cairo.h"
#include "guid.h"
#include "interpolation.h"
#include "node.h"
#include "paramdesc.h"
#include "progresscallback.h"
#include "real.h"
#include "rendering/task.h"
#include "string.h"
#include "time.h"
#include "vector.h"
#include "value.h"
#include <giomm/filemonitor.h>

/* === M A C R O S ========================================================= */

// This macros should be removed when rendering optimization complete
#define RENDER_TRANSFORMED_IF_NEED(file, line) \
	if (!renddesc.get_transformation_matrix().is_identity()) \
		return render_transformed(this, context, surface, quality, renddesc, cb, file, line);


//! Defines various variables and the create method, common for all importers.
//! To be used in the private part of the importer class definition.
// Static `name`, `local_name`, `version`, and `category` is needed to register layer.
// We also have non-static versions of these methods in order to be able to change the
// layer name (visual presentation) at runtime depending on its parameters.
#define SYNFIG_LAYER_MODULE_EXT \
	public: \
	static const char* get_register_name(); \
	static const char* get_register_version(); \
	static const char* get_register_local_name(); \
	static const char* get_register_category(); \
	static Layer *create();
//! Sets the name of the layer
#define SYNFIG_LAYER_SET_NAME(class,x) \
	const char* class::get_register_name() { return x; }

//! Sets the local name of the layer
#define SYNFIG_LAYER_SET_LOCAL_NAME(class,x) \
	const char* class::get_register_local_name() { return x; }

//! Sets the category of the layer
#define SYNFIG_LAYER_SET_CATEGORY(class,x) \
	const char* class::get_register_category() { return x; }

//! Sets the version string for the layer
#define SYNFIG_LAYER_SET_VERSION(class,x) \
	const char* class::get_register_version() { return x; }

//! Defines de implementation of the create method for the importer
#define SYNFIG_LAYER_INIT(class) \
	synfig::Layer* class::create() \
	{ \
		return new class(); \
	}

//! Imports a parameter if it is of the same type as param
#define IMPORT_VALUE(x) \
	if (#x=="param_"+param && x.get_type()==value.get_type()) \
	{ \
		x=value; \
        static_param_changed(param); \
		return true; \
	}

//! Imports a parameter 'x' and perform an action usually based on
//! some condition 'y'
#define IMPORT_VALUE_PLUS_BEGIN(x) \
	if (#x=="param_"+param && x.get_type()==value.get_type()) \
	{ \
		x=value; \
		{
#define IMPORT_VALUE_PLUS_END \
		} \
        static_param_changed(param); \
		return true; \
	}
#define IMPORT_VALUE_PLUS(x,y) \
        IMPORT_VALUE_PLUS_BEGIN(x) \
			y; \
        IMPORT_VALUE_PLUS_END

//! Exports a parameter if it is the same type as value
#define EXPORT_VALUE(x) \
	if (#x=="param_"+param) \
	{ \
		synfig::ValueBase ret; \
		ret.copy(x); \
		return ret; \
	}

//! Exports the name or the local name of the layer
#define EXPORT_NAME() \
	if (param=="Name" || param=="name" || param=="name__") \
		return get_register_name(); \
	else if (param=="local_name__") \
		return synfigcore_localize(get_register_local_name());

//! Exports the version of the layer
#define EXPORT_VERSION() \
	if (param=="Version" || param=="version" || param=="version__") \
		return get_register_version();

//! This is used as the category for layer book entries which represent aliases of layers.
//! It prevents these layers showing up in the menu.
#define CATEGORY_DO_NOT_USE "Do Not Use"

//! Sets the interpolation defaults for the layer
#define SET_INTERPOLATION_DEFAULTS() \
{ \
	Vocab vocab(get_param_vocab()); \
	Vocab::const_iterator viter; \
	for(viter=vocab.begin();viter!=vocab.end();viter++) \
	{ \
		ValueBase v=get_param(viter->get_name()); \
		v.set_interpolation(viter->get_interpolation()); \
		set_param(viter->get_name(), v); \
	} \
} \

//! Sets the static defaults for the layer
#define SET_STATIC_DEFAULTS() \
{ \
	Vocab vocab(get_param_vocab()); \
	Vocab::const_iterator viter; \
	for(viter=vocab.begin();viter!=vocab.end();viter++) \
	{ \
		ValueBase v=get_param(viter->get_name()); \
		v.set_static(viter->get_static()); \
		set_param(viter->get_name(), v); \
	} \
} \

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class CairoColor;
class CairoSurface;
class Canvas;
class Color;
class Context;
class ContextParams;
class IndependentContext;
class Rect;
class RendDesc;
class SoundProcessor;
class Surface;
class Transform;
class ValueNode;


/*!	\class Layer
**	\todo writeme
**	\see Canvas
*/
class Layer : public Node
{
	friend class ValueNode;
	friend class IndependentContext;
	friend class Context;

	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	//! Type that represents a pointer to a Layer's constructor.
	/*! As a pointer to the constructor, it represents a "factory" of layers.
	*/
	typedef Layer* (*Factory)();

	struct BookEntry
	{
		Factory factory;
		String name;
		String local_name;
		String category;
		String version;
		BookEntry(): factory() { }
		BookEntry(Factory		 factory,
				  const String	&name,
				  const String	&local_name,
				  const String	&category,
				  const String	&version):
			factory(factory),
			name(name),
			local_name(local_name),
			category(category),
			version(version) { }
	};

	//! Book of types of layers indexed by layer type name.
	/*! While the sifz file is read, each time a new layer entry is found,
	**  the factory constructor that the "factory" pointer member of the
	**  "BookEntry" struct points to, is called, and a new layer of that type
	**  is created.
	**  \sa Layer::Factory
	*/
	typedef std::map<String,BookEntry> Book;

	static void register_in_book(const BookEntry &);

	static Book& book();

	//! Inits the book of layers and inserts in it the basic layers that
	//! doesn't depend on modules
	/*! \todo motionblur should be in the mod_filter module
	*/
	static bool subsys_init();

	//! Stops the layer system by deleting the book of registered layers
	static bool subsys_stop();

	//! Map of Value Base parameters indexed by name
	typedef std::map<String,ValueBase> ParamList;

	typedef etl::handle<Layer> Handle;

	typedef etl::loose_handle<Layer> LooseHandle;

	typedef etl::handle<const Layer> ConstHandle;

	//! Map of parameters that are animated Value Nodes indexed by the param name
	typedef std::map<String,etl::rhandle<ValueNode> > DynamicParamList;

	//! A list type which describes all the parameters that a layer has.
	/*! \see get_param_vocab() */
	typedef ParamVocab Vocab;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	/*! \c true if the layer is visible, \c false if it is to be skipped
	**	\see set_active(), enable(), disable, active()
	*/
	bool active_;

	//! flag to prevent re-apply optimization features
	bool optimized_;

	/*! When \c true, layer will skipped while final rendering
	**	but will still present onto work view.
	**	\see set_exclude_from_rendering(), get_exclude_from_rendering()
	*/
	bool exclude_from_rendering_;

	//! Handle to the canvas to which this layer belongs
	etl::loose_handle<Canvas> canvas_;

	//! Map of parameter with animated value nodes
	DynamicParamList dynamic_param_list_;

	//! A description of what this layer does
	String description_;

	//! The depth parameter of the layer in the layer stack
	ValueBase param_z_depth;

	//! \writeme
	mutable Time time_mark;
	mutable Real outline_grow_mark;

	//! Contains the name of the group that this layer belongs to
	String group_;

	//! Signal to connect to the signal_deleted canvas's member
	//! Used to do let a layer with a canvas parent that doesn't exists
	//! Instead of that it connects to a zero canvas
	//! \see Layer::set_canvas()
	sigc::connection parent_death_connect_;

	// added ability to monitor file changes
	Glib::RefPtr<Gio::FileMonitor> file_monitor;
	void on_file_changed(const Glib::RefPtr<Gio::File>&, const Glib::RefPtr<Gio::File>&, Gio::FileMonitorEvent);
	sigc::connection monitor_connection;
	std::string monitored_path;
public:
	bool monitor(const std::string& path); // append file monitor (returns true on success, false on fail)


	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	//!	Status Changed
	sigc::signal<void> signal_status_changed_;

	//!	Parameter changed
	sigc::signal<void,String> signal_param_changed_;

	//!	Description Changed
	sigc::signal<void> signal_description_changed_;

	//!	Moved
	sigc::signal<void, int, etl::handle<Canvas> > signal_moved_;

	sigc::signal<void, String> signal_added_to_group_;

	sigc::signal<void, String> signal_removed_from_group_;

	sigc::signal<void, String> signal_static_param_changed_;

	sigc::signal<void, String> signal_dynamic_param_changed_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	//!	Status Changed
	sigc::signal<void>& signal_status_changed() { return signal_status_changed_; }

	//!	Parameter changed
	sigc::signal<void,String>& signal_param_changed() { return signal_param_changed_; }

	//!	Description Changed
	sigc::signal<void>& signal_description_changed() { return signal_description_changed_;}

	//!	Moved
	sigc::signal<void, int, etl::handle<Canvas> >& signal_moved() { return signal_moved_; }

	sigc::signal<void, String>& signal_added_to_group() { return signal_added_to_group_; }

	sigc::signal<void, String>& signal_removed_from_group() { return signal_removed_from_group_; }

	sigc::signal<void, String>& signal_static_param_changed() { return signal_static_param_changed_; }

	sigc::signal<void, String>& signal_dynamic_param_changed() { return signal_dynamic_param_changed_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

	void static_param_changed(const String &param);
	void dynamic_param_changed(const String &param);

	Layer();

public:
	virtual ~Layer();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	virtual void on_canvas_set();
	virtual void on_static_param_changed(const String &param);
	virtual void on_dynamic_param_changed(const String &param);

	//! Adds this layer to the given layer group
	void add_to_group(const String&);

	//! Removes this layer from the given layer group
	void remove_from_group(const String&);

	//! Removes this layer from all layer groups
	void remove_from_all_groups();

	//! Gets the name of the group that this layer belongs to
	String get_group()const;

	//! Retrieves the dynamic param list member
	//! \see DynamicParamList
	const DynamicParamList &dynamic_param_list()const { return dynamic_param_list_; }

	//! Enables the layer for rendering (Making it \em active)
	void enable() { set_active(true); }

	//! Disables the layer for rendering. (Making it \em inactive)
	/*! When a layer is disabled, it will be skipped when the
	**	canvas is rendered. */
	void disable() { set_active(false); }

	//! Sets the 'active' flag for the Layer to the state described by \a x
	/*! When a layer is disabled, it will be skipped when the
	**	canvas is rendered. */
	void set_active(bool x);

	//! Returns that status of the 'active' flag
	bool active()const { return active_; }

	//! flag to prevent re-apply optimization features
	bool optimized()const { return optimized_; }

	//! set flag to prevent re-apply optimization features
	void set_optimized(bool x) { optimized_ = x; }

	//! Sets the 'exclude_from_rendering' flag for the Layer
	/*! When set, layer will skipped while final rendering
	**	but will still present onto work view. */
	void set_exclude_from_rendering(bool x);

	//! Returns that status of the 'exclude_from_rendering' flag
	bool get_exclude_from_rendering()const { return exclude_from_rendering_; }

	//! Returns the position of the layer in the canvas.
	/*! Returns negative on error */
	int get_depth()const;

	//! Gets the non animated z depth of the layer
	float get_z_depth()const { return param_z_depth.get(Real()); }

	//! Gets the z depth of the layer at a time t
	float get_z_depth(const synfig::Time& t)const;

	//! Gets the true z depth of the layer (index + parameter)
	float get_true_z_depth(const synfig::Time& t)const;

	//! Gets the true z depth of the layer (index + parameter) for current time
	float get_true_z_depth()const;

	//! Sets the z depth of the layer (non animated)
	void set_z_depth(float x) { param_z_depth=ValueBase(Real(x)); }

	//! Sets the Canvas that this Layer is a part of
	void set_canvas(etl::loose_handle<Canvas> canvas);

	//! Returns a handle to the Canvas to which this Layer belongs
	etl::loose_handle<Canvas> get_canvas()const;

	//! Returns the description of the layer
	const String& get_description()const { return description_; }


	String get_string()const;

	//! Sets the description of the layer
	void set_description(const String& x);

	//! Returns the layer's description if it's not empty, else its local name
	const String get_non_empty_description()const { return get_description().empty() ? get_local_name() : get_description(); }

	//! Returns the localised version of the given layer parameter
	const String get_param_local_name(const String &param_name)const;

	//! Returns a handle to the Parent PasteCanvas layer or NULL if layer belongs to root canvas
	/*! Notice that it could return the wrong handle to PasteCanvas if the layer */
	/*! belongs to a exported canvas (canvas can be referenced multiple times)*/
	Layer::LooseHandle get_parent_paste_canvas_layer()const;

	/*
 --	** -- V I R T U A L   F U N C T I O N S -----------------------------------
	*/

public:
	//! Returns the rectangle that includes the layer
	//! \see synfig::Rect
	virtual Rect get_bounding_rect()const;

	//!Returns the rectangle that includes the context of the layer
	//!\see synfig::Rect synfig::Context
	virtual Rect get_full_bounding_rect(Context context)const;

	//! Returns a string containing the name of the Layer
	virtual String get_name()const;

	//! Returns a string containing the localized name of the Layer
	virtual String get_local_name()const;

	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;

	//! Gets the version string for this layer
	virtual String get_version()const;

	//! Returns a handle to the Transform class of the layer
	//! \see synfig::Transform
	virtual etl::handle<Transform> get_transform()const;

	//! Sets the virtual version to use for backwards-compatibility
	/*!
	**	\see reset_version() */
	virtual bool set_version(const String &ver);

	//! Resets the virtual version
	/*!
	**	\see set_version() */
	virtual void reset_version();

	//!	Sets the parameter described by \a param to \a value.
	/*!	\param param The name of the parameter to set
	**	\param value What the parameter is to be set to.
	**	\return \c true on success, \c false upon rejection or failure.
	**		If it returns \c false, then the Layer is assumed to remain unchanged.
	**	\sa get_param()
	**	\todo \a param should be of the type <tt>const String \&param</tt>
	*/
	virtual bool set_param(const String &param, const ValueBase &value);

	//!	Sets a list of parameters
	virtual bool set_param_list(const ParamList &);

	//! Get the value of the specified parameter.
	/*!	\return The requested parameter value, or (upon failure) a NIL ValueBase.
	**	\sa set_param()
	**	\todo \a param should be of the type <tt>const String \&</tt>
	*/
	virtual ValueBase get_param(const String &param)const;

	//! Get a list of all of the parameters and their values
	virtual ParamList get_param_list()const;

	Time get_time_mark() const { return time_mark; }
	void set_time_mark(Time time) const { time_mark = time; }
	void clear_time_mark() const { time_mark = Time::end(); }

	Real get_outline_grow_mark() const { return outline_grow_mark; }
	void set_outline_grow_mark(Real outline_grow) const { outline_grow_mark = outline_grow; }
	void clear_outline_grow_mark() const { outline_grow_mark = 0.0; }

	//! Sets the \a time for the Layer and those under it
	/*!	\param context		Context iterator referring to next Layer.
	**	\param time			writeme
	**	\see Context::set_time()
	*/
	void set_time(IndependentContext context, Time time)const;
	
	//! Loads external resources (frames) for the Layer recursively
	/*!	\param context		Context iterator referring to next Layer.
	**	\param time			writeme
	**	\see Context::load_resources()
	*/
	void load_resources(IndependentContext context, Time time)const;

	//! Sets the \a outline_grow for the Layer and those under it
	/*!	\param context		Context iterator referring to next Layer.
	**	\param outline_grow	writeme
	**	\see Context::set_outline_grow()
	*/
	void set_outline_grow(IndependentContext context, Real outline_grow)const;

	//! Gets the blend color of the Layer in the context at \a pos
	/*!	\param context		Context iterator referring to next Layer.
	**	\param pos		Point which indicates where the Color should come from
	**	\see Context::get_color()
	*/
	virtual Color get_color(Context context, const Point &pos)const;
	virtual CairoColor get_cairocolor(Context context, const Point &pos)const;

	// Temporary function to render transformed layer for layers which yet not support transformed rendering
	static bool render_transformed(const Layer *layer, Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb, const char *file, int line);

	//! Renders the Canvas to the given Surface in an accelerated manner
	/*!	\param context		Context iterator referring to next Layer.
	**	\param surface		Pointer to Surface to render to.
	**	\param quality		The requested quality-level to render at.
	**	\param renddesc		The associated RendDesc.
	**	\param cb			Pointer to callback object. May be NULL if there is no callback.
	**	\return \c true on success, \c false on failure
	**	\see Context::accelerated_render()
	*/
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t* cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

protected:
	virtual void set_time_vfunc(IndependentContext context, Time time) const;
	virtual void load_resources_vfunc(IndependentContext context, Time time) const;
	virtual void set_outline_grow_vfunc(IndependentContext context, Real outline_grow) const;
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;

	virtual RendDesc get_sub_renddesc_vfunc(const RendDesc &renddesc) const;
	virtual void get_sub_renddesc_vfunc(const RendDesc &renddesc, std::vector<RendDesc> &out_descs) const;

public:
	void get_sub_renddesc(const RendDesc &renddesc, std::vector<RendDesc> &out_descs) const;
	RendDesc get_sub_renddesc(const RendDesc &renddesc, int index = 0) const;

	//! Returns rendering task for context
	/*!	\param context		Context iterator referring to next Layer.
	**	\return \c null on failure
	**	\see Context::build_rendering_task()
	*/
	rendering::Task::Handle build_rendering_task(Context context)const;

	//! Checks to see if a part of the layer is directly under \a point
	/*!	\param context		Context iterator referring to next Layer.
	**	\param point		The point to check
	**	\return 	The handle of the layer under \a point. If there is not
	**				a layer under \a point, then returns an empty handle.
	**	\see Context::hit_check
	*/
	virtual Handle hit_check(Context context, const Point &point)const;

	//! Duplicates the Layer
	virtual Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	//! Returns true if the layer needs to be able to examine its context.
	/*! context to render itself, other than for simple blending.  For
	**  example, the blur layer will return true - it can't do its job
	**  if it can't see its context, and the circle layer will return
	**  false - rendering a circle doesn't depend on the underlying
	**  context until the final blend operation. */
	virtual bool reads_context()const;

	//! Duplicates the Layer without duplicating the value nodes
	virtual Handle simple_clone()const;

	//! Connects the parameter to another Value Node
	virtual bool connect_dynamic_param(const String& param, etl::loose_handle<ValueNode>);

	//! Disconnects the parameter from any Value Node
	virtual bool disconnect_dynamic_param(const String& param);

	virtual void fill_sound_processor(SoundProcessor &soundProcessor) const;

protected:

	//! This is called whenever a parameter is changed
	virtual void on_changed();

	virtual void on_child_changed(const Node *x);

	//! Called to figure out the animation time information
	virtual void get_times_vfunc(Node::time_set &set) const;

	/*
 --	** -- S T A T I C  F U N C T I O N S --------------------------------------
	*/

public:

	//! Creates a Layer of type \a type
	/*!	If the Layer type is unknown, then a Mime layer is created in its place.
	**	\param type	A string describing the name of the layer to construct.
	**	\return Always returns a handle to a new Layer.
	**	\see Mime
	*/
	static Layer::LooseHandle create(const String &type);

}; // END of class Layer

}; // END of namespace synfig


/* === E N D =============================================================== */

#endif
