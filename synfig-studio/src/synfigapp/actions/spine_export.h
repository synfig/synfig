/* === S Y N F I G ========================================================= */
/*! \file spine_export.h
** \brief Spine JSON Export Header File
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
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
** \endlegal
*/
/* ========================================================================= */

#ifndef SYNFIG_APP_ACTION_SPINE_EXPORT_H
#define SYNFIG_APP_ACTION_SPINE_EXPORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfig/string.h>
#include <synfigapp/action.h>


/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class SpineExport : public Super {
private:
    synfig::Canvas::Handle canvas_;
    synfig::String export_path_;

    void export_to_spine_json(const synfig::Canvas::Handle& canvas, const synfig::String& filename);

public:
    SpineExport();

    static ParamVocab get_param_vocab();
    static bool is_candidate(const ParamList &x);

    virtual bool set_param(const synfig::String& name, const Param &);
    virtual bool is_ready() const;
    virtual void prepare();

    ACTION_MODULE_EXT
};

}; // END of namespace Action
}; // END of namespace synfigapp

#endif
