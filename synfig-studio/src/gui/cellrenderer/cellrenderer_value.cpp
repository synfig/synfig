/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_value.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "cellrenderer_value.h"

#include <gdkmm/general.h>
#include <gtkmm/celleditable.h>
#include <gtkmm/eventbox.h>

#include <ETL/stringf>

#include <synfig/general.h>
#include <synfig/transformation.h>
#include <synfig/valuenodes/valuenode_bone.h>

#include <gui/app.h>
#include <gui/dialogs/dialog_gradient.h>
#include <gui/dialogs/dialog_color.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/widgets/widget_color.h> // render_color_to_window()
#include <gui/widgets/widget_gradient.h> // render_gradient_to_window()
#include <gui/widgets/widget_value.h>

#endif

using namespace synfig;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

// Number of decimal places for real values
static constexpr int real_num_decimals = 6;
static constexpr int angle_num_decimals = 2;

class studio::ValueBase_Entry : public Gtk::CellEditable, public Gtk::EventBox
{
	Glib::ustring     path;
	Widget_ValueBase *valuewidget;
	Gtk::Widget      *parent;
	bool              edit_done_called;
public:
	ValueBase_Entry():
		Glib::ObjectBase(typeid(ValueBase_Entry))
	{
		parent           = nullptr;
		edit_done_called = false;

		valuewidget = manage(new class Widget_ValueBase());
		valuewidget->inside_cellrenderer();
		add(*valuewidget);
		valuewidget->show();

		//set_can_focus(true);
		//set_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

		show_all_children();

	}
	~ValueBase_Entry()
	{
	}

	void on_editing_done()
	{
		hide();
		if (parent) parent->grab_focus();
		if (!edit_done_called)
		{
			edit_done_called = true;
			Gtk::CellEditable::on_editing_done();
		}
		else
		{
			synfig::error("on_editing_done(): Called twice!");
		}
	}

	void set_parent(Gtk::Widget* x) { parent = x; }

	void on_remove_widget()
	{
		hide();
		edit_done_called = true;
		if (parent) parent->grab_focus();
		Gtk::CellEditable::on_remove_widget();
	}

	void start_editing_vfunc(GdkEvent *event)
	{
		SYNFIG_EXCEPTION_GUARD_BEGIN()
		if (valuewidget) {
			valuewidget->signal_activate().connect(sigc::mem_fun(*this, &studio::ValueBase_Entry::editing_done));
			valuewidget->signal_key_press_event().connect(sigc::mem_fun(*this, &studio::ValueBase_Entry::on_key_press_event));
		}

		// popup combobox menu if its is a enum editor
		if (event && event->type == GDK_BUTTON_PRESS && valuewidget) {
			Type &type(valuewidget->get_value().get_type());
			bool popup_combobox = false;
			if (type == type_integer) {
				std::string param_hint = valuewidget->get_param_desc().get_hint();
				std::string child_param_hint = valuewidget->get_child_param_desc().get_hint();
				if ( param_hint == "enum" || child_param_hint == "enum" )
					popup_combobox = true;
			} else if (type == type_canvas)
				popup_combobox = true;
			else if (type == type_bone_valuenode)
				popup_combobox = true;
			else if (type == type_string) {
				std::string param_hint = valuewidget->get_param_desc().get_hint();
				std::string child_param_hint = valuewidget->get_child_param_desc().get_hint();
				if( param_hint == "sublayer_name" || child_param_hint == "sublayer_name")
					popup_combobox = true;
			}
			if (popup_combobox)
				valuewidget->popup_combobox();
		}

		show();
		//valuewidget->grab_focus();
		//get_window()->set_focus(*valuewidget);
		SYNFIG_EXCEPTION_GUARD_END()
	}

	bool on_event(GdkEvent *event)
	{
		SYNFIG_EXCEPTION_GUARD_BEGIN()
		if (event->any.type == GDK_BUTTON_PRESS
		 || event->any.type == GDK_2BUTTON_PRESS
		 || event->any.type == GDK_KEY_PRESS
		 || event->any.type == GDK_KEY_RELEASE
		 || event->any.type == GDK_SCROLL
		 || event->any.type == GDK_3BUTTON_PRESS )
			return true;
		return Gtk::EventBox::on_event(event);
		SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
	}

	bool on_key_press_event(GdkEventKey* key_event)
	{
		SYNFIG_EXCEPTION_GUARD_BEGIN()
		if(key_event->keyval == GDK_KEY_Escape)
		{
			property_editing_canceled() = true;
			on_editing_done();
			return true;
		}
		return Gtk::EventBox::on_key_press_event(key_event);
		SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
	}

	void on_grab_focus()
	{
		Gtk::EventBox::on_grab_focus();
		if (valuewidget)
			valuewidget->grab_focus();
	}

	void set_path(const Glib::ustring &p)
	{
		path = p;
	}

	void set_value(const synfig::ValueBase &data)
	{
		if (valuewidget)
			valuewidget->set_value(data);
		//valuewidget->grab_focus();
	}

	void set_canvas(const etl::handle<synfig::Canvas> &data)
	{
		assert(data);
		if (valuewidget)
			valuewidget->set_canvas(data);
	}

	void set_param_desc(const synfig::ParamDesc &data)
	{
		if (valuewidget)
			valuewidget->set_param_desc(data);
	}

	void set_value_desc(const synfigapp::ValueDesc &data)
	{
		if (valuewidget)
			valuewidget->set_value_desc(data);
	}

	void set_child_param_desc(const synfig::ParamDesc &data)
	{
		if (valuewidget)
			valuewidget->set_child_param_desc(data);
	}

	const synfig::ValueBase &get_value()
	{
		if (valuewidget)
			return valuewidget->get_value();

		warning("%s:%d this code shouldn't be reached", __FILE__, __LINE__);
		return *(new synfig::ValueBase());
	}

	const Glib::ustring &get_path()
	{
		return path;
	}

};

/* === P R O C E D U R E S ================================================= */

bool get_paragraph(synfig::String& text)
{
	return App::dialog_paragraph(_("Text Paragraph"), _("Enter text here:"), text);
}

/* === M E T H O D S ======================================================= */

CellRenderer_ValueBase::CellRenderer_ValueBase():
	Glib::ObjectBase          (typeid(CellRenderer_ValueBase)),
	property_value_	          (*this, "value",                   synfig::ValueBase()),
	property_canvas_          (*this, "canvas",      etl::handle<synfig::Canvas>()),
	property_param_desc_      (*this, "param_desc",              synfig::ParamDesc()),
	property_value_desc_      (*this, "value_desc",           synfigapp::ValueDesc()),
	property_child_param_desc_(*this, "child_param_desc",        synfig::ParamDesc()),
	edit_value_done_called    (false),
	value_entry()
{
	CellRendererText::signal_edited().connect(sigc::mem_fun(*this,
		&CellRenderer_ValueBase::string_edited_));

	Pango::AttrList attr_list;
	{
		Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*8));
		pango_size.set_start_index(0);
		pango_size.set_end_index(64);
		attr_list.change(pango_size);
	}
	property_attributes()   = attr_list;

	property_foreground()   = Glib::ustring("#7f7f7f");
	property_inconsistent() = false;
}

CellRenderer_ValueBase::~CellRenderer_ValueBase()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("CellRenderer_ValueBase::~CellRenderer_ValueBase(): Deleted");
}

void
CellRenderer_ValueBase::string_edited_(const Glib::ustring& path, const Glib::ustring& str)
{
	ValueBase old_value = property_value_.get_value();
	ValueBase value;

	if (old_value.get_type() == type_time)
	{
		value = ValueBase( Time(str, get_canvas()->rend_desc().get_frame_rate() ) );
	}
	else
		value = ValueBase( str );

	if (old_value != value)
		signal_edited_(path, value);
}

void
CellRenderer_ValueBase::render_vfunc(
	const    ::Cairo::RefPtr< ::Cairo::Context>& cr,
	      Gtk::Widget&           widget,
	const Gdk::Rectangle&        background_area,
	const Gdk::Rectangle&        cell_area,
	      Gtk::CellRendererState flags)
{
	if (!cr)
		return;

	Gdk::Rectangle aligned_area;
	get_aligned_area(widget, flags, cell_area, aligned_area);

	/*
	TODO: is widget state equals this state variable?
	      for checkbox only
	Gtk::StateType state = Gtk::STATE_INSENSITIVE;
	if(property_editable())
		state = Gtk::STATE_NORMAL;
	if((flags & Gtk::CELL_RENDERER_SELECTED) != 0)
		state = (widget.has_focus()) ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;
	*/

	ValueBase data = property_value_.get_value();

	Type &type(data.get_type());

	if (type == type_real)
	{
		if ( get_param_desc().get_is_distance()  || get_child_param_desc().get_is_distance())
		{
			Distance x( data.get(Real()), Distance::SYSTEM_UNITS);
			x.convert( App::distance_system, get_canvas()->rend_desc() );
			property_text() = x.get_string(real_num_decimals).c_str();
		}
		else
		{
			std::string format = strprintf("%%.%df", real_num_decimals);
			property_text() = strprintf(format.c_str(), data.get(Real()));
		}
	}
	else
	if (type == type_time)
	{
		property_text() =
			data.get(Time()).get_string( get_canvas()->rend_desc().get_frame_rate(),
				                                         App::get_time_format());
	}
	else
	if (type == type_angle)
	{
		const std::string angle_format = strprintf("%%.%df°", angle_num_decimals);
		property_text() = strprintf( angle_format.c_str(), (Real) Angle::deg( data.get(Angle()) ).get() );
	}
	else
	if (type == type_integer)
	{
		String param_hint, child_param_hint;
		param_hint       =       get_param_desc().get_hint();
		child_param_hint = get_child_param_desc().get_hint();
		if ( param_hint != "enum" && child_param_hint != "enum" )
		{
			property_text() = strprintf("%i", data.get(int()));
		}
		else
		{
			property_text() = strprintf("(%i)",data.get(int()));

			std::list<synfig::ParamDesc::EnumData> enum_list;
			if (param_hint == "enum")
				enum_list = ((synfig::ParamDesc) property_param_desc_).get_enum_list();
			else if (child_param_hint == "enum")
				enum_list = ((synfig::ParamDesc) property_child_param_desc_).get_enum_list();

			std::list<synfig::ParamDesc::EnumData>::iterator iter;
			for (iter = enum_list.begin(); iter != enum_list.end(); iter++)
				if (iter->value == data.get(int()))
				{
					// don't show the key_board s_hortcut under_scores
					String local_name = iter->local_name;
					String::size_type pos = local_name.find_first_of('_');
					if (pos != String::npos)
						property_text() = local_name.substr(0, pos) + local_name.substr(pos+1);
					else
						property_text() = local_name;
					break;
				}
		}
	}
	else
	if (type == type_vector)
	{
		Vector vector = data.get(Vector());
		if (get_param_desc().get_is_distance() || get_child_param_desc().get_is_distance()) {
			Distance x( vector[0], Distance::SYSTEM_UNITS ), y( vector[1], Distance::SYSTEM_UNITS );
			x.convert( App::distance_system, get_canvas()->rend_desc() );
			y.convert( App::distance_system, get_canvas()->rend_desc() );
			property_text() = strprintf("%s,%s", x.get_string(real_num_decimals).c_str(), y.get_string(real_num_decimals).c_str());
		} else {
			std::string format = strprintf("%%.%01df,%%.%01df", real_num_decimals, real_num_decimals);
			property_text() = strprintf(format.c_str(), vector[0], vector[1]);
		}
	}
	else
	if (type == type_transformation)
	{
		const Transformation &transformation = data.get(Transformation());
		const Vector         &offset         = transformation.offset;
		const Angle::deg     angle            (transformation.angle);
		const Vector         &scale          = transformation.scale;

		Distance x( offset[0], Distance::SYSTEM_UNITS ), y( offset[1], Distance::SYSTEM_UNITS );
		x.convert( App::distance_system, get_canvas()->rend_desc() );
		y.convert( App::distance_system, get_canvas()->rend_desc() );

		Distance sx( scale[0], Distance::SYSTEM_UNITS ), sy( scale[1], Distance::SYSTEM_UNITS );
		sx.convert( App::distance_system, get_canvas()->rend_desc() );
		sy.convert( App::distance_system, get_canvas()->rend_desc() );

		std::string format = strprintf("%%s,%%s,%%.%df°,%%s,%%s", angle_num_decimals);
		property_text() = static_cast<Glib::ustring>(strprintf(
			format.c_str(),
			x.get_string(real_num_decimals).c_str(),
			y.get_string(real_num_decimals).c_str(),
			(Real) angle.get(),
			sx.get_string(real_num_decimals).c_str(),
			sy.get_string(real_num_decimals).c_str()
		));
	}
	else
	if (type == type_string)
	{
		if ( !data.get(synfig::String()).empty() )
			property_text() = static_cast<Glib::ustring>( data.get(synfig::String()) );
		else
			property_text() = Glib::ustring("<empty>");
	}
	else
	if (type == type_canvas)
	{
		if ( data.get(etl::handle<synfig::Canvas>()) )
		{
			if (data.get( etl::handle<synfig::Canvas>())->is_inline() )
				property_text() = _("<Group>");
			else
				property_text() = data.get(etl::handle<synfig::Canvas>())->get_id();
		}
		else
			property_text() = _("<No Image Selected>");
	}
	else
	if (type == type_color)
	{
		render_color_to_window(cr, cell_area, data.get(Color()));
		return;
	}
	else
	if (type == type_bool)
	{
		Glib::RefPtr<Gtk::StyleContext> context = widget.get_style_context();
		context->context_save();
		Gtk::StateFlags state = get_state(widget, flags);
#if GTKMM_MAJOR_VERSION < 3 || (GTKMM_MAJOR_VERSION == 3 && GTKMM_MINOR_VERSION < 14)
		state &= ~(Gtk::STATE_FLAG_INCONSISTENT | Gtk::STATE_FLAG_ACTIVE);
#else
		state &= ~(Gtk::STATE_FLAG_INCONSISTENT | Gtk::STATE_FLAG_ACTIVE | Gtk::STATE_FLAG_CHECKED);
#endif
		if ((flags & Gtk::CELL_RENDERER_SELECTED) != 0 && widget.has_focus())
			state |= Gtk::STATE_FLAG_SELECTED;
		if (!property_editable())
			state |= Gtk::STATE_FLAG_INSENSITIVE;
		if (data.get(bool()))
#if GTKMM_MAJOR_VERSION < 3 || (GTKMM_MAJOR_VERSION == 3 && GTKMM_MINOR_VERSION < 14)
			state |= Gtk::STATE_FLAG_ACTIVE;
#else
			state |= Gtk::STATE_FLAG_CHECKED;
#endif

		cr->save();
		Gdk::Cairo::add_rectangle_to_path(cr, cell_area);
		cr->clip();

		context->add_class("check");
		context->set_state(state);
		context->render_check(
			cr,
			aligned_area.get_x(),
			aligned_area.get_y(),
			aligned_area.get_height(),
			aligned_area.get_height()
		);
		cr->restore();
		context->context_restore();
		return;
	}
	else
	if (type == type_nil)
	{
		return;
	}
	else
	if (type == type_gradient)
	{
		render_gradient_to_window(cr, cell_area, data.get(Gradient()));
		return;
	}
	else
	if (type == type_bone_object
	 || type == type_segment
	 || type == type_list
	 || type == type_bline_point
	 || type == type_width_point
	 || type == type_dash_item)
	{
		property_text() = data.get_type().description.local_name;
	}
	else
	if (type == type_bone_valuenode)
	{
		ValueNode_Bone::Handle bone_node(data.get(ValueNode_Bone::Handle()));
		String name(_("No Parent"));

		if (!bone_node->is_root())
		{
			name = (*(bone_node->get_link("name")))(get_canvas()->get_time()).get(String());
			if (name.empty())
				name = bone_node->get_guid().get_string();
		}

		property_text() = name;
	}
	else
	{
		property_text() = static_cast<Glib::ustring>(type.description.local_name);
	}

	CellRendererText::render_vfunc(cr, widget, background_area, cell_area, flags);
}


/*
bool
CellRenderer_ValueBase::activate_vfunc(	GdkEvent* event,
	Gtk::Widget& widget,
	const Glib::ustring& path,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	ValueBase data=(ValueBase)property_value_.get_value();

	if (data.type == type_bool)
	{
		if(property_editable())
			signal_edited_(path,ValueBase(!data.get(bool())));
    	return true;
    }
    else
    if (data.type == type_string)
    {
		return CellRendererText::activate_vfunc(event,widget,path,background_area,cell_area,flags);
	}
	return false;
}
*/

void
CellRenderer_ValueBase::gradient_edited(synfig::Gradient gradient, Glib::ustring path)
{
	ValueBase old_value(property_value_.get_value());
	ValueBase value(gradient);
	if (old_value != value)
		signal_edited_(path, value);
}

void
CellRenderer_ValueBase::color_edited(synfig::Color color, Glib::ustring path)
{
	ValueBase old_value(property_value_.get_value());
	ValueBase value(color);
	if (old_value != value)
		signal_edited_(path, value);
}

Gtk::CellEditable*
CellRenderer_ValueBase::start_editing_vfunc(
	GdkEvent*              /*event*/,
	Gtk::Widget&           widget,
	const Glib::ustring&   path,
	const Gdk::Rectangle&  /*background_area*/,
	const Gdk::Rectangle&  /*cell_area*/,
	Gtk::CellRendererState /*flags*/)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	edit_value_done_called = false;
	// If we aren't editable, then there is nothing to do
	if (!property_editable())
		return nullptr;

	ValueBase data = property_value_.get_value();

	Type &type(data.get_type());

	if (type == type_bool)
	{
		signal_edited_( path, ValueBase(!data.get(bool())) );
		return nullptr;
	}
	//else
	//if (type == type_time)
	//{
	//	property_text()=data.get(Time()).get_string(get_canvas()->rend_desc().get_frame_rate(),App::get_time_format()|Time::FORMAT_FULL);
	//	return CellRendererText::start_editing_vfunc(event,widget,path,background_area,cell_area,flags);
	//}
	else
	if (type == type_gradient)
	{
		App::dialog_gradient->reset();
		App::dialog_gradient->set_gradient( data.get(Gradient()) );
		App::dialog_gradient->signal_edited().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CellRenderer_ValueBase::gradient_edited),
				path
			)
		);
		App::dialog_gradient->set_default_button_set_sensitive(true);
		App::dialog_gradient->present();
		return nullptr;
	}
	else
	if (type == type_color)
	{
		App::dialog_color->reset();
		App::dialog_color->set_color( data.get(Color()) );
		App::dialog_color->signal_edited().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CellRenderer_ValueBase::color_edited),
				path
			)
		);
		App::dialog_color->present();
		return nullptr;
	}
	else
	if (type == type_string
	 && get_param_desc().get_hint() == "paragraph")
	{
		synfig::String string;
		string = data.get(string);
		if (get_paragraph(string))
			signal_edited_(path, ValueBase(string));
		return nullptr;
	}
	// if (type == type_string) && (get_param_desc().get_hint()!="filename")
		// return CellRendererText::start_editing_vfunc(event,widget,path,background_area,cell_area,flags);
	else
	{
		assert(get_canvas());

		saved_data = data;

		value_entry = manage(new ValueBase_Entry());
		value_entry->set_path(path);
		value_entry->set_canvas(get_canvas());
		value_entry->set_param_desc(get_param_desc());
		value_entry->set_value_desc(get_value_desc());
		value_entry->set_child_param_desc(get_child_param_desc());
		value_entry->set_value(data);
		value_entry->set_parent(&widget);
		value_entry->show(); // in order to enable "instant"/"single-click" pop-up for enum comboboxes
		value_entry->signal_editing_done().connect(sigc::mem_fun(*this, &CellRenderer_ValueBase::on_value_editing_done));
		CellRenderer::signal_editing_canceled().connect(sigc::mem_fun(*this, &CellRenderer_ValueBase::on_value_editing_done));
		return value_entry;
	}

	return nullptr;
	SYNFIG_EXCEPTION_GUARD_END_NULL(nullptr)
}

void
CellRenderer_ValueBase::on_value_editing_done()
{
	if(value_entry->property_editing_canceled()){
		CellRenderer::stop_editing(true);
	}

	if (edit_value_done_called)
	{
		synfig::error("on_value_editing_done(): Called twice!");
		//return;
	}

	edit_value_done_called = true;

	if(!value_entry->property_editing_canceled()){
		if (value_entry)
		{
			ValueBase value(value_entry->get_value());

			if (saved_data != value)
				signal_edited_(value_entry->get_path(), value);
		}
	}
}
