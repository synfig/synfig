/* === S Y N F I G ========================================================= */
/*!	\file colorcorrect.h
**	\brief Header file for implementation of the "Color Correct" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_LAYER_COLORCORRECT_H
#define __SYNFIG_LAYER_COLORCORRECT_H

/* === H E A D E R S ======================================================= */

#include <synfig/angle.h>
#include <synfig/color.h>
#include <synfig/layer.h>
#include <synfig/rect.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/software/task/tasksw.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
namespace modules {
namespace mod_filter {

//! Task for RGB-based saturation adjustment
class TaskSaturation : public rendering::TaskPixelProcessor {
public:
  typedef etl::handle<TaskSaturation> Handle;
  static Token token;
  virtual Token::Handle get_token() const { return token.handle(); }

  Real saturation;
  Gamma canvas_gamma; //!< Canvas gamma for linearization

  TaskSaturation() : saturation(1.0), canvas_gamma(1.0) {}

  bool is_transparent() const {
    return approximate_equal_lp(saturation, Real(1.0));
  }
};

//! Software implementation of TaskSaturation
class TaskSaturationSW : public TaskSaturation, public rendering::TaskSW {
public:
  typedef etl::handle<TaskSaturationSW> Handle;
  static Token token;
  virtual Token::Handle get_token() const { return token.handle(); }

  virtual bool run(RunParams &params) const;

private:
  void apply_saturation(Color &dst, const Color &src) const;
};

class Layer_ColorCorrect : public Layer {
  SYNFIG_LAYER_MODULE_EXT

private:
  //! Parameter: (Angle)
  ValueBase param_hue_adjust;
  //! Parameter: (Real)
  ValueBase param_brightness;
  //! Parameter: (Real)
  ValueBase param_contrast;
  //! Parameter: (Real)
  ValueBase param_exposure;
  //! Parameter: (Real)
  ValueBase param_saturation;
  //! Parameter: (Real)
  ValueBase param_gamma;
  // This gamma member is kept to avoid need to recalculate the gamma table
  // on each pixel
  Gamma gamma;

  Color correct_color(const Color &in) const;

public:
  Layer_ColorCorrect();

  virtual bool set_param(const String &param, const synfig::ValueBase &value);

  virtual ValueBase get_param(const String &param) const;

  virtual Color get_color(Context context, const Point &pos) const;

  virtual Rect get_full_bounding_rect(Context context) const;

  virtual Vocab get_param_vocab() const;

  virtual rendering::Task::Handle
  build_rendering_task_vfunc(Context context) const;
}; // END of class Layer_ColorCorrect

}; // END of namespace mod_filter
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
