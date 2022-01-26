/* === S Y N F I G ========================================================= */
/*!	\file duckmatic.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Nikita Kitaev
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DUCKMATIC_H
#define __SYNFIG_STUDIO_DUCKMATIC_H

/* === H E A D E R S ======================================================= */

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <gui/duck.h>

#include <list>
#include <map>
#include <set>
#include <sigc++/sigc++.h>

#include <synfig/vector.h>
#include <synfig/string.h>
#include <synfig/real.h>
#include <synfig/time.h>
#include <synfig/color.h>
#include <synfig/guidset.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp { class ValueDesc; class CanvasInterface; }
namespace synfig { class ParamDesc; }

namespace studio
{

class CanvasView;
class Duckmatic;

class DuckDrag_Base : public etl::shared_object
{
public:
	virtual void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin)=0;
	virtual bool end_duck_drag(Duckmatic* duckmatic)=0;
	virtual void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)=0;
};

class DuckDrag_Translate : public DuckDrag_Base
{
	synfig::Vector last_translate_;
	synfig::Vector drag_offset_;
	synfig::Vector snap_offset_;
	std::vector<synfig::Vector> positions;
	bool is_moving = false;

public:
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);
};

class BezierDrag_Base : public etl::shared_object
{
public:
	virtual void begin_bezier_drag(Duckmatic* duckmatic, const synfig::Vector& begin, float bezier_click_pos)=0;
	virtual bool end_bezier_drag(Duckmatic* duckmatic)=0;
	virtual void bezier_drag(Duckmatic* duckmatic, const synfig::Vector& vector)=0;
};

class BezierDrag_Default : public BezierDrag_Base
{
	synfig::Vector last_translate_;
	synfig::Vector drag_offset_;
	float click_pos_;
	synfig::Vector c1_initial;
	synfig::Vector c2_initial;
	float c1_ratio;
	float c2_ratio;
	//bool c1_selected;
	//bool c2_selected;
	//Warning: unused variables c1_selected c2_selected
	bool is_moving;

public:
	void begin_bezier_drag(Duckmatic* duckmatic, const synfig::Vector& begin, float bezier_click_pos);
	bool end_bezier_drag(Duckmatic* duckmatic);
	void bezier_drag(Duckmatic* duckmatic, const synfig::Vector& vector);
};

/*! \class Duckmatic
**
**	This class helps organize any of the devices displayed in
**	the work area that the user may want to interact with.
**	This includes ducks, beziers, and strokes
**
*/
class Duckmatic
{
	friend class DuckDrag_Base;
	friend class DuckDrag_Translate;

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef std::map<synfig::GUID,etl::smart_ptr<synfig::Point> > DuckDataMap;

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

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;

	Type type_mask, type_mask_state;

	DuckMap duck_map;

	DuckDataMap duck_data_share_map;

	std::list<etl::handle<Stroke> > stroke_list_;

	std::list<etl::handle<Stroke> > persistent_stroke_list_;

	synfig::GUIDSet selected_ducks;

	synfig::GUID last_duck_guid;

	std::list<etl::handle<Bezier> > bezier_list_;

	//! I cannot recall what this is for
	//synfig::Vector snap;

	etl::handle<DuckDrag_Base> duck_dragger_;

	etl::handle<BezierDrag_Base> bezier_dragger_;

	sigc::signal<void> signal_duck_selection_changed_;
	sigc::signal<void, const etl::handle<Duck>& > signal_duck_selection_single_;

	sigc::signal<void> signal_strokes_changed_;

	sigc::signal<void> signal_grid_changed_;

	mutable sigc::signal<void> signal_sketch_saved_;

	GuideList guide_list_x_;
	GuideList guide_list_y_;

	mutable synfig::String sketch_filename_;

	synfig::TransformStack curr_transform_stack;
	bool curr_transform_stack_set = false;
	std::list<sigc::connection> duck_changed_connections;

	bool alternative_mode_;
	bool lock_animation_mode_;

	/*
 -- ** -- P R O T E C T E D   D A T A -----------------------------------------
	*/

protected:

	etl::handle<Bezier> selected_bezier;

	synfig::Time cur_time;

	//! This flag is set if operations should snap to the grid
	/*! \todo perhaps there should be two of these flags, one for each axis?
	**	\see show_grid, grid_size */
	bool grid_snap;

	bool guide_snap;

	//! This vector describes the grid size.
	/*! \see grid_snap, show_grid */
	synfig::Vector grid_size;
	//! Hold the grid color.
	synfig::Color grid_color;
	//! Hold the guides color.
	synfig::Color guides_color;

	float zoom;					//!< Zoom factor
	float prev_zoom;			//!< Previous Zoom factor

	bool show_persistent_strokes;

	bool axis_lock;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	synfig::Vector last_translate_;
	synfig::Vector drag_offset_;

	//etl::handle<Duck> selected_duck;

	void connect_signals(const Duck::Handle &duck, const synfigapp::ValueDesc& value_desc, CanvasView &canvas_view);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	Duckmatic(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface);
	virtual ~Duckmatic();

	void set_alternative_mode(bool x) { alternative_mode_=x; }
	bool get_alternative_mode()const { return alternative_mode_; }

	void set_lock_animation_mode(bool x) { lock_animation_mode_=x; }
	bool get_lock_animation_mode()const { return lock_animation_mode_; }

	sigc::signal<void>& signal_duck_selection_changed() { return signal_duck_selection_changed_; }
	sigc::signal<void, const etl::handle<Duck>& >& signal_duck_selection_single() { return signal_duck_selection_single_; }
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
	//! Sets the color of the guides
	void set_guides_color(const synfig::Color &c);
	//! Returns the color of the guides
	const synfig::Color &get_guides_color()const { return guides_color;}

	//! Sets the state of the grid snap flag
	void set_grid_snap(bool x=true);

	//! Gets the state of the grid snap flag
	bool get_grid_snap()const { return grid_snap; }

	void enable_grid_snap() { set_grid_snap(true); }

	void disable_grid_snap() { set_grid_snap(false); }

	void toggle_grid_snap() { set_grid_snap(!grid_snap); }

	synfig::Point snap_point_to_grid(const synfig::Point& x)const;

	bool get_show_persistent_strokes()const { return show_persistent_strokes; }
	void set_show_persistent_strokes(bool x);

	//! Sets the size of the grid
	void set_grid_size(const synfig::Vector &s);
	//! Sets the color of the grid
	void set_grid_color(const synfig::Color &c);

	//! Returns the size of the grid
	const synfig::Vector &get_grid_size()const { return grid_size; }
	//! Returns the color of the grid
	const synfig::Color &get_grid_color()const { return grid_color;}

	const synfig::Time &get_time()const { return cur_time; }

	bool get_axis_lock()const { return axis_lock; }
	void set_axis_lock(bool x) { axis_lock=x; }

	void set_time(synfig::Time x) { cur_time=x; }

	bool is_duck_group_selectable(const etl::handle<Duck>& x)const;

	//const DuckMap& duck_map()const { return duck_map; }
	DuckList get_duck_list()const;

	const std::list<etl::handle<Bezier> >& bezier_list()const { return bezier_list_; }

	const std::list<etl::handle<Stroke> >& stroke_list()const { return stroke_list_; }

	const std::list<etl::handle<Stroke> >& persistent_stroke_list()const { return persistent_stroke_list_; }

	std::list<etl::handle<Stroke> >& persistent_stroke_list() { return persistent_stroke_list_; }

    /*
 -- ** -- D U C K  S E L E C T I O N  M E T H O D S----------------------------
    */

    //! Return first selected duck (handle) has const Duck etl::handle
	etl::handle<Duck> get_selected_duck()const;
	//! Return list of selected ducks (handles)
	/*!
     ** \return ducks (handles) has const DuckList
     ** \sa get_selected_duck, clear_selected_ducks, count_selected_ducks
    */
	DuckList get_selected_ducks()const;
    //! Return list of box contained ducks (handles). The box is defined by a vector's pair
    /*!
     ** \param tl The top left canvas coordinate has const synfig::Vector
     ** \param br The bottom right canvas coordinate has const synfig::Vector
     ** \return ducks (handles) has const DuckList
     ** \sa toggle_select_ducks_in_box, select_ducks_in_box, find_duck
    */
	DuckList get_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br)const;

	void refresh_selected_ducks();
    //! Clear all selected ducks
	void clear_selected_ducks();
	//! Return the number of selected ducks
    /*!
     ** \return the number of selected ducks (handles) has int
     */
	int count_selected_ducks()const;
    //! Give the selection status of a duck
    /*!
     ** \return \a true if the given duck (handle) is currently selected
     */
    bool duck_is_selected(const etl::handle<Duck> &duck)const;
	//! Toggle the duck (handle)
    /*!
     ** \param duck The duck (handle) to toggle has etl::handle parameter
     */
	void toggle_select_duck(const etl::handle<Duck> &duck);
    //! Select the duck (handle)
    /*!
     ** \param duck The duck (handle) to select has etl::handle parameter
     */
	void select_duck(const etl::handle<Duck> &duck);
    //! Unselect the duck (handle)
    /*!
     ** \param duck The duck (handle) to unselect has etl::handle parameter
     */
	void unselect_duck(const etl::handle<Duck> &duck);

    //! Toggle the ducks (handles) contained in the box defined by a pair of vectors
    /*!
     ** \param tl The top left canvas coordinate has const synfig::Vector
     ** \param br The bottom right canvas coordinate has const synfig::Vector
     ** \sa toggle_select_duck, select_ducks_in_box, get_ducks_in_box
    */
	void toggle_select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br);
	//! Select the ducks (handles) contained in the box defined by a pair of vectors
    /*!
     ** \param tl The top left canvas coordinate has const synfig::Vector
     ** \param br The bottom right canvas coordinate has const synfig::Vector
     ** \sa toggle_select_ducks_in_box, select_ducks_in_box, clear_selected_ducks
    */
	void select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br);


	const synfig::TransformStack& get_curr_transform_stack()const { return curr_transform_stack; }

	inline void clear_curr_transform_stack() { curr_transform_stack.clear(); curr_transform_stack_set=false; }


	etl::handle<Bezier> get_selected_bezier()const;

	//! Begin dragging ducks
	/*!
	 ** \param offset Canvas coordinates of the mouse when the drag began
	*/
	void start_duck_drag(const synfig::Vector& offset);

	//! Continue dragging the selected ducks
	/*! The overall vector of the drag is vector-offset
	 ** (where offset was given in start_duck_drag)
	 ** \param vector Canvas coordinates of the mouse at this moment
	*/
	void translate_selected_ducks(const synfig::Vector& vector);

	//! Update the coordinates of tangents and linked-to-bline ducks
	void update_ducks();

	//! Ends the duck drag
	bool end_duck_drag();

	//! \todo writeme
	// bezier drags (similar to duck drags)
	void start_bezier_drag(const synfig::Vector& offset, float bezier_click_pos);

	void translate_selected_bezier(const synfig::Vector& vector);

	bool end_bezier_drag();


	//! Signals to each selected duck that it has been clicked
	void signal_user_click_selected_ducks(int button);

	//! Calls a single duck's edited signal
	/*! Updates the corresponding valuenodes after a drag */
	void signal_edited_duck(const etl::handle<Duck> &duck, bool moving = false);

	//! Calls all of the ducks' edited signals
	/*! Updates corresponding valuenodes after a drag */
	void signal_edited_selected_ducks(bool moving = false);

	bool on_duck_changed(const studio::Duck &duck,const synfigapp::ValueDesc& value_desc);

	etl::handle<Duck> find_similar_duck(etl::handle<Duck> duck);
	etl::handle<Duck> add_similar_duck(etl::handle<Duck> duck);

	void add_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color=synfig::Color(0,0,0));

	void add_persistent_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color=synfig::Color(0,0,0));

	void clear_persistent_strokes();

	void add_duck(const etl::handle<Duck> &duck);

	void add_bezier(const etl::handle<Bezier> &bezier);

	void erase_duck(const etl::handle<Duck> &duck);

	void erase_bezier(const etl::handle<Bezier> &bezier);

	//! Returns the last duck added
	etl::handle<Duck> last_duck()const;

	etl::handle<Bezier> last_bezier()const;

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	etl::handle<Duck> find_duck(synfig::Point pos, synfig::Real radius=0, Duck::Type type=Duck::TYPE_DEFAULT);

	GuideList::iterator find_guide_x(synfig::Point pos, float radius=0.1);
	GuideList::iterator find_guide_y(synfig::Point pos, float radius=0.1);
	GuideList::const_iterator find_guide_x(synfig::Point pos, float radius=0.1)const { return const_cast<Duckmatic*>(this)->find_guide_x(pos,radius); }
	GuideList::const_iterator find_guide_y(synfig::Point pos, float radius=0.1)const { return const_cast<Duckmatic*>(this)->find_guide_y(pos,radius); }

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	//etl::handle<Bezier> find_bezier(synfig::Point pos, synfig::Real radius=0);

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	etl::handle<Bezier> find_bezier(synfig::Point pos, synfig::Real radius=0, float* location=0);

	etl::handle<Bezier> find_bezier(synfig::Point pos, synfig::Real scale, synfig::Real radius, float* location=0);

	//! if transform_count is set function will not restore transporm stack
	void add_ducks_layers(synfig::Canvas::Handle canvas, std::set<synfig::Layer::Handle>& selected_layer_set, etl::handle<CanvasView> canvas_view, synfig::TransformStack& transform_stack, int *transform_count = NULL);

	bool add_to_ducks(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack_, synfig::ParamDesc *param_desc=0);

	//! Set the type mask, which determines what types of ducks are shown
	//! \Param[in]   x   Duck::Type set to backup when toggling handles
	//! \Sa              get_type_mask(), CanvasView::toggle_duck_all()
	void set_type_mask(Type x) { type_mask=x; }
	//! Get the type mask, which determines what types of ducks are shown
	//! \Sa              set_type_mask(), CanvasView::toggle_duck_all()
	Type get_type_mask()const { return type_mask; }

	//! Set the type mask state, which determines what types of ducks are shown on toggle
	//! \Param[in]   x   Duck::Type set to backup when toggling handles
	//! \Sa              get_type_mask_state(), CanvasView::toggle_duck_mask_all()
	void set_type_mask_state(Type x) { type_mask_state=x; }
	//! Get the type mask state, which determines what types of ducks are shown on toggle
	//! \Sa              set_type_mask_state(), CanvasView::toggle_duck_mask_all()
	Type get_type_mask_state()const { return type_mask_state; }

	void select_all_ducks();
	void unselect_all_ducks();

	void clear_ducks();

	bool save_sketch(const synfig::String& filename)const;
	bool load_sketch(const synfig::String& filename);
	const synfig::String& get_sketch_filename()const { return sketch_filename_; }

	void set_duck_dragger(etl::handle<DuckDrag_Base> x) { duck_dragger_=x; }
	etl::handle<DuckDrag_Base> get_duck_dragger()const { return duck_dragger_; }
	void clear_duck_dragger() { duck_dragger_=new DuckDrag_Translate(); }


	void set_bezier_dragger(etl::handle<BezierDrag_Base> x) { bezier_dragger_=x; }
	etl::handle<BezierDrag_Base> get_bezier_dragger()const { return bezier_dragger_; }
	void clear_bezier_dragger() { bezier_dragger_=new BezierDrag_Default(); }
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
	sigc::signal<void,float> signal_user_doubleclick_[5];
public:

	etl::handle<Duck> p1,p2,c1,c2;
	bool is_valid()const { return p1 && p2 && c1 && c2; }

	sigc::signal<void,float> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }
	sigc::signal<void,float> &signal_user_doubleclick(int i=0) { assert(i>=0); assert(i<5); return signal_user_doubleclick_[i]; }
}; // END of struct Duckmatic::Bezier

/*! \struct Duckmatic::Stroke
**	\writeme */
struct Duckmatic::Stroke : public etl::shared_object
{
private:
	sigc::signal<void,float> signal_user_click_[5];
public:

	etl::smart_ptr<std::list<synfig::Point> > stroke_data;

	synfig::Color color;

	bool is_valid()const { return (bool)stroke_data; }

	sigc::signal<void,float> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }
}; // END of struct Duckmatic::Stroke

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
