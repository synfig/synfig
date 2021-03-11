/* === S Y N F I G ========================================================= */
/*!	\file target.h
**	\brief Target Class Implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2010 Diego Barrios Romero
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

#ifndef __SYNFIG_TARGET_H
#define __SYNFIG_TARGET_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <utility>

#include <sigc++/signal.h>

#include <ETL/handle>

#include "canvas.h"
#include "color.h"
#include "progresscallback.h"
#include "renddesc.h"
#include "string.h"
#include "targetparam.h"

/* === M A C R O S ========================================================= */

//! Defines various variables and the create method, common for all Targets.
//! To be used in the private part of the target class definition.
#define SYNFIG_TARGET_MODULE_EXT \
		public: static const char name__[], version__[], ext__[];\
		static Target* create (const char *filename, const synfig::TargetParam& p);

//! Sets the name of the target
#define SYNFIG_TARGET_SET_NAME(class,x) const char class::name__[]=x

//! Sets the primary file extension of the target
#define SYNFIG_TARGET_SET_EXT(class,x) const char class::ext__[]=x

//! Sets the version of the target
#define SYNFIG_TARGET_SET_VERSION(class,x) const char class::version__[]=x

//! Defines implementation of the create method for the target
//! \param filename The file name to be created by the target.
//! \param p The parameters passed to the target (bit rate and vcodec)
#define SYNFIG_TARGET_INIT(class)										\
	synfig::Target* class::create (const char *filename,				\
								   const synfig::TargetParam& p)		\
	{ return new class(filename, p); }

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Surface;
class CairoSurface;
class RendDesc;
class Canvas;
class ProgressCallback;
struct TargetParam;

enum TargetAlphaMode
{
	TARGET_ALPHA_MODE_KEEP,          // 0
	TARGET_ALPHA_MODE_FILL,          // 1
	TARGET_ALPHA_MODE_REDUCE,        // 2
	TARGET_ALPHA_MODE_EXTRACT        // 3
}; // END enum TargetAlphaMode

/*!	\class Target
**	\brief Used to produce rendered animations of the documents
**
*	It is the base class for all the target renderers. It defines the has a static Book
*	pointer class that is a map for the targets factory creators and the strings
*	of the extension that the renderer can understand. It allows to create the a
*	pointer to a particular renderer just by using the extension of the name of file
*	to import. Also it creates a virtual member render() that must be declared in
*	the inherited classes.
*/
class Target : public etl::shared_object
{
public:
	typedef etl::handle<Target> Handle;
	typedef etl::loose_handle<Target> LooseHandle;
	typedef etl::handle<const Target> ConstHandle;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	sigc::signal<void> signal_progress_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	sigc::signal<void>& signal_progress() { return signal_progress_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:
	//! Type that represents a pointer to a Target's constructor.
	/*! As a pointer to the constructor, it represents a "factory" of targets.
	**  Receives the output filename (including path) and the parameters of the target.
	*/
	typedef Target* (*Factory)(const char *filename, const TargetParam& p);

	struct BookEntry
	{
		Factory factory;
		String filename; ///< Output filename including path
		TargetParam target_param; ///< Target module parameters
	};

	//! Book of types of targets indexed by the name of the Target.
	typedef std::map<String,BookEntry> Book;

	//! Book of types of targets indexed by the file extension
	typedef std::map<String,String> ExtBook;

	//! Target Book, indexed by the target's name
	static Book* book_;

	//! Map of target names indexed by associated file extension
	static ExtBook* ext_book_;

	static Book& book();
	static ExtBook& ext_book();

	//! Initializes the Target module by creating a book of targets names
	//! and its creators
	static bool subsys_init();
	//! Stops the Target module by deleting the book and the extension book
	static bool subsys_stop();

	//! Adjusted Render description set by set_rend_desc()
	RendDesc desc;

	//! Canvas being rendered in this target module
	/*!
	 ** \sa set_canvas()
	 */
	etl::handle<Canvas> canvas;

	//! Render quality used for the render process of the target.
	int quality_;
	
	//! Tells how to handle alpha. Used by non alpha supported targets to decide if the background must be filled or not
	TargetAlphaMode alpha_mode;

	//! When set to true, the target doesn't sync to canvas time.
	bool avoid_time_sync_;
	
	//! The current frame being rendered
	int curr_frame_;

protected:
	//! Default constructor
	Target();

public:
	virtual ~Target() { }
	//! Gets the target quality
	int get_quality()const { return quality_; }
	//! Sets the target quality
	void set_quality(int q) { quality_=q; }
	//! Sets the target avoid time synchronization
	void set_avoid_time_sync(bool x=true) { avoid_time_sync_=x; }
	//! Gets the target avoid time synchronization
	bool get_avoid_time_sync()const { return avoid_time_sync_; }
	//! Tells how to handle alpha
	/*! Used by non alpha supported targets to decide if the background
	 ** must be filled or not
	 */
	TargetAlphaMode get_alpha_mode()const { return alpha_mode; }
	//! Sets how to handle alpha
	void set_alpha_mode(TargetAlphaMode x=TARGET_ALPHA_MODE_KEEP) { alpha_mode=x; }
	//! Sets the target canvas. Must be defined by derived targets
	virtual void set_canvas(etl::handle<Canvas> c);
	//! Gets the target canvas.
	const etl::handle<Canvas> &get_canvas()const { return canvas; }
	//! Gets the target particular render description
	RendDesc &rend_desc() { return desc; }
	//! Gets the target particular render description
	const RendDesc &rend_desc()const { return desc; }
	//! Sets the RendDesc for the Target to \a desc.
	/*!	If there are any parts of \a desc that the render target
	 ** is not capable of doing, the render target will adjust
	 ** \a desc to fit its needs.
	 ** \param d an RendDesc pointer.
	 ** \return true on success
	*/
	virtual bool set_rend_desc(RendDesc *d) { desc=*d; return true; }
	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL)=0;
	//! Initialization tasks of the derived target.
	/*!
	 ** \returns true if the initialization has no errors
	*/
	virtual bool init(ProgressCallback *cb=NULL) { (void)cb; return true; }

	//! Creates a new Target described by \a type, outputting to a file described by \a filename.
	static Handle create(const String &type, const String &filename,
						 const synfig::TargetParam& params);
	
	//!	Sets the time for the next frame at \a time
	/*! It modifies the curr_frame_ member which has to be set to zero when next_frame is called for the first time
	 ** \param time The time reference to be modified
	 **	\return The number of remaining frames to render
	 **	\sa curr_frame_
	*/
	virtual int	next_frame(Time& time);
}; // END of class Target

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
