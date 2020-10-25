/*! ========================================================================
** Extended Template Library
**
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
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__REF_COUNT_H
#define __ETL__REF_COUNT_H

/* === H E A D E R S ======================================================= */

#include <cassert>
#include <atomic>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

// ========================================================================
/*!	\class	reference_counter	_ref_count.h	ETL/ref_count
**	\brief	Reference counter
**	\writeme
*/
class reference_counter
{
public:
	typedef std::atomic<int> counter_type;

private:
	counter_type *counter;

public:
	explicit reference_counter(const bool &x = true):
		counter()
		{ if (x) reset(); }
	reference_counter(const reference_counter &x):
		reference_counter(false)
		{ *this = x; }
	~reference_counter() { detach(); }

	void reset() {
		detach();
		counter = new counter_type(1);
	}

	void detach() {
		if (counter) {
			int count = --(*counter);
			assert(count >= 0);
			if (count <= 0) delete counter;
			counter = 0;
		}
	}

	reference_counter& operator=(const reference_counter &other) {
		if (other.counter != counter) {
			detach();
			if (other.counter) {
				counter = other.counter;
				(*counter)++;
				assert(count() > 1);
			}
		}
		return *this;
	}


	int count() const { return counter ? (int)*counter: 0; }
	bool unique() const { return count() == 1; }
	operator int() const { return count(); }
}; // END of class reference_counter

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
