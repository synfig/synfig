#include "dialog_canvaspasteoptions.h"

#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <gtkmm/cellrenderertoggle.h>

#include <gui/localization.h>
#include <gui/resourcehelper.h>
#include <synfig/general.h>

using namespace synfig;
using namespace studio;

static Glib::RefPtr<Gtk::Builder> load_interface()
{
    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_file(ResourceHelper::get_ui_path("dialog_canvaspasteoptions.glade"));
    } catch (const Glib::FileError& ex) {
        synfig::error("FileError: " + ex.what());
        return Glib::RefPtr<Gtk::Builder>();
    } catch (const Glib::MarkupError& ex) {
        synfig::error("MarkupError: " + ex.what());
        return Glib::RefPtr<Gtk::Builder>();
    } catch (const Gtk::BuilderError& ex) {
        synfig::error("BuilderError: " + ex.what());
        return Glib::RefPtr<Gtk::Builder>();
    }
    return refBuilder;
}

Dialog_CanvasPasteOptions::Dialog_CanvasPasteOptions(
    Gtk::Dialog::BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Dialog(cobject)
    , builder(refGlade)
{
    canvases_model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
        refGlade->get_object("canvases_liststore"));

    Glib::RefPtr<Gtk::CellRendererToggle> toggle =
        Glib::RefPtr<Gtk::CellRendererToggle>::cast_dynamic(
            refGlade->get_object("canvas_copy_column_renderer"));
    if (toggle)
        toggle->signal_toggled().connect(
            sigc::mem_fun(*this, &Dialog_CanvasPasteOptions::on_canvas_copy_toggled));
}

std::shared_ptr<Dialog_CanvasPasteOptions>
Dialog_CanvasPasteOptions::create(Gtk::Window& parent)
{
    auto refBuilder = load_interface();
    if (!refBuilder)
        return nullptr;
    Dialog_CanvasPasteOptions* dialog_ptr = nullptr;
    refBuilder->get_widget_derived("dialog_canvaspasteoptions", dialog_ptr);
    if (dialog_ptr)
        dialog_ptr->set_transient_for(parent);
    return std::shared_ptr<Dialog_CanvasPasteOptions>(dialog_ptr);
}

void Dialog_CanvasPasteOptions::set_canvases(
    const std::vector<Canvas::LooseHandle>& canvas_list)
{
    if (!canvases_model)
        return;
    canvases_model->clear();
    for (auto canvas : canvas_list) {
        if (!canvas) continue;
        auto iter = canvases_model->append();
        iter->set_value(COLUMN_CANVAS_POINTER, canvas.get());
        iter->set_value(COLUMN_NAME, canvas->get_id());
        iter->set_value(COLUMN_FILE_PATH,
            strprintf("(%s)", canvas->get_file_name().c_str()));
        iter->set_value(COLUMN_COPY_OR_NOT, true);
    }
}

void Dialog_CanvasPasteOptions::set_destination_canvas(Canvas::Handle /*canvas*/)
{
    // reserved for future conflict detection
}

void Dialog_CanvasPasteOptions::get_user_choices(
    std::map<Canvas::LooseHandle, bool>& user_choices) const
{
    canvases_model->foreach_iter(
        [&user_choices](const Gtk::TreeModel::iterator& iter) -> bool {
            Canvas* canvas_ptr;
            bool should_copy;
            iter->get_value(0, canvas_ptr);
            iter->get_value(3, should_copy);
            if (canvas_ptr)
                user_choices[canvas_ptr] = should_copy;
            return false;
        });
}

void Dialog_CanvasPasteOptions::on_canvas_copy_toggled(const Glib::ustring& path)
{
    auto iter = canvases_model->get_iter(path);
    bool is_copy;
    iter->get_value(COLUMN_COPY_OR_NOT, is_copy);
    iter->set_value(COLUMN_COPY_OR_NOT, !is_copy);
}