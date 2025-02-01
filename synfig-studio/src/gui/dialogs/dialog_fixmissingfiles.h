/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_fixmissingfiles.h
**	\brief Header for Dialog_FixMissingFiles
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2023-     Synfig Contributors
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

#ifndef SYNFIG_STUDIO_DIALOG_FIXMISSINGFILES_H
#define SYNFIG_STUDIO_DIALOG_FIXMISSINGFILES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>

#include <synfig/loadcanvas.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
class ListBox;
}

namespace studio {

class Dialog_FixMissingFiles : public Gtk::Dialog
{
public:
	static std::shared_ptr<Dialog_FixMissingFiles> create(Gtk::Window& parent);

	Dialog_FixMissingFiles(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);

	void set_canvas_filepath(const synfig::filesystem::Path& path);

	void set_broken_useids(synfig::CanvasBrokenUseIdMap& map);

protected:
	void on_response(int response_id) override;

private:
	Glib::RefPtr<Gtk::Builder> builder_;

	synfig::filesystem::Path canvas_filepath_;

	Glib::RefPtr<Gtk::ListBox> missing_file_list_;
	Glib::RefPtr<Gtk::Label> canvas_filepath_label_;
	synfig::CanvasBrokenUseIdMap* map_;

	/** maps (original file path, replacer file path) **/
	typedef std::map<synfig::filesystem::Path, synfig::filesystem::Path> FileReplacerMap;
	FileReplacerMap replacer_map_;

	void create_row(FileReplacerMap& replacer_map_, const synfig::CanvasBrokenUseIdMap::iterator& iter);

	void on_replace_button_clicked(const synfig::filesystem::Path& missing_path, Gtk::Label* label);

	/** return true if all missing files have a correspondent replacement file **/
	bool is_replacer_map_complete() const;

	void update_response_button_sensitivity();
}; // END of Dialog_FixMissingFiles

}; // END of namespace studio

/* === E N D =============================================================== */
#endif // SYNFIG_STUDIO_DIALOG_FIXMISSINGFILES_H
