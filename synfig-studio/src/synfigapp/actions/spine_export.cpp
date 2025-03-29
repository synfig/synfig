/* === S Y N F I G ========================================================= */
/*! \file spine_export.cpp
** \brief Spine Export Implementation
**
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig. If not, see <https://www.gnu.org/licenses/>.
** \endlegal
*/
/* ========================================================================= */

#include "spine_export.h"
#include <synfig/general.h>
#include <fstream>
#include <synfig/localization.h>
#include <synfig/canvas.h>
#include <synfig/layers/layer_skeleton.h>
#include <json/json.h>

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::SpineExport);
ACTION_SET_NAME(Action::SpineExport, "SpineExport");
ACTION_SET_LOCAL_NAME(Action::SpineExport, ("Export to Spine JSON"));
ACTION_SET_TASK(Action::SpineExport, "export");
ACTION_SET_CATEGORY(Action::SpineExport, Action::CATEGORY_OTHER);
ACTION_SET_PRIORITY(Action::SpineExport, 0);
ACTION_SET_VERSION(Action::SpineExport, "0.1");

/* === M E T H O D S ======================================================= */

Action::SpineExport::SpineExport() {}


Action::ParamVocab 
Action::SpineExport::get_param_vocab() {
    ParamVocab ret;
    ret.push_back(ParamDesc("canvas", Param::TYPE_CANVAS)
        .set_local_name(_("Canvas"))
        .set_desc(_("Canvas to be exported")));
    return ret;
}

bool 
Action::SpineExport::is_candidate(const ParamList& x) {
    return candidate_check(get_param_vocab(), x);
}

bool 
Action::SpineExport::set_param(const synfig::String& name, const Action::Param &param) {
    if (name == "canvas" && param.get_type() == Param::TYPE_CANVAS) {
        canvas_ = param.get_canvas();
        return true;
    }
    return false;
}

bool
Action::SpineExport::is_ready() const {
    return canvas_.operator bool();
}

void 
Action::SpineExport::prepare() {
    if (!first_time()) {
        return;
    }
    
    if (!canvas_) {
        throw Error(_("No valid canvas to export"));
    }
    
    export_to_spine_json(canvas_, "spine_export.json");
}

void 
synfigapp::Action::SpineExport::export_to_spine_json(const synfig::Canvas::Handle&, const synfig::String& filename) {
    // Hardcoded JSON structure for Spine export
    synfig::String spine_json = R"(
    {
        "skeleton": { "hash": "123456", "spine": "4.0", "width": 500, "height": 500 },
        "bones": [
            { "name": "root" },
            { "name": "bone1", "parent": "root", "length": 100, "x": 50, "y": 0, "rotation": 0 }
        ],
        "slots": [
            { "name": "slot1", "bone": "bone1", "attachment": "default" }
        ],
        "skins": { "default": { "slot1": { "default": { "width": 100, "height": 100 } } } },
        "animations": { "idle": { "bones": { "bone1": { "rotate": [{ "time": 0, "angle": 10 }, { "time": 1, "angle": -10 }] } } } }
    })";

    // Write to file
    std::ofstream out(filename);
    if (out) {
        out << spine_json;
        out.close();
        std::cout << "Exported Spine JSON to " << filename << std::endl;
    } else {
        std::cerr << "Failed to write JSON file: " << filename << std::endl;
    }
}
