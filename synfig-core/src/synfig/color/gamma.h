/* === S Y N F I G ========================================================= */
/*!	\file gamma.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_COLOR_GAMMA_H
#define __SYNFIG_COLOR_GAMMA_H

/* === H E A D E R S ======================================================= */

#include "color.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Gamma
**	\brief This class performs color correction on Color classes.
**	\stub
*/
class Gamma
{
private:
	ColorReal gamma[3];

public:
	static ColorReal calculate(ColorReal f, ColorReal gamma)
		{ return f < 0 ? -powf(-f, gamma) : powf(f, gamma); }

	explicit Gamma(ColorReal x = ColorReal(1)):
		Gamma(x, x, x) { }
	Gamma(ColorReal r, ColorReal g, ColorReal b)
		{ set(r, g, b); }
	
	bool operator==(const Gamma &other) const
		{ return get_r() == other.get_r() && get_g() == other.get_g() && get_b() == other.get_b(); }
	bool operator!=(const Gamma &other) const
		{ return !(*this == other); }
	bool operator<(const Gamma &other) const {
		return get_r() < other.get_r() ? true
		     : other.get_r() < get_r() ? false
		     : get_g() < other.get_g() ? true
		     : other.get_g() < get_g() ? false
		     : get_b() < other.get_b();
	}
	
	Gamma operator* (const Gamma &other) const
		{ return Gamma(get_r()*other.get_r(), get_g()*other.get_g(), get_b()*other.get_b()); }
	Gamma operator/ (const Gamma &other) const
		{ return *this * other.get_inverted(); }

	Gamma operator* (const ColorReal &x) const
		{ return *this * Gamma(x); }
	Gamma operator/ (const ColorReal &x) const
		{ return *this * Gamma(1/x); }

	Gamma& operator*= (const Gamma &other)
		{ return *this = *this * other; }
	Gamma& operator/= (const Gamma &other)
		{ return *this = *this / other; }

	Gamma& operator*= (const ColorReal &x)
		{ return *this = *this * x; }
	Gamma& operator/= (const ColorReal &x)
		{ return *this = *this / x; }

	void set(int channel, ColorReal x) { gamma[channel] = x; }
	void set_r(ColorReal x) { set(0, x); }
	void set_g(ColorReal x) { set(1, x); }
	void set_b(ColorReal x) { set(2, x); }
	void set(ColorReal x) { set(x, x, x); }
	void set(ColorReal r, ColorReal g, ColorReal b)
		{ set_r(r); set_g(g); set_b(b); }

	ColorReal get(int channel) const { return gamma[channel]; }
	ColorReal get_r() const { return get(0); }
	ColorReal get_g() const { return get(1); }
	ColorReal get_b() const { return get(2); }
	ColorReal get() const
		{ return (get_r() + get_g() + get_b())*(1/ColorReal(3)); }

	ColorReal apply(int channel, ColorReal x) const { return calculate(x, get(channel)); }
	ColorReal apply_r(ColorReal x) const { return apply(0, x); }
	ColorReal apply_g(ColorReal x) const { return apply(1, x); }
	ColorReal apply_b(ColorReal x) const { return apply(2, x); }
	Color apply(const Color &x) const
		{ return Color(apply_r(x.get_r()), apply_g(x.get_g()), apply_b(x.get_b()), x.get_a()); }
	
	void invert() { *this = get_inverted(); }
	Gamma get_inverted() const
		{ return Gamma(1/get_r(), 1/get_g(), 1/get_b()); }
}; // END of class Gamma

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
