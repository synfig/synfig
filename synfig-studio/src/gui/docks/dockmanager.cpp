/* === S Y N F I G ========================================================= */
/*!	\file dockmanager.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "docks/dockmanager.h"
#include <stdexcept>
#include "docks/dockable.h"
#include "docks/dockbook.h"
#include "docks/dockdialog.h"
#include <synfigapp/settings.h>
#include <synfigapp/main.h>
#include <gdkmm/general.h>

#include <gui/localization.h>

#include <gtkmm/paned.h>
#include <gtkmm/box.h>
#include <gtkmm/window.h>

#include "app.h"
#include "mainwindow.h"
#include "canvasview.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === P R O C E D U R E S ================================================= */

std::map<Gtk::Container*, bool> DockManager::containers_to_remove_;

namespace studio {
	class DockLinkPoint {
	public:
		Gtk::Bin *bin;
		Gtk::Paned *paned;
		Gtk::Window *window;
		bool is_first;

		DockLinkPoint(): bin(NULL), paned(NULL), window(NULL), is_first(false) { }
		explicit DockLinkPoint(Gtk::Bin *bin): bin(bin), paned(NULL), window(NULL), is_first(false) { }
		explicit DockLinkPoint(Gtk::Paned *paned, bool is_first): bin(NULL), paned(paned), window(NULL), is_first(is_first) { }
		explicit DockLinkPoint(Gtk::Window *window): bin(NULL), paned(NULL), window(window), is_first(false) { }
		explicit DockLinkPoint(Gtk::Widget &widget) {
			Gtk::Container *container = widget.get_parent();
			bin = dynamic_cast<Gtk::Bin*>(container);
			paned = dynamic_cast<Gtk::Paned*>(container);
			window = dynamic_cast<Gtk::Window*>(container);
			is_first = paned != NULL && paned->get_child1() == &widget;
		}

		bool is_valid() { return bin || paned || window; }

		void unlink() {
			if (paned && is_first && paned->get_child1())
				paned->remove(*paned->get_child1());
			else
			if (paned && !is_first && paned->get_child2())
				paned->remove(*paned->get_child2());
			else
			if (window)
				window->remove();
			if (bin)
				bin->remove();
		}

		void link(Gtk::Widget &widget)
		{
			if (paned && is_first)
				paned->pack1(widget, true, false);
			else
			if (paned && !is_first)
				paned->pack2(widget, true, false);
			else
			if (window)
				window->add(widget);
			else
			if (bin)
				bin->add(widget);
		}
	};
}

class studio::DockSettings : public synfigapp::Settings
{
	DockManager* dock_manager;

public:
	DockSettings(DockManager* dock_manager):dock_manager(dock_manager)
	{
		synfigapp::Main::settings().add_domain(this,"dock");
	}

	virtual ~DockSettings()
	{
		synfigapp::Main::settings().remove_domain("dock");
	}

	virtual bool get_value(const synfig::String& key_, synfig::String& value)const
	{
		try
		{
			if (key_ == "layout")
			{
				value = dock_manager->save_layout_to_string();
				return true;
			}
		}catch (...) { return false; }
		return synfigapp::Settings::get_value(key_,value);
	}

	virtual bool set_value(const synfig::String& key_,const synfig::String& value)
	{
		try
		{
			if (key_ == "layout")
			{
				dock_manager->load_layout_from_string(value);
				return true;
			}
		}catch (...) { return false; }
		return synfigapp::Settings::set_value(key_,value);
	}

	virtual KeyList get_key_list()const
	{
		synfigapp::Settings::KeyList ret(synfigapp::Settings::get_key_list());
		ret.push_back("layout");
		return ret;
	}
};

/* === M E T H O D S ======================================================= */

DockManager::DockManager():
	dock_settings(new DockSettings(this))
{
}

DockManager::~DockManager()
{
	while(!dock_dialog_list_.empty())
	{
		dock_dialog_list_.back()->close();
	}
	while(!dockable_list_.empty())
	{
		Dockable* dockable(dockable_list_.back());
		// synfig::info("DockManager::~DockManager(): Deleting dockable \"%s\"",dockable->get_name().c_str());
		dockable_list_.pop_back();
		delete dockable;
	}
}

void
DockManager::register_dockable(Dockable& x)
{
	dockable_list_.push_back(&x);
	signal_dockable_registered()(&x);
}

bool
DockManager::unregister_dockable(Dockable& x)
{
	for(std::list<Dockable*>::iterator iter = dockable_list_.begin(); iter != dockable_list_.end(); ++iter)
	{
		if (&x == *iter)
		{
			remove_widget_recursive(x);
			dockable_list_.erase(iter);
			signal_dockable_unregistered()(&x);
			update_window_titles();
			return true;
		}
	}
	return false;
}

Dockable&
DockManager::find_dockable(const synfig::String& x)
{
	std::list<Dockable*>::iterator iter;
	for(iter=dockable_list_.begin();iter!=dockable_list_.end();++iter)
		if((*iter)->get_name()==x)
			return **iter;

	throw std::runtime_error("DockManager::find_dockable(): not found");
}

void
DockManager::present(synfig::String x)
{
	try
	{
		find_dockable(x).present();
	}
	catch(...)
	{
	}
}

DockDialog&
DockManager::find_dock_dialog(int id)
{
	std::list<DockDialog*>::iterator iter;
	for(iter=dock_dialog_list_.begin();iter!=dock_dialog_list_.end();++iter)
		if((*iter)->get_id()==id)
			return **iter;

	DockDialog* dock_dialog(new DockDialog());
	dock_dialog->set_id(id);
	return *dock_dialog;
}

const DockDialog&
DockManager::find_dock_dialog(int id)const
{
	std::list<DockDialog*>::const_iterator iter;
	for(iter=dock_dialog_list_.begin();iter!=dock_dialog_list_.end();++iter)
		if((*iter)->get_id()==id)
			return **iter;

	throw std::runtime_error("DockManager::find_dock_dialog(int id)const: not found");
}

void
DockManager::show_all_dock_dialogs()
{
	std::list<DockDialog*>::iterator iter;
	for(iter=dock_dialog_list_.begin();iter!=dock_dialog_list_.end();++iter)
		(*iter)->present();
}

bool
DockManager::swap_widgets(Gtk::Widget &widget1, Gtk::Widget &widget2)
{
	DockLinkPoint point1(widget1);
	DockLinkPoint point2(widget2);
	if (point1.is_valid() && point2.is_valid())
	{
		point1.unlink();
		point2.unlink();
		point1.link(widget2);
		point2.link(widget1);
		return true;
	}
	return false;
}

void
DockManager::remove_empty_container_recursive(Gtk::Container &container)
{
	containers_to_remove_.erase(&container);
	Gtk::Paned *paned = dynamic_cast<Gtk::Paned*>(&container);
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(&container);
	DockBook *book = dynamic_cast<DockBook*>(&container);

	if (paned)
	{
		if (paned->get_child1() && paned->get_child2()) return;
		Gtk::Widget *child = paned->get_child1() ? paned->get_child1() : paned->get_child2();
		if (child)
		{
			DockLinkPoint link(*paned);
			if (link.is_valid())
			{
				paned->remove(*child);
				link.unlink();
				link.link(*child);
				delete paned;
			}
		}
		else
		{
			remove_widget_recursive(*paned);
			delete paned;
			return;
		}
	}
	else
	if (window)
	{
		if (!window->get_child())
			window->close();
	}
	else
	if (book)
	{
		if (!book->allow_empty && book->get_n_pages() == 0)
		{
			remove_widget_recursive(*book);
			delete book;
		}
	}
}

void
DockManager::remove_widget_recursive(Gtk::Widget &widget)
{
	Gtk::Container *container = widget.get_parent();
	if (container)
	{
		container->remove(widget);
		remove_empty_container_recursive(*container);
	}
}

bool
DockManager::add_widget(Gtk::Widget &dest_widget, Gtk::Widget &src_widget, bool vertical, bool first)
{
	if (&src_widget == &dest_widget) return false;

	// check for src widget is parent for dest_widget
	for(Gtk::Widget *parent = src_widget.get_parent(); parent != NULL; parent = parent->get_parent())
		if (parent == &dest_widget)
			return swap_widgets(src_widget, dest_widget);

	// unlink dest_widget
	DockLinkPoint dest_link(dest_widget);
	if (!dest_link.is_valid()) return false;
	dest_link.unlink();

	// unlink src_widget
	remove_widget_recursive(src_widget);

	// create new paned and link all
	Gtk::Paned *paned = manage(new Gtk::Paned(vertical ? Gtk::ORIENTATION_VERTICAL : Gtk::ORIENTATION_HORIZONTAL));
	paned->show();
	DockLinkPoint(paned, first).link(src_widget);
	DockLinkPoint(paned, !first).link(dest_widget);
	dest_link.link(*paned);
	return true;
}

bool
DockManager::add_dockable(Gtk::Widget &dest_widget, Dockable &dockable, bool vertical, bool first)
{
	DockBook *book = manage(new DockBook());
	book->show();
	if (add_widget(dest_widget, *book, vertical, first))
	{
		book->add(dockable);
		return true;
	}
	delete book;
	return false;
}

bool DockManager::read_separator(std::string &x)
{
	size_t pos = x.find_first_of("|]");
	if (pos == std::string::npos) { x.clear(); return false; }
	if (x[pos] == '|') { x = x.substr(pos+1); return true; }
	if (x[pos] == ']') x = x.substr(pos+1);
	return false;
}

std::string DockManager::read_string(std::string &x)
{
	size_t pos = x.find_first_of("|]");
	std::string res = x.substr(0, pos);
	if (pos == std::string::npos) x.clear(); else x = x.substr(pos);
	return res;
}

int DockManager::read_int(std::string &x)
{
	return strtol(read_string(x).c_str(), NULL, 10);
}

bool DockManager::read_bool(std::string &x)
{
	return read_string(x) == "true";
}

Gtk::Widget* DockManager::read_widget(std::string &x)
{
	bool hor = x.substr(0, 5) == "[hor|";
	bool vert = x.substr(0, 6) == "[vert|";

	// paned
	if (hor || vert)
	{
		// skip "[hor|" or "[vert|"
		x = x.substr(1);
		if (!read_separator(x)) return NULL;

		int size = read_int(x);
		if (!read_separator(x)) return NULL;

		Gtk::Widget *first = NULL;
		Gtk::Widget *second = NULL;

		first = read_widget(x);
		if (!read_separator(x)) return first;
		second = read_widget(x);
		read_separator(x);

		if (!first && !second) return NULL;
		if (first && !second) return first;
		if (!first && second) return second;

		// create paned
		Gtk::Paned *paned = manage(new Gtk::Paned(vert ? Gtk::ORIENTATION_VERTICAL : Gtk::ORIENTATION_HORIZONTAL));
		paned->pack1(*first,  true, false);
		paned->pack2(*second, true, false);
		paned->set_position(size);
		paned->show();
		return paned;
	}
	else
	if (x.substr(0, 6) == "[book|")
	{
		// skip "[book|"
		x = x.substr(1);
		if (!read_separator(x)) return NULL;

		DockBook *book = NULL;
		do
		{
			std::string name = read_string(x);
			if (!name.empty())
			{
				Dockable &dockable = find_dockable(name);
				Gtk::Container *container = dockable.get_parent();
				if (container) {
					container->remove(dockable);
					containers_to_remove_[container] = true;
				}
				if (book == NULL) { book = manage(new DockBook()); book->show(); }
				book->add(dockable);
			}			
			/*std::string name = read_string(x);
			if (!name.empty())
			{
				Dockable *dockable = &find_dockable(name);
				if (dockable != NULL)
				{
					Gtk::Container *container = dockable->get_parent();
					if (container)
					{
						container->remove(*dockable);
						containers_to_remove_[container] = true;
					}
					if (book == NULL) { book = manage(new DockBook()); book->show(); }
					book->add(*dockable);
				}
			}*/
		} while (read_separator(x));

		return book;
	}
	else
	if (x.substr(0, 8) == "[dialog|")
	{
		// skip "[dialog|"
		x = x.substr(1);
		if (!read_separator(x)) return NULL;

		int left = read_int(x);
		if (!read_separator(x)) return NULL;
		int top = read_int(x);
		if (!read_separator(x)) return NULL;
		int width = read_int(x);
		if (!read_separator(x)) return NULL;
		int height = read_int(x);
		if (!read_separator(x)) return NULL;

		Gtk::Widget *widget = read_widget(x);
		read_separator(x);

		if (!widget) return NULL;

		DockDialog *dialog = new DockDialog();
		dialog->add(*widget);
		dialog->move(left, top);
		dialog->set_default_size(width, height);
		dialog->resize(width, height);
		dialog->present();

		return NULL;
	}
	else
	if (x.substr(0, 12) == "[mainwindow|")
	{
		// skip "[dialog|"
		x = x.substr(1);
		if (!read_separator(x)) return NULL;

		int left = read_int(x);
		if (!read_separator(x)) return NULL;
		int top = read_int(x);
		if (!read_separator(x)) return NULL;
		int width = read_int(x);
		if (!read_separator(x)) return NULL;
		int height = read_int(x);
		if (!read_separator(x)) return NULL;

		Gtk::Widget *widget = read_widget(x);
		read_separator(x);

		if (!widget) return NULL;

		Gtk::Widget *child = App::main_window->root().get_child();
		App::main_window->root().remove();
		if (child && child != &App::main_window->main_dock_book())
			delete child;
		App::main_window->root().add(*widget);

		App::main_window->move(left, top);
		App::main_window->set_default_size(width, height);
		App::main_window->resize(width, height);
		App::main_window->present();

		return NULL;
	}
	else
	if (x.substr(0, 14) == "[mainnotebook]")
	{
		x = x.substr(14);
		if (App::main_window->main_dock_book().get_parent())
			App::main_window->main_dock_book().get_parent()->remove(App::main_window->main_dock_book());
		return &App::main_window->main_dock_book();
	}

	return NULL;
}

void DockManager::write_string(std::string &x, const std::string &str)
	{ x += str; }
void DockManager::write_separator(std::string &x, bool continue_)
	{ write_string(x, continue_ ? "|" : "]"); }
void DockManager::write_int(std::string &x, int i)
	{ write_string(x, strprintf("%d", i)); }
void DockManager::write_bool(std::string &x, bool b)
	{ write_string(x, b ? "true" : "false"); }

void DockManager::write_widget(std::string &x, Gtk::Widget* widget)
{
	Gtk::Paned *paned = dynamic_cast<Gtk::Paned*>(widget);
	DockBook *book = dynamic_cast<DockBook*>(widget);
	DockDialog *dialog = dynamic_cast<DockDialog*>(widget);

	if (widget == NULL)
	{
		return;
	}
	else
	if (widget == App::main_window)
	{
		write_string(x, "[mainwindow|");
		int left = 0, top = 0, width = 0, height = 0;
		App::main_window->get_position(left, top);
		App::main_window->get_size(width, height);
		write_int(x, left);
		write_separator(x);
		write_int(x, top);
		write_separator(x);
		write_int(x, width);
		write_separator(x);
		write_int(x, height);
		write_separator(x);

		write_widget(x, App::main_window->root().get_child());
		write_separator(x, false);
	}
	else
	if (widget == &App::main_window->main_dock_book())
	{
		write_string(x, "[mainnotebook]");
	}
	else
	if (dialog)
	{
		write_string(x, "[dialog|");
		int left = 0, top = 0, width = 0, height = 0;
		dialog->get_position(left, top);
		dialog->get_size(width, height);
		write_int(x, left);
		write_separator(x);
		write_int(x, top);
		write_separator(x);
		write_int(x, width);
		write_separator(x);
		write_int(x, height);
		write_separator(x);

		write_widget(x, dialog->get_child());
		write_separator(x, false);
	}
	else
	if (paned)
	{
		write_string(x, paned->get_orientation() == Gtk::ORIENTATION_HORIZONTAL ? "[hor|" : "[vert|");
		write_int(x, paned->get_position());
		write_separator(x);
		write_widget(x, paned->get_child1());
		write_separator(x);
		write_widget(x, paned->get_child2());
		write_separator(x, false);
	}
	else
	if (book)
	{
		write_string(x, "[book");
		for(int i = 0; i < book->get_n_pages(); ++i)
		{
			Dockable *dockable = dynamic_cast<Dockable*>(book->get_nth_page(i));
			if (dockable)
			{
				write_separator(x);
				write_string(x, dockable->get_name());
			}
		}
		write_separator(x, false);
	}
}

std::string DockManager::save_widget_to_string(Gtk::Widget *widget)
{
	std::string res;
	write_widget(res, widget);
	return res;
}

Gtk::Widget* DockManager::load_widget_from_string(const std::string &x)
{
	std::string copy(x);
	Gtk::Widget *widget = read_widget(copy);
	while (!containers_to_remove_.empty())
		remove_empty_container_recursive(*containers_to_remove_.begin()->first);
	return widget;
}

std::string DockManager::save_layout_to_string()
{
	std::string res;
	for(std::list<DockDialog*>::iterator i = dock_dialog_list_.begin(); i != dock_dialog_list_.end(); i++)
	{
		write_widget(res, *i);
		write_separator(res);
	}
	write_widget(res, App::main_window);
	return res;
}

void DockManager::load_layout_from_string(const std::string &x)
{
	std::string copy(x);
	do
	{
		read_widget(copy);
	} while (read_separator(copy));
	while (!containers_to_remove_.empty())
		remove_empty_container_recursive(*containers_to_remove_.begin()->first);
}

std::string DockManager::layout_from_template(const std::string &tpl, float dx, float dy, float sx, float sy)
{
	std::string res;
	size_t pos_begin;
	size_t pos_end = 0;
	while(true)
	{
		pos_begin = tpl.find_first_of("%", pos_end);
		if (pos_begin == std::string::npos)
			{ res+=tpl.substr(pos_end); break; }
		res+=tpl.substr(pos_end, pos_begin-pos_end);
		pos_end = tpl.find_first_of("xyXY", pos_begin);
		if (pos_end == std::string::npos) break;
		float f = (float)strtol(tpl.c_str()+pos_begin+1, NULL, 10);
		if (tpl[pos_end] == 'X') res += strprintf("%d", (int)roundf(dx+f*sx/100.f));
		if (tpl[pos_end] == 'Y') res += strprintf("%d", (int)roundf(dy+f*sy/100.f));
		if (tpl[pos_end] == 'x') res += strprintf("%d", (int)roundf(f*sx/100.f));
		if (tpl[pos_end] == 'y') res += strprintf("%d", (int)roundf(f*sy/100.f));
		pos_end++;
	}
	return res;
}

void DockManager::set_dock_area_visibility(bool visible, DockBook* source)
{
	for(auto iter=dockable_list_.begin();iter!=dockable_list_.end();++iter) {
		Dockable * dockable = *iter;
		if (!dockable->is_visible())
			continue;
		DockBook * book = dynamic_cast<DockBook*>((*iter)->get_parent());
		if (book)
			book->set_dock_area_visibility(visible, source);
	}
}

void
DockManager::update_window_titles()
{
	// build maps
	typedef std::map< CanvasView::ActivationIndex, CanvasView* > CanvasViewMap;
	typedef std::map< Glib::RefPtr<Gdk::Window>, std::string > TitleMap;
	CanvasViewMap canvas_view_map;
	TitleMap title_map;
	for(std::list<Dockable*>::iterator i = dockable_list_.begin(); i != dockable_list_.end(); i++)
	{
		if ((*i)->get_parent_window())
		{
			title_map[(*i)->get_parent_window()] = (*i)->get_parent_window() == App::main_window->get_window()
			                                     ? _("Synfig Studio") : _("Dock Panel");
			CanvasView *canvas_view = dynamic_cast<CanvasView*>(*i);
			if (canvas_view)
				canvas_view_map[canvas_view->get_activation_index()] = canvas_view;
		}
	}

	// prepare titles
	for(CanvasViewMap::iterator i = canvas_view_map.begin(); i != canvas_view_map.end(); i++)
		title_map[ i->second->get_parent_window() ] =
			i->second->get_local_name() + " - " + _("Synfig Studio");

	// set titles
	for(TitleMap::iterator i = title_map.begin(); i != title_map.end(); i++)
		i->first->set_title(i->second);
}
