/* === S Y N F I G ========================================================= */
/*!	\file dock_timetrack.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2010 Carlos López
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
#include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>

#include <gtkmm/drawingarea.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>

#include <synfig/general.h>
#include <synfig/timepointcollect.h>

#include <app.h>
#include <canvasview.h>
#include <gui/helpers.h>
#include <instance.h>
#include <synfigapp/canvasinterface.h>
#include <trees/layerparamtreestore.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <trees/layertree.h>
#include <widgets/widget_canvastimeslider.h>
#include <widgets/widget_keyframe_list.h>
#include <widgets/widget_timeslider.h>
#include <workarea.h>

#include "dock_timetrack.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === C L A S S E S ======================================================= */

static void calc_divisions(float fps, double range, double sub_range, double &out_step, int &out_subdivisions)
{
  int ifps = etl::round_to_int(fps);
  if (ifps < 1)
    ifps = 1;

  // build a list of all the factors of the frame rate
  int pos = 0;
  std::vector<double> ranges;
  for (int i = 1; i * i <= ifps; i++)
    if (ifps % i == 0) {
      ranges.insert(ranges.begin() + pos, i / fps);
      if (i * i != ifps)
        ranges.insert(ranges.begin() + pos + 1, ifps / i / fps);
      pos++;
    }

  { // fill in any gaps where one factor is more than 2 times the previous
    std::vector<double>::iterator iter, next;
    pos = 0;
    for (int pos = 0; pos < (int)ranges.size() - 1; pos++) {
      next = ranges.begin() + pos;
      iter = next++;
      if (*iter * 2 < *next)
        ranges.insert(next, *iter * 2);
    }
  }

  double more_ranges[] = {2,         3,          5,          10,         20,         30,        60,
                          90,        120,        180,        300,        600,        1200,      1800,
                          2700,      3600,       3600 * 2,   3600 * 4,   3600 * 8,   3600 * 16, 3600 * 32,
                          3600 * 64, 3600 * 128, 3600 * 256, 3600 * 512, 3600 * 1024};
  ranges.insert(ranges.end(), more_ranges, more_ranges + sizeof(more_ranges) / sizeof(double));

  double mid_range = (range + sub_range) / 2;

  // find most ideal scale
  double scale;
  {
    std::vector<double>::iterator next = etl::binary_find(ranges.begin(), ranges.end(), mid_range);
    std::vector<double>::iterator iter = next++;
    if (iter == ranges.end())
      iter--;
    if (next == ranges.end())
      next--;
    if (fabs(*next - mid_range) < fabs(*iter - mid_range))
      iter = next;
    scale = *iter;
  }

  // subdivide into this many tick marks (8 or less)
  int subdiv = etl::round_to_int(scale * ifps);
  if (subdiv > 8) {
    const int ideal = subdiv;

    // find a number of tick marks that nicely divides the scale
    // (5 minutes divided by 6 is 50s, but that's not 'nice' -
    //  5 ticks of 1m each is much simpler than 6 ticks of 50s)
    for (subdiv = 8; subdiv > 0; subdiv--)
      if ((ideal <= ifps * 2 && (ideal % (subdiv)) == 0) ||
          (ideal <= ifps * 2 * 60 && (ideal % (subdiv * ifps)) == 0) ||
          (ideal <= ifps * 2 * 60 * 60 && (ideal % (subdiv * ifps * 60)) == 0) ||
          (true && (ideal % (subdiv * ifps * 60 * 60)) == 0))
        break;

    // if we didn't find anything, use 4 ticks
    if (!subdiv)
      subdiv = 4;
  }

  out_step = scale;
  out_subdivisions = subdiv;
}

/* ------T  I  M  E  T  R  A  C  K  V  I  E  W----------------------------------------------------*/

class TimeTrackView : public Gtk::DrawingArea
{
  sigc::connection time_changed;

  etl::handle<TimeModel> time_model;

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;


protected:
  synfig::Time step;

  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
  {
    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(0, 0, get_width(), get_height());
    cr->fill();
    if (!time_model || get_width() <= 0 || get_height() <= 0)
      return true;

    // Get the time information since we now know it's valid
    Time time = time_model->get_time();
    Time lower = time_model->get_visible_lower();
    Time upper = time_model->get_visible_upper();

    if (lower >= upper)
      return true;
    double k = (double)get_width() / (double)(upper - lower);

    Time extra_time = (double)get_height() / k;
    Time lower_ex = lower - extra_time;
    Time upper_ex = upper + extra_time;

    double big_step_value;
    int subdivisions;
    calc_divisions(time_model->get_frame_rate(), 140.0 / k, 280.0 / k, big_step_value, subdivisions);

    step = time_model->round_time(Time(big_step_value / (double)subdivisions));
    step = std::max(time_model->get_step_increment(), step);

    Time big_step = step * (double)subdivisions;
    Time current = big_step * floor((double)lower_ex / (double)big_step);
    current = time_model->round_time(current);
    double y = etl::round_to_int((double)(current - lower) * k) + 0.5;
    // draw
    cr->save();
    cr->set_source_rgb(1, 0, 0);
    cr->set_line_width(1.0);
    for (int i = 0; current <= upper_ex; ++i, current = time_model->round_time(current + step)) {
      double x = etl::round_to_int((double)(current - lower) * k) + 0.5;
      cr->move_to(x, 0.0);
      cr->line_to(x, get_height());
      cr->stroke();
    }
    cr->set_source_rgb(1, 1, 0);
    cr->set_line_width(1.0);
    cr->restore();
  }

public:
  const etl::handle<TimeModel> &get_time_model() const { return time_model; }
  void set_time_model(const etl::handle<TimeModel> &x)
{
	if (time_model == x) return;
	time_changed.disconnect();
	time_model = x;
	if (time_model)
		time_changed = time_model->signal_time_changed().connect(
			sigc::mem_fun(*this, &TimeTrackView::queue_draw) );
}
  void set_canvas_interface(const etl::loose_handle<synfigapp::CanvasInterface> &x) { canvas_interface = x; }


  TimeTrackView()
  {}

  // add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK);
};

Dock_Timetrack::Dock_Timetrack()
  : Dock_CanvasSpecific("timetrack", _("Timetrack"), Gtk::StockID("synfig-timetrack")), table_(), timetrackview_()
{
  set_use_scrolled(false);
}

Dock_Timetrack::~Dock_Timetrack()
{
  if (table_)
    delete table_;
}
void Dock_Timetrack::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	
  TimeTrackView *timetrackview(new TimeTrackView());
  timetrackview->set_time_model(canvas_view->time_model());
  timetrackview->set_canvas_interface(canvas_view->canvas_interface());
  studio::LayerTree *tree_layer(dynamic_cast<studio::LayerTree *>(canvas_view->get_ext_widget("layers_cmp")));
  tree_layer->signal_param_tree_header_height_changed().connect(
      sigc::mem_fun(*this, &studio::Dock_Timetrack::on_update_header_height));
  canvas_view->set_ext_widget(get_name(), timetrackview);
}
void Dock_Timetrack::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
  if (table_) {
    delete table_;
    table_ = 0;

    hscrollbar_.unset_adjustment();
    vscrollbar_.unset_adjustment();

    widget_timeslider_.set_canvas_view(CanvasView::Handle());

    widget_kf_list_.set_time_model(etl::handle<TimeModel>());
    widget_kf_list_.set_canvas_interface(etl::loose_handle<synfigapp::CanvasInterface>());
  }

  if (canvas_view) {
    timetrackview_ = dynamic_cast<TimeTrackView *>(canvas_view->get_ext_widget(get_name()));
    hscrollbar_.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

    widget_timeslider_.set_canvas_view(canvas_view);

    widget_kf_list_.set_time_model(canvas_view->time_model());
    widget_kf_list_.set_canvas_interface(canvas_view->canvas_interface());

    table_ = new Gtk::Table(3, 2);
    table_->attach(widget_kf_list_, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
    table_->attach(widget_timeslider_, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
    table_->attach(*timetrackview_, 0, 1, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
    table_->attach(hscrollbar_, 0, 1, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
    table_->attach(vscrollbar_, 1, 2, 0, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
    add(*table_);

    table_->show_all();
    show_all();
  } else {
    // clear_previous();
  }
}
void Dock_Timetrack::on_update_header_height(int header_height)
{
  int width = 0;
  int height = 0;
  int kf_list_height = 8;
  widget_timeslider_.set_size_request(-1, height - kf_list_height);
  widget_kf_list_.set_size_request(-1, kf_list_height);
}
