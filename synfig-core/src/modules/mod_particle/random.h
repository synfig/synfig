/* === S Y N F I G ========================================================= */
/*!	\file mod_particle/random.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_RANDOM_H
#define __SYNFIG_RANDOM_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */


#define POOL_SIZE	(256)
class Random
{
	int pool_[POOL_SIZE];
	int seed_;

	int x_mask, y_mask, t_mask;

public:

	void set_seed(int x);
	int get_seed()const { return seed_; }

	enum SmoothType
	{
		SMOOTH_DEFAULT		= 0,
		SMOOTH_LINEAR		= 1,
		SMOOTH_COSINE		= 2,
		SMOOTH_SPLINE		= 3,
		SMOOTH_CUBIC		= 4,
		SMOOTH_FAST_SPLINE	= 5,
	};

	float operator()(int salt,int x,int y=0, int t=0)const;
	float operator()(SmoothType smooth,int subseed,float x,float y=0, float t=0)const;
};

/* === E N D =============================================================== */

#endif
