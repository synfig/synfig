/* === S Y N F I G ========================================================= */
/*!	\file layer.h
**	\brief Layer Class Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "string_decl.h"
#include <map>
#include <ETL/handle>
#include "real.h"
#include "string.h"
#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include "node.h"
#include "time.h"
#include "guid.h"

/* === M A C R O S ========================================================= */

//! \writeme
#define SYNFIG_LAYER_MODULE_EXT															\
	public:																				\
	static const char name__[], version__[], cvs_id__[], local_name__[], category__[];	\
	static Layer *create();

//! Sets the name of the layer
#define SYNFIG_LAYER_SET_NAME(class,x)													\
	const char class::name__[]=x

//! Sets the local name of the layer
#define SYNFIG_LAYER_SET_LOCAL_NAME(class,x)											\
	const char class::local_name__[]=x;

//! Sets the category of the layer
#define SYNFIG_LAYER_SET_CATEGORY(class,x)												\
	const char class::category__[]=x

//! Sets the version string for the layer
#define SYNFIG_LAYER_SET_VERSION(class,x)												\
	const char class::version__[]=x

//! Sets the CVS ID string for the layer
#define SYNFIG_LAYER_SET_CVS_ID(class,x)												\
	const char class::cvs_id__[]=x

//! \writeme
#define SYNFIG_LAYER_INIT(class)														\
	synfig::Layer* class::create()														\
	{																					\
		return new class();																\
	}

//! \writeme
#define IMPORT_PLUS(x,y)																\
	if (param==#x && value.same_type_as(x))												\
	{																					\
		value.put(&x);																	\
		{																				\
			y;																			\
		}																				\
		return true;																	\
	}

//! \writeme
#define IMPORT_AS(x,y)																	\
	if (param==y && value.same_type_as(x))												\
	{																					\
		value.put(&x);																	\
		return true;																	\
	}

//! \writeme
#define IMPORT(x)																		\
	IMPORT_AS(x,#x)

//! \writeme
#define EXPORT_AS(x,y)																	\
	if (param==y)																		\
		return ValueBase(x);

//! \writeme
#define EXPORT(x)																		\
	EXPORT_AS(x,#x)

//! \writeme
#define EXPORT_NAME()																	\
	if (param=="Name" || param=="name" || param=="name__")								\
		return name__;																	\
	else if (param=="local_name__")														\
		return dgettext("synfig",local_name__);

//! \writeme
#define EXPORT_VERSION()																\
	if (param=="Version" || param=="version" || param=="version__")						\
		return version__;

//! This is used as the category for layer book entries which represent aliases of layers.
//! It prevents these layers showing up in the menu.
#define CATEGORY_DO_NOT_USE "Do Not Use"

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Canvas;
class Vector;
typedef Vector Point;
class Canvas;
class ParamDesc;
class ParamVocab;
class ValueNode;
class ValueBase;
class Time;
class Surface;
class RendDesc;
class ProgressCallback;
class Context;
class Color;
class Transform;
class Rect;
class GUID;


/*!	\class Layer
**	\todo writeme
**	\see Canvas
*/
class Layer : public Node
{
	friend class ValueNode;
	friend class Context;

	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	//! Type that represents a pointer to a layer's constructor
	typedef Layer* (*Factory)();

	struct BookEntry
	{
		Factory factory;
		String name;
		String local_name;
		String category;
		String cvs_id;
		String version;
		BookEntry() { }
		BookEntry(Factory		 factory,
				  const String	&name,
				  const String	&local_name,
				  const String	&category,
				  const String	&cvs_id,
				  const String	&version):
			factory(factory),
			name(name),
			local_name(local_name),
			category(category),
			cvs_id(cvs_id),
			version(version) { }
	};

	typedef std::map<String,BookEntry> Book;

	static void register_in_book(const BookEntry &);

	static Book& book();

	static bool subsys_init();

	static bool subsys_stop();

	typedef std::map<String,ValueBase> ParamList;

	typedef etl::handle<Layer> Handle;

	typedef etl::loose_handle<Layer> LooseHandle;

	typedef etl::handle<const Layer> ConstHandle;

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

	//! Handle to the canvas to which this layer belongs
	etl::loose_handle<Canvas> canvas_;

	DynamicParamList dynamic_param_list_;

	//! A description of what this layer does
	String description_;

	//! \writeme
	float z_depth_;

	//! \writeme
	mutable Time dirty_time_;

	//! Contains the name of the group that this layer belongs to
	String group_;

	//! \writeme
	sigc::connection parent_death_connect_;

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

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

	Layer();

public:
	virtual ~Layer();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	virtual void on_canvas_set();

	//! Adds this layer to the given layer group
	void add_to_group(const String&);

	//! Removes this layer from the given layer group
	void remove_from_group(const String&);

	//! Removes this layer from all layer groups
	void remove_from_all_groups();

	//! Gets the name of the group that this layer belongs to
	String get_group()const;

	//! writeme
	//DynamicParamList &dynamic_param_list() { return dynamic_param_list_; }

	//! \todo writeme
	const DynamicParamList &dynamic_param_list()const { return dynamic_param_list_; }

	bool connect_dynamic_param(const String& param, etl::loose_handle<ValueNode>);
	bool disconnect_dynamic_param(const String& param);

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

	//! Returns the position of the layer in the canvas.
	/*! Returns negative on error */
	int get_depth()const;

	//! \writeme
	float get_z_depth()const { return z_depth_; }

	//! \writeme
	float get_z_depth(const synfig::Time& t)const;

	//! \writeme
	void set_z_depth(float x) { z_depth_=x; }

	//! Sets the Canvas that this Layer is a part of
	void set_canvas(etl::loose_handle<Canvas> canvas);

	//! Returns a handle to the Canvas to which this Layer belongs
	etl::loose_handle<Canvas> get_canvas()const;

	//! \writeme
	const String& get_description()const { return description_; }

	String get_string()const;

	//! \writeme
	void set_description(const String& x);

	//! Returns the layer's description if it's not empty, else its local name
	const String get_non_empty_description()const { return get_description().empty() ? get_local_name() : get_description(); }

	//! Returns the localised version of the given layer parameter
	const String get_param_local_name(const String &param_name)const;

	/*
 --	** -- V I R T U A L   F U N C T I O N S -----------------------------------
	*/

public:
	virtual Rect get_bounding_rect()const;

	virtual Rect get_full_bounding_rect(Context context)const;

	//! Returns a string containing the name of the Layer
	virtual String get_name()const;

	//! Returns a string containing the localized name of the Layer
	virtual String get_local_name()const;

	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;

	//! Gets the version string for this layer
	virtual String get_version()const;

	//! \writeme
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

	//! Sets the \a time for the selected Layer and those under it
	/*!	\param context		Context iterator referring to next Layer.
	**	\param time			writeme
	**	\see Handle::set_time()
	*/
	virtual void set_time(Context context, Time time)const;

	//! Sets the \a time for the selected Layer and those under it for a specific \a point
	/*!	\param context		Context iterator referring to next Layer.
	**	\param time			writeme
	**	\param point		writeme
	**	\see Handle::set_time()
	**	\todo \a point should be of the type <tt>const Point \&</tt> */
	virtual void set_time(Context context, Time time, const Point &point)const;

	//! Gets the color of the Canvas at \a pos
	/*!	\param context		Context iterator referring to next Layer.
	**	\param pos		Point which indicates where the Color should come from
	**	\see Handle::get_color()
	*/
	virtual Color get_color(Context context, const Point &pos)const;

	//! Renders the Canvas to the given Surface in an accelerated manner
	/*!	\param context		Context iterator referring to next Layer.
	**	\param surface		Pointer to Surface to render to.
	**	\param quality		The requested quality-level to render at.
	**	\param renddesc		The associated RendDesc.
	**	\param cb			Pointer to callback object. May be NULL if there is no callback.
	**	\return \c true on success, \c false on failure
	**	\see Handle::accelerated_render()
	*/
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	//! Checks to see if a part of the layer is directly under \a point
	/*!	\param context		Context iterator referring to next Layer.
	**	\param point		The point to check
	**	\return 	The handle of the layer under \a point. If there is not
	**				a layer under \a point, then returns an empty handle. */
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

protected:

	//! This is called whenever a parameter is changed
	virtual void on_changed();

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
