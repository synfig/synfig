/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_eyedropper.cpp
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widgets/widget_eyedropper.h"

#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <synfig/general.h>

#endif

using namespace studio;

void Widget_Eyedropper::init()
{
	color = Gdk::RGBA("#00000000");
	should_ungrab = false;
	add_events(Gdk::BUTTON_RELEASE_MASK | Gdk::KEY_RELEASE_MASK | Gdk::KEY_PRESS_MASK);
	signal_grab_broken_event().connect(sigc::mem_fun(*this, &Widget_Eyedropper::on_grab_broken_event));
}

Widget_Eyedropper::Widget_Eyedropper()
	: Glib::ObjectBase("widget_eyedropper")
{
	init();
}

Widget_Eyedropper::Widget_Eyedropper(BaseObjectType* cobject)
	: Glib::ObjectBase("widget_eyedropper"),
	  Gtk::DrawingArea(cobject)
{
	init();
}

Widget_Eyedropper::~Widget_Eyedropper()
{
	if (should_ungrab)
		ungrab();
}

Gdk::RGBA Widget_Eyedropper::get_color() const
{
	return color;
}

void Widget_Eyedropper::grab()
{
	if (should_ungrab || !get_window())
		return;
	should_ungrab = true;
	set_can_focus(true);
	grab_focus();

	Glib::RefPtr<Gdk::Cursor> cursor = Gdk::Cursor::create(Gdk::CursorType::CROSSHAIR);

#if GTK_CHECK_VERSION(3, 20, 0)
	seat = Gdk::Display::get_default()->get_default_seat();
	seat->grab(get_window(), Gdk::SEAT_CAPABILITY_ALL, true, cursor);

	gtk_device_grab_add(GTK_WIDGET(this->gobj()), seat->get_pointer()->gobj(), true);
	gtk_device_grab_add(GTK_WIDGET(this->gobj()), seat->get_keyboard()->gobj(), true);
#else
	grabbed_devices = get_keyboards();
	grabbed_devices.push_back(get_pointer());

	for (auto &device : grabbed_devices) {
		device->grab(get_window(), Gdk::GrabOwnership::OWNERSHIP_APPLICATION, true,
					 Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::KEY_RELEASE_MASK | Gdk::KEY_PRESS_MASK,
					 cursor, GDK_CURRENT_TIME);
		gtk_device_grab_add(GTK_WIDGET(this->gobj()), device->gobj(), true);
	}
#endif
}

void Widget_Eyedropper::ungrab()
{
	if (!should_ungrab)
		return;
#if GTK_CHECK_VERSION(3, 20, 0)
	seat->ungrab();
	gtk_device_grab_remove(GTK_WIDGET(this->gobj()), seat->get_pointer()->gobj());
	gtk_device_grab_remove(GTK_WIDGET(this->gobj()), seat->get_keyboard()->gobj());
#else
	for (auto &device : grabbed_devices) {
		device->ungrab(GDK_CURRENT_TIME);
		gtk_device_grab_remove(GTK_WIDGET(this->gobj()), device->gobj());
	}
	grabbed_devices.clear();
#endif
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window)
		window->set_cursor(Glib::RefPtr<Gdk::Cursor>());

	should_ungrab = false;
}

void Widget_Eyedropper::pick_color_at(int x, int y)
{
	if (!get_window())
		return;

	Glib::RefPtr<Gdk::Pixbuf> screenshot_pixbuf;
	Glib::RefPtr<Gdk::Window> root_window = get_window()->get_default_root_window();

	screenshot_pixbuf = Gdk::Pixbuf::create(root_window, x, y, 1, 1);

	if (screenshot_pixbuf->get_colorspace() != Gdk::COLORSPACE_RGB) {
		synfig::error(_("Unsupported colorspace for eyedropper. Please report your Gtk version"));
		return;
	}

	if (screenshot_pixbuf->get_bits_per_sample() % 8 != 0) {
		synfig::error(_("Unsupported bits per sample for eyedropper: %i. Please report your Gtk version and system info"), screenshot_pixbuf->get_bits_per_sample());
		return;
	}

	if (screenshot_pixbuf->get_n_channels() < 3) {
		synfig::error(_("Unsupported number of color channels for eyedropper: %i. Please report your Gtk version and system info"), screenshot_pixbuf->get_n_channels());
		return;
	}

	const double max_channel_value = (1 << screenshot_pixbuf->get_bits_per_sample()) - 1.0;
	const guint8 * pixels = screenshot_pixbuf->get_pixels();

	switch (screenshot_pixbuf->get_bits_per_sample()) {
	case 8:
		color.set_rgba_u(pixels[0] << 8, pixels[1] << 8, pixels[2] << 8);
		break;
	case 16: {
		const guint16 * pixels16 = reinterpret_cast<const guint16*>(pixels);
		color.set_rgba_u(pixels16[0], pixels16[1], pixels16[2]);
		break;
	}
	case 32: {
		const guint32 * pixels32 = reinterpret_cast<const guint32*>(pixels);
		color.set_rgba(pixels32[0] / max_channel_value, pixels32[1] / max_channel_value, pixels32[2] / max_channel_value);
		break;
	}
	}
}

bool Widget_Eyedropper::on_key_press_event(GdkEventKey* key_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	if (key_event->keyval == GDK_KEY_Escape) {
		if (should_ungrab) {
			color.set("#00000000");
			ungrab();
			signal_cancelled().emit();
		}
	}

	SYNFIG_EXCEPTION_GUARD_END_NO_RETURN()
	return true;
}

bool Widget_Eyedropper::on_key_release_event(GdkEventKey* key_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	if (key_event->keyval == GDK_KEY_Escape) {
		if (should_ungrab) {
			color.set("#00000000");
			ungrab();
			signal_cancelled().emit();
		}
	}

	SYNFIG_EXCEPTION_GUARD_END_NO_RETURN()
	return true;
}

// Note: ungrab() must be called on button release.
// Otherwise the button release event can activate some behavior on the widget
// the pointer is hovering (inside or outside the app)
bool Widget_Eyedropper::on_button_release_event(GdkEventButton* button_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	if (button_event->button == 1) {
		pick_color_at(int(button_event->x_root), int(button_event->y_root));
		ungrab();
		signal_color_picked().emit(color);
	}

	SYNFIG_EXCEPTION_GUARD_END_NO_RETURN()
	return true;
}

bool Widget_Eyedropper::on_grab_broken_event(GdkEventGrabBroken*)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	ungrab();
	signal_cancelled().emit();

	SYNFIG_EXCEPTION_GUARD_END_NO_RETURN()
	return false;
}

#if not GTK_CHECK_VERSION(3, 20, 0)
Glib::RefPtr<Gdk::Device> Widget_Eyedropper::get_pointer() {
	auto device_manager = Gdk::Display::get_default()->get_device_manager();
	return device_manager->get_client_pointer();
}

std::vector<Glib::RefPtr<Gdk::Device>> Widget_Eyedropper::get_keyboards() {
	std::vector<Glib::RefPtr<Gdk::Device>> detected_keyboards;

	auto device_manager = Gdk::Display::get_default()->get_device_manager();
	auto device_list = device_manager->list_devices(Gdk::DEVICE_TYPE_MASTER);
	for (auto& device : device_list) {
		if (device->get_source() == Gdk::SOURCE_KEYBOARD) {
			detected_keyboards.push_back(device);
		}
	}

	return detected_keyboards;
}
#endif

// Glade & GtkBuilder related

GType Widget_Eyedropper::gtype = 0;

Glib::ObjectBase* Widget_Eyedropper::wrap_new(GObject* o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new Widget_Eyedropper(GTK_DRAWING_AREA(o));
	else
		return Gtk::manage(new Widget_Eyedropper(GTK_DRAWING_AREA(o)));
}

void Widget_Eyedropper::register_type()
{
	if (gtype)
		return;

	Widget_Eyedropper dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	Glib::wrap_register(gtype, Widget_Eyedropper::wrap_new);
}
