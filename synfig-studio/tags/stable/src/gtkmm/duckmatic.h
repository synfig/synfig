/* === S I N F G =========================================================== */
/*!	\file duckmatic.h
**	\brief Template Header
**
**	$Id: duckmatic.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_DUCKMATIC_H
#define __SINFG_STUDIO_DUCKMATIC_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <map>
#include <set>

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <sinfg/vector.h>
#include <sinfg/string.h>
#include <sinfg/real.h>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <sinfg/time.h>
#include <sinfg/color.h>
#include <ETL/smart_ptr>

#include "duck.h"
#include <sinfg/color.h>
#include <sinfg/guidset.h>

/* === M A C R O S ========================================================= */

#define HASH_MAP_H <ext/hash_map>

#ifdef HASH_MAP_H
#include HASH_MAP_H
#ifndef __STRING_HASH__
#define __STRING_HASH__
class StringHash
{
	__gnu_cxx::hash<const char*> hasher_;
public:
	size_t operator()(const sinfg::String& x)const
	{
		return hasher_(x.c_str());
	}
};
#endif
#else
#include <map>
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp { class ValueDesc; }
namespace sinfg { class ParamDesc; }

namespace studio
{

class CanvasView;
class Duckmatic;
	
class DuckDrag_Base : public etl::shared_object
{
public:
	virtual void begin_duck_drag(Duckmatic* duckmatic, const sinfg::Vector& begin)=0;
	virtual bool end_duck_drag(Duckmatic* duckmatic)=0;
	virtual void duck_drag(Duckmatic* duckmatic, const sinfg::Vector& vector)=0;
};

class DuckDrag_Translate : public DuckDrag_Base
{
	sinfg::Vector last_translate_;
	sinfg::Vector drag_offset_;
	sinfg::Vector snap;
	std::vector<sinfg::Vector> positions;

public:
	void begin_duck_drag(Duckmatic* duckmatic, const sinfg::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const sinfg::Vector& vector);
};

/*! \class Duckmatic
**
**	This class helps organize any of the devices displayed in
**	the work area that the user may want to interact with.
**	This includes ducks, beziers, and strokes
**
**	\note At some point I'll probably rename this class to "DuckOMatic".
*/
class Duckmatic
{
	friend class DuckDrag_Base;
	friend class DuckDrag_Translate;
	
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/
	
public:

#ifdef HASH_MAP_H
typedef __gnu_cxx::hash_map<sinfg::GUID,etl::smart_ptr<sinfg::Point>,sinfg::GUIDHash> DuckDataMap;
#else
typedef std::map<sinfg::GUID,etl::smart_ptr<sinfg::Point> > DuckDataMap;
#endif

	typedef studio::DuckMap DuckMap;

	typedef studio::Duck Duck;

	struct Stroke;

	struct Bezier;

	class Push;
	
	friend class Push;
		
	typedef Duck::Type Type;
	
	typedef std::list<float> GuideList;
	
	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Type type_mask;

	DuckMap duck_map;

	DuckDataMap duck_data_share_map;

	std::list<etl::handle<Stroke> > stroke_list_;

	std::list<etl::handle<Stroke> > persistant_stroke_list_;

	sinfg::GUIDSet selected_ducks;

	sinfg::GUID last_duck_guid;

	std::list<etl::handle<Bezier> > bezier_list_;	

	//! I cannot recall what this is for
	//sinfg::Vector snap;

	etl::handle<DuckDrag_Base> duck_dragger_;

	sigc::signal<void> signal_duck_selection_changed_;

	sigc::signal<void> signal_strokes_changed_;

	sigc::signal<void> signal_grid_changed_;

	mutable sigc::signal<void> signal_sketch_saved_;
	
	GuideList guide_list_x_;
	GuideList guide_list_y_;

	mutable sinfg::String sketch_filename_;
	
	/*
 -- ** -- P R O T E C T E D   D A T A -----------------------------------------
	*/

protected:

	etl::handle<Bezier> selected_bezier;

	sinfg::Time cur_time;

	//! This flag is set if operations should snap to the grid
	/*! \todo perhaps there should be two of these flags, one for each axis?
	**	\see show_grid, grid_size */
	bool grid_snap;

	bool guide_snap;

	//! This vector describes the grid size.
	/*! \see grid_snap, show_grid */
	sinfg::Vector grid_size;

	bool show_persistant_strokes;

	bool axis_lock;
	
	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	
	sinfg::Vector last_translate_;
	sinfg::Vector drag_offset_;
	
	//etl::handle<Duck> selected_duck;


	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	Duckmatic();
	virtual ~Duckmatic();

	sigc::signal<void>& signal_duck_selection_changed() { return signal_duck_selection_changed_; }
	sigc::signal<void>& signal_strokes_changed() { return signal_strokes_changed_; }
	sigc::signal<void>& signal_grid_changed() { return signal_grid_changed_; }
	sigc::signal<void>& signal_sketch_saved() { return signal_sketch_saved_; }

	GuideList& get_guide_list_x() { return guide_list_x_; }
	GuideList& get_guide_list_y() { return guide_list_y_; }
	const GuideList& get_guide_list_x()const { return guide_list_x_; }
	const GuideList& get_guide_list_y()const { return guide_list_y_; }

	void set_guide_snap(bool x=true);
	bool get_guide_snap()const { return guide_snap; }
	void toggle_guide_snap() { set_guide_snap(!get_guide_snap()); }
	
	//! Sets the state of the grid snap flag
	void set_grid_snap(bool x=true);
	
	//! Gets the state of the grid snap flag
	bool get_grid_snap()const { return grid_snap; }
	
	void enable_grid_snap() { set_grid_snap(true); }

	void disable_grid_snap() { set_grid_snap(false); }

	void toggle_grid_snap() { set_grid_snap(!grid_snap); }

	sinfg::Point snap_point_to_grid(const sinfg::Point& x, float radius=0.1)const;
	
	bool get_show_persistant_strokes()const { return show_persistant_strokes; }
	void set_show_persistant_strokes(bool x);

	//! Sets the size of the grid
	void set_grid_size(const sinfg::Vector &s);
	
	//! Returns the size of the grid
	const sinfg::Vector &get_grid_size()const { return grid_size; }
	
	
	const sinfg::Time &get_time()const { return cur_time; }

	bool get_axis_lock()const { return axis_lock; }
	void set_axis_lock(bool x) { axis_lock=x; }
	
	void set_time(sinfg::Time x) { cur_time=x; }

	bool is_duck_group_selectable(const etl::handle<Duck>& x)const;
	
	//const DuckMap& duck_map()const { return duck_map; }	
	DuckList get_duck_list()const;
	
	const std::list<etl::handle<Bezier> >& bezier_list()const { return bezier_list_; }	

	const std::list<etl::handle<Stroke> >& stroke_list()const { return stroke_list_; }

	const std::list<etl::handle<Stroke> >& persistant_stroke_list()const { return persistant_stroke_list_; }

	std::list<etl::handle<Stroke> >& persistant_stroke_list() { return persistant_stroke_list_; }

	//! \todo We should modify this to support multiple selections
	etl::handle<Duck> get_selected_duck()const;

	DuckList get_selected_ducks()const;
	
	//! Returns \a true if the given duck is currently selected
	bool duck_is_selected(const etl::handle<Duck> &duck)const;


	void refresh_selected_ducks();
	
	void clear_selected_ducks();

	int count_selected_ducks()const;

	void toggle_select_duck(const etl::handle<Duck> &duck);

	void select_duck(const etl::handle<Duck> &duck);
	
	void select_ducks_in_box(const sinfg::Vector& tl,const sinfg::Vector& br);

	void unselect_duck(const etl::handle<Duck> &duck);

	void start_duck_drag(const sinfg::Vector& offset);
	void translate_selected_ducks(const sinfg::Vector& vector);	
	bool end_duck_drag();

	void signal_edited_selected_ducks();	

	void signal_user_click_selected_ducks(int button);	

	
	etl::handle<Duck> find_similar_duck(etl::handle<Duck> duck);
	etl::handle<Duck> add_similar_duck(etl::handle<Duck> duck);
	
	void add_stroke(etl::smart_ptr<std::list<sinfg::Point> > stroke_point_list, const sinfg::Color& color=sinfg::Color(0,0,0));

	void add_persistant_stroke(etl::smart_ptr<std::list<sinfg::Point> > stroke_point_list, const sinfg::Color& color=sinfg::Color(0,0,0));

	void clear_persistant_strokes();

	void add_duck(const etl::handle<Duck> &duck);

	void add_bezier(const etl::handle<Bezier> &bezier);

	void erase_duck(const etl::handle<Duck> &duck);

	void erase_bezier(const etl::handle<Bezier> &bezier);

	//! Returns the last duck added
	etl::handle<Duck> last_duck()const;

	etl::handle<Bezier> last_bezier()const;
	
	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	etl::handle<Duck> find_duck(sinfg::Point pos, sinfg::Real radius=0, Duck::Type type=Duck::TYPE_DEFAULT);

	GuideList::iterator find_guide_x(sinfg::Point pos, float radius=0.1);
	GuideList::iterator find_guide_y(sinfg::Point pos, float radius=0.1);
	GuideList::const_iterator find_guide_x(sinfg::Point pos, float radius=0.1)const { return const_cast<Duckmatic*>(this)->find_guide_x(pos,radius); }
	GuideList::const_iterator find_guide_y(sinfg::Point pos, float radius=0.1)const { return const_cast<Duckmatic*>(this)->find_guide_y(pos,radius); }
	
	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	//etl::handle<Bezier> find_bezier(sinfg::Point pos, sinfg::Real radius=0);

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	etl::handle<Bezier> find_bezier(sinfg::Point pos, sinfg::Real radius=0, float* location=0);
	
	etl::handle<Bezier> find_bezier(sinfg::Point pos, sinfg::Real scale, sinfg::Real radius, float* location=0);
	
	bool add_to_ducks(const sinfgapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const sinfg::TransformStack& transform_stack_, sinfg::ParamDesc *param_desc=0, int multiple=0);

	//! \writeme
	void set_type_mask(Type x) { type_mask=x; }

	//! \writeme
	Type get_type_mask()const { return type_mask; }

	void select_all_ducks();

	void clear_ducks();
	
	bool save_sketch(const sinfg::String& filename)const;
	bool load_sketch(const sinfg::String& filename);
	const sinfg::String& get_sketch_filename()const { return sketch_filename_; }

	void set_duck_dragger(etl::handle<DuckDrag_Base> x) { duck_dragger_=x; }
	etl::handle<DuckDrag_Base> get_duck_dragger()const { return duck_dragger_; }
	void clear_duck_dragger() { duck_dragger_=new DuckDrag_Translate(); }
}; // END of class Duckmatic


/*! \class Duckmatic::Push
**	\writeme */
class Duckmatic::Push
{
	Duckmatic *duckmatic_;
	DuckMap duck_map;	
	std::list<etl::handle<Bezier> > bezier_list_;
	std::list<etl::handle<Stroke> > stroke_list_;
	DuckDataMap duck_data_share_map;
	etl::handle<DuckDrag_Base> duck_dragger_;
	
	bool needs_restore;
	
public:
	Push(Duckmatic *duckmatic_);
	~Push();
	void restore();
}; // END of class Duckmatic::Push

/*! \struct Duckmatic::Bezier
**	\writeme */
struct Duckmatic::Bezier : public etl::shared_object
{
private:
	sigc::signal<void,float> signal_user_click_[5];
public:
	
	etl::handle<Duck> p1,p2,c1,c2;
	bool is_valid()const { return p1 && p2 && c1 && c2; }

	sigc::signal<void,float> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }
}; // END of struct Duckmatic::Bezier

/*! \struct Duckmatic::Stroke
**	\writeme */
struct Duckmatic::Stroke : public etl::shared_object
{
private:
	sigc::signal<void,float> signal_user_click_[5];
public:
	
	etl::smart_ptr<std::list<sinfg::Point> > stroke_data;

	sinfg::Color color;

	bool is_valid()const { return (bool)stroke_data; }

	sigc::signal<void,float> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }
}; // END of struct Duckmatic::Bezier

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
