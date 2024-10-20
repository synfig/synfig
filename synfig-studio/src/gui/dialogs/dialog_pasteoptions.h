/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_pasteoptions.h
**	\brief Header of dialog to user choose how to handle value nodes when pasting layers
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef SYNFIG_STUDIO_DIALOG_PASTEOPTIONS_H
#define SYNFIG_STUDIO_DIALOG_PASTEOPTIONS_H

#include <gtkmm/builder.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/treemodel.h>

#include <synfig/canvas.h>
#include <synfig/valuenode.h>

namespace Gtk {
class ListStore;
}

namespace studio
{

/**
 * @brief Dialog to let/warn user about exported valuenodes on layer pasting
 *
 * This dialog let user choose how to handle some exported valuenodes from their
 * canvas to another.
 *
 * This is useful for copying and pasting layers from one
 * file to another one. This action can lead to corruption of destination file,
 * specially if the source file is deleted, moved or renamed.
 *
 * This dialog lists (exported) valuenode candidates (set_value_nodes()) to be
 * used/"pasted" in a canvas (set_canvas()). User can choose, for every exported
 * value node, if (s)he wants to link the valuenode from source file or copy them
 * to target canvas.
 * There are 3 cases when copying exported value nodes:
 * 1) Target canvas doesn't have a value node with same id:
 *   \_ Regular copy
 * 2) Target canvas has a value node with same id, type and value type:
 *   \_ Both value nodes will be bound (they represent same stuff)
 * 3) Target canvas has a value node with same id, but is of a different type
 *    or handles a different value type:
 *   \_ Cannot copy on this case. User should rename it or let it be linked to source file
 *
 * Therefore, before running this dialog you should use set_value_nodes() and
 * set_canvas().
 *
 * Use get_user_choices() to retrieve user decisions for every valuenode.
 */
class Dialog_PasteOptions : public Gtk::Dialog
{
	const Glib::RefPtr<Gtk::Builder>& builder;

	Glib::RefPtr<Gtk::ListStore> valuenodes_model;

	std::set<synfig::ValueNode::LooseHandle> value_nodes;
	synfig::Canvas::Handle destination_canvas;

	std::map<synfig::ValueNode::LooseHandle, std::vector<synfig::ValueNode::LooseHandle>> value_node_dependencies;

	Glib::RefPtr<Gdk::Pixbuf> pixbuf_empty;
	Glib::RefPtr<Gdk::Pixbuf> pixbuf_link;
	Glib::RefPtr<Gdk::Pixbuf> pixbuf_external_link;
	Glib::RefPtr<Gdk::Pixbuf> pixbuf_conflict;

	Gtk::CheckButton* copy_all_checkbutton;


	enum {
		COLUMN_VALUENODE_POINTER,
		COLUMN_ORIGINAL_NAME,
		COLUMN_NAME,
		COLUMN_IS_NAME_EDITABLE,
		COLUMN_FILE_PATH,
		COLUMN_FILE_PATH_VISIBILITY,
		COLUMN_STATUS,
		COLUMN_STATUS_ICON,
		COLUMN_STATUS_TOOLTIP,
		COLUMN_VALUE_TYPE,
		COLUMN_COPY_OR_NOT,
		COLUMN_IS_EDITABLE
	};

public:
	Dialog_PasteOptions(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);

	static std::shared_ptr<Dialog_PasteOptions> create(Gtk::Window& parent);

	void set_value_nodes(const std::vector<synfig::ValueNode::LooseHandle>& value_node_list);
	void set_destination_canvas(synfig::Canvas::Handle canvas);

	/**
	 *  Retrieve user decisions for every valuenode
	 *  \param[out] user_choices Maps the valuenode to a possible new ID.
	 *       If the new ID is empty, user chose to link to external canvas
	 *       If same as original ID (the current value node/map key ID), user chose to link to existent valuenode in local canvas
	 *       If different from original ID, user chose to create a new valuenode on copying
	 */
	void get_user_choices(std::map<synfig::ValueNode::LooseHandle, std::string>& user_choices) const;

private:
	void on_valuenode_copy_toggled(const Glib::ustring& path);
	void on_valuenode_name_edited(const Glib::ustring& path, const Glib::ustring& new_text);
	void on_copy_all_toggled();

	void update_ok_button_sensitivity();
	void update_copy_all_button_status();

	void clear();
	void rebuild_model();
	void refresh_status();
	void refresh_row_status(size_t row_index);
};

}

#endif // SYNFIG_STUDIO_DIALOG_PASTEOPTIONS_H
