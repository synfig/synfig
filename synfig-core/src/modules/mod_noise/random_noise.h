/* === S Y N F I G ========================================================= */
/*!	\file mod_noise/random_noise.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RANDOM_NOISE_H
#define __SYNFIG_RANDOM_NOISE_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */


#define POOL_SIZE	(256)
class RandomNoise
{
	int seed_;
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

	float operator()(int subseed,int x,int y=0, int t=0)const;
	float operator()(SmoothType smooth,int subseed,float x,float y=0,float t=0,int loop=0)const;
};

/* === E N D =============================================================== */

#endif
