/* === S Y N F I G ========================================================= */
/*!	\file spineexport.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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

#include "spineexport.h"
#include "layeradd.h"
#include "layerremove.h"
#include <synfig/general.h>
#include <fstream>
#include <synfigapp/localization.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>
#include <json/json.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::SpineExport);
ACTION_SET_NAME(Action::SpineExport, "SpineExport");
ACTION_SET_LOCAL_NAME(Action::SpineExport, N_("Export to Spine JSON"));
ACTION_SET_TASK(Action::SpineExport, "export_spine");
ACTION_SET_CATEGORY(Action::SpineExport, Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::SpineExport, 0);
ACTION_SET_VERSION(Action::SpineExport, "0.0");


/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::SpineExport::SpineExport()
{
}

synfig::String
Action::SpineExport::get_local_name()const
{
	return get_layer_descriptions(layers, _("Export to Spine JSON"), _("Export Spine JSON"));
}

Action::ParamVocab
Action::SpineExport::get_param_vocab()
{
	return ParamVocab();
}

bool
Action::SpineExport::is_candidate(const ParamList &x)
{
	return true;
}

bool
Action::SpineExport::set_param(const synfig::String& name, const Action::Param &param)
{
	return false;
}

bool
Action::SpineExport::is_ready()const
{
	return true;
}

int
Action::SpineExport::lowest_depth()const
{
	return 0;
}

void 
Action::SpineExport::prepare() {
    std::string filename = synfigapp::Main::get_user_app_directory().u8string() + "/exported_spine.json";

    synfig::info("Creating Spine JSON file at: " + filename);

    // Open file for writing JSON data
    std::ofstream file(filename, std::ios::out);
    if (!file.is_open()) {
        synfig::warning("Failed to create Spine JSON file: " + filename);
        return;
    }

    // Hardcoded JSON content
    std::string jsonData = R"({
        "skeleton": {
            "hash": "abc123",
            "spine": "4.1",
            "width": 800,
            "height": 600
        },
        "bones": [
            {"name": "root"},
            {"name": "head", "parent": "root"}
        ]
    })";
    
    // Write JSON to file
    file << jsonData;
    file.close();

    synfig::info("Spine JSON file successfully created!");
    synfig::info("User can now download: " + filename);
}