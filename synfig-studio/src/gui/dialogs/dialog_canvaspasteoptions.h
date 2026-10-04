#ifndef SYNFIG_STUDIO_DIALOG_CANVASPASTEOPTIONS_H
#define SYNFIG_STUDIO_DIALOG_CANVASPASTEOPTIONS_H

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>

#include <synfig/canvas.h>

namespace studio {

class Dialog_CanvasPasteOptions : public Gtk::Dialog
{
    const Glib::RefPtr<Gtk::Builder>& builder;
    Glib::RefPtr<Gtk::ListStore> canvases_model;

    enum {
        COLUMN_CANVAS_POINTER,
        COLUMN_NAME,
        COLUMN_FILE_PATH,
        COLUMN_COPY_OR_NOT
    };

public:
    Dialog_CanvasPasteOptions(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);

    static std::shared_ptr<Dialog_CanvasPasteOptions> create(Gtk::Window& parent);

    void set_canvases(const std::vector<synfig::Canvas::LooseHandle>& canvas_list);
    void set_destination_canvas(synfig::Canvas::Handle canvas);

    /// Maps each foreign canvas to a bool: true = copy, false = link
    void get_user_choices(std::map<synfig::Canvas::LooseHandle, bool>& user_choices) const;

private:
    void on_canvas_copy_toggled(const Glib::ustring& path);
};

} 

#endif // SYNFIG_STUDIO_DIALOG_CANVASPASTEOPTIONS_H