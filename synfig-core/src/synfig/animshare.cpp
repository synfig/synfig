/* === S Y N F I G ========================================================= */
/*!	\file animshare.cpp
**	\brief Implementation of the AnimShare value type
**	\legal
**	This file is part of Synfig.
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "animshare.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M E T H O D S ======================================================= */

AnimShare::AnimShare() : delay_(0.0), order_(0) {}

AnimShare::AnimShare(const String &param, const Time &delay, int order)
    : param_(param), delay_(delay), order_(order) {}

const String &AnimShare::get_param() const { return param_; }

void AnimShare::set_param(const String &x) { param_ = x; }

const Time &AnimShare::get_delay() const { return delay_; }

void AnimShare::set_delay(const Time &x) { delay_ = x; }

int AnimShare::get_order() const { return order_; }

void AnimShare::set_order(int x) { order_ = x; }

bool AnimShare::operator==(const AnimShare &rhs) const {
  return param_ == rhs.param_ && delay_ == rhs.delay_ && order_ == rhs.order_;
}
