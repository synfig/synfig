/* === S I N F G =========================================================== */
/*!	\file dialog_setup.h
**	\brief Template Header
**
**	$Id: dialog_setup.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DIALOG_SETUP_H
#define __SINFG_STUDIO_DIALOG_SETUP_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/checkbutton.h>

#include <sinfg/gamma.h>
#include <sinfg/time.h>
#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class GammaPattern : public Gtk::DrawingArea
{
	float gamma_r;
	float gamma_g;
	float gamma_b;
	float black_level;
	float red_blue_level;

	int tile_w, tile_h;

	Gdk::Color black[4],white[4],gray50[4],gray25[4];

	float r_F32_to_F32(float x)const { float f((pow(x,gamma_r)*std::min(red_blue_level,1.0f)*(1.0f-black_level)+black_level)); if(f<0)f=0; if(f>1)f=1; return f; }
	float g_F32_to_F32(float x)const { float f((pow(x,gamma_g)*sqrt(std::min(2.0f-red_blue_level,red_blue_level))*(1.0f-black_level)+black_level)); if(f<0)f=0; if(f>1)f=1; return f; }
	float b_F32_to_F32(float x)const { float f((pow(x,gamma_b)*std::min(2.0f-red_blue_level,1.0f)*(1.0f-black_level)+black_level)); if(f<0)f=0; if(f>1)f=1; return f; }
	
public:
	
	void refresh();
	
	void set_gamma_r(float x) { gamma_r=x; }
	void set_gamma_g(float x) { gamma_g=x; };
	void set_gamma_b(float x) { gamma_b=x; };
	void set_black_level(float x) { black_level=x; };
	void set_red_blue_level(float x) { red_blue_level=x; };

	float get_gamma_r()const { return gamma_r; }
	float get_gamma_g()const { return gamma_g; }
	float get_gamma_b()const { return gamma_b; }
	float get_black_level()const { return black_level; }
	float get_red_blue_level()const { return red_blue_level; }
	
	GammaPattern();
	
	~GammaPattern();

	bool redraw(GdkEventExpose*bleh=NULL);
}; // END of class GammaPattern

class BlackLevelSelector : public Gtk::DrawingArea
{
	float level;

	sigc::signal<void> signal_value_changed_;

public:
	
	BlackLevelSelector();
	
	~BlackLevelSelector();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	
	void set_value(float x) { level=x; queue_draw(); }

	const float &get_value()const { return level; }	

	bool redraw(GdkEventExpose*bleh=NULL);

	bool on_event(GdkEvent *event);
}; // END of class BlackLevelSelector

class RedBlueLevelSelector : public Gtk::DrawingArea
{
	float level;

	sigc::signal<void> signal_value_changed_;

public:
	
	RedBlueLevelSelector();
	
	~RedBlueLevelSelector();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	
	void set_value(float x) { level=x; queue_draw(); }

	const float &get_value()const { return level; }	

	bool redraw(GdkEventExpose*bleh=NULL);

	bool on_event(GdkEvent *event);
}; // END of class RedBlueSelector

class Widget_Enum;

class Dialog_Setup : public Gtk::Dialog
{
		
	void on_ok_pressed();
	void on_apply_pressed();

	void on_gamma_r_change();
	void on_gamma_g_change();
	void on_gamma_b_change();
	void on_black_level_change();
	void on_red_blue_level_change();

	GammaPattern gamma_pattern;	
	BlackLevelSelector black_level_selector;
	RedBlueLevelSelector red_blue_level_selector;
	Gtk::OptionMenu timestamp_optionmenu;
	
	Gtk::Adjustment adj_gamma_r;
	Gtk::Adjustment adj_gamma_g;
	Gtk::Adjustment adj_gamma_b;

	Gtk::Adjustment adj_recent_files;
	Gtk::Adjustment adj_undo_depth;

	Gtk::CheckButton toggle_use_colorspace_gamma;

	sinfg::Time::Format time_format;
	
	Gtk::Menu *timestamp_menu;
	Widget_Enum *widget_enum;
public:

	void set_time_format(sinfg::Time::Format time_format);

	const sinfg::Time::Format& get_time_format()const { return time_format; }
	
	Dialog_Setup();
	~Dialog_Setup();

    void refresh();

}; // END of Dialog_Waypoint

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
