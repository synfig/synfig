/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_eyedropper.h
**	\brief A "fake" widget to allow pick a color from display screen
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2021 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_WIDGET_EYEDROPPER_H
#define SYNFIG_STUDIO_WIDGET_EYEDROPPER_H

#include <gtkmm/drawingarea.h>

#if GTK_CHECK_VERSION(3, 20, 0)
#include <gdkmm/seat.h>
#else
#include <gdkmm/devicemanager.h>
#endif

namespace studio {

///
/// \brief A bogus widget to enable the eyedropper action
///
/// This widget displays nothing by itself, but it must be add to a Gtk::Window/Gtk::Container
/// and set as visible (eg. via Gtk::Widget::show()), otherwise it doesn't work.
///
/// To start eye-dropper action, call grab(). The mouse/pointer cursor changes
/// and user should unable to do any input event but to click on a pixel in the
/// entire display screen in order to fetch its color.
/// Besides pointer movement and clicking, only ESC key pressing is enabled.
/// Programatically, it can be cancelled by calling ungrab().
///
/// Once user releases the left mouse button on a pixel - wherever it is -, the
/// user input works back to normal, and the signal signal_color_picked() is emitted.
///
/// The picked color can be retrieved via the signal_color_picked() or get_color().
///
class Widget_Eyedropper : public Gtk::DrawingArea
{
	void init();
public:
	Widget_Eyedropper();
	virtual ~Widget_Eyedropper() override;

	//! signal emitted when user picks a color from screen
	sigc::signal<void, const Gdk::RGBA&>& signal_color_picked() { return signal_color_picked_; }
	//! signal emitted when user cancels eye-dropping (by pressing ESC) or grab is broken
	//! \see Widget::signal_grab_broken_event()
	sigc::signal<void>& signal_cancelled() { return signal_cancelled_; }

	//! The color picked by user in last grab session. Full transparent "black" if not picked yet or cancelled.
	Gdk::RGBA get_color() const;

	//! Start the eyedropper session
	//! Every regular user pointer and keyboard input is disabled except:
	//! - mouse moving, for letting user moves to the pixel :P
	//! - mouse button clicking, for letting user selects the pixel :P
	//! - ESC key pressing, for cancel the eyedropper
	//! User isn't able to interact with other open apps until eydropper session ending
	void grab();
	//! Programatically cancels the eydropper session, without any picked color
	void ungrab();

	//! Programatically picks the color of a pixel, in screen coordinates
	void pick_color_at(int x, int y);

protected:
	virtual bool on_key_press_event(GdkEventKey *key_event) override;
	virtual bool on_key_release_event(GdkEventKey *key_event) override;
	virtual bool on_button_release_event(GdkEventButton *button_event) override;
	bool on_grab_broken_event(GdkEventGrabBroken * /*grab_event*/);

private:
#if GTK_CHECK_VERSION(3, 20, 0)
	//! Grabbed seat while in eyedropper session
	Glib::RefPtr<Gdk::Seat> seat;
#else
	//! Grabbed devices while in eyedropper session
	std::vector<Glib::RefPtr<Gdk::Device>> grabbed_devices;
#endif
	//! The picked color
	Gdk::RGBA color;

	///! \see signal_color_picked()
	sigc::signal<void, const Gdk::RGBA&> signal_color_picked_;
	///! \see signal_color_cancelled()
	sigc::signal<void> signal_cancelled_;

	//! If widget is grabbing user input events
	//! Not thread-safe. Should it be?
	bool should_ungrab;

#if not GTK_CHECK_VERSION(3, 20, 0)
	static Glib::RefPtr<Gdk::Device> get_pointer();
	static std::vector<Glib::RefPtr<Gdk::Device>> get_keyboards();
#endif

// Glade & GtkBuilder related
public:
	Widget_Eyedropper(BaseObjectType* cobject);
	static Glib::ObjectBase* wrap_new(GObject* o);
	static void register_type();
private:
	static GType gtype;
};

}
#endif // SYNFIG_STUDIO_WIDGET_EYEDROPPER_H
