/* === S Y N F I G ========================================================= */
/*!	\file clock.h
**	\brief A timer class
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_CLOCK_H
#define SYNFIG_CLOCK_H

/* === H E A D E R S ======================================================= */

#include <chrono>

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

	class clock {
		std::chrono::time_point<std::chrono::steady_clock> base_time_;
	public:

		clock() {
			reset();
		}

		void reset() {
			base_time_ = std::chrono::steady_clock::now();
		}

		float operator()() const {
			return std::chrono::duration<float>(std::chrono::steady_clock::now() - base_time_).count();
		}

		float pop_time() {
			// Grab the old base time
			std::chrono::time_point<std::chrono::steady_clock> old_time = base_time_;

			// Put the current time into base_time
			base_time_ = std::chrono::steady_clock::now();

			return std::chrono::duration<float>(base_time_ - old_time).count();
		}
	};
}

/* === E N D =============================================================== */

#endif
