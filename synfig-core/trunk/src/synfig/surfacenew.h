/* === S Y N F I G ========================================================= */
/*!	\file surfacenew.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_SURFACENEW_H
#define __SYNFIG_SURFACENEW_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <ETL/ref_count>
#include "color.h"
#include "mutex.h"
#include <map>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Surface;
class SurfaceChannelLock;
class SurfaceChannelLockConst;

//! \writeme
enum SurfaceColorSystem
{
	COLORSYS_RGB,
	COLORSYS_YUV,

	COLORSYS_END
}; // END of enum SurfaceColorSystem

//! \writeme
enum SurfaceChannel
{
	CHAN_A,
	CHAN_R,
	CHAN_G,
	CHAN_B,

	CHAN_Y,
	CHAN_U,
	CHAN_V,

	CHAN_END
}; // END of enum SurfaceChannel

class SurfaceNew : etl::shared_object
{
	friend class SurfaceChannelLock;

	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	//! \writeme
	typedef etl::handle<SurfaceNew> Handle;

	//! \writeme
	typedef etl::handle<const SurfaceNew> HandleConst;

	//! \writeme
	typedef etl::loose_handle<SurfaceNew> LooseHandle;

	//! \writeme
	typedef SurfaceChannel;

	//! \writeme
	typedef SurfaceChannelLock ChannelLock;

	//! \writeme
	typedef SurfaceChannelLockConst ChannelLockConst;

	//! \writeme
	typedef SurfaceColorSystem;

	//! \writeme
	class Lock
	{
		Handle x;
	public:
		Lock(const Handle& x):x(x) { x->lock(); }
		void unlock() { if(x){ x->unlock(); x=0; } }
		~Lock() { unlock(); }
	}; // END of class Lock
	friend class Lock;

private:

	//! \writeme
	class ChannelData;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	//! \writeme
	RecMutex mutex_;

	//! \writeme
	int w_,h_;

	//! \writeme
	ColorSystem color_system_;

	//! \writeme
	bool premult_flag_;

	//! \writeme
	std::map<Channel,ChannelData> channel_map_;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

protected:

	//! \writeme
	SurfaceNew();

public:

	//! \writeme
	virtual ~SurfaceNew();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! \writeme
	int get_w()const;

	//! \writeme
	int get_h()const;

	//! \writeme
	void set_wh(int w, int h);

	//! \writeme
	ColorSystem get_color_system()const;

	//! \writeme
	void set_color_system(ColorSystem x);

	//! Should only be used in certain circumstances
	Color get_color(int x, int y)const;

	//! \writeme
	void lock();

	//! \writeme
	void unlock();

	//! \writeme
	bool trylock();

	//! \writeme
	ChannelLock lock_channel(Channel chan);

	//! \writeme
	ChannelLockConst lock_channel_const(Channel chan)const;

	//! \writeme
	ChannelLock lock_channel_alpha(Channel chan);

	//! \writeme
	ChannelLockConst lock_channel_alpha_const(Channel chan)const;

	//! \writeme
	bool is_channel_defined(Channel chan)const;

	//! \writeme
	bool get_premult()const;

	//! \writeme
	void set_premult();

	/*
 --	** -- S T A T I C   F U N C T I O N S -------------------------------------
	*/

public:

	//! Normal SurfaceNew Constructor
	static Handle create(int w=0, int h=0, ColorSystem sys=COLORSYS_RGB);

	//! Converts an old Surface to a SurfaceNew
	static Handle create(const Surface&);

	//! Duplicates a surface
	static Handle create(HandleConst);

	//! Creates a cropped copy of a surface
	static Handle crop(HandleConst, int x, int y, int w, int h);

	static void blit(
		Handle dest,
		int x_dest,
		int y_dest,
		HandleConst src,
		float amount=1.0,
		Color::BlendMethod bm=Color::BLEND_COMPOSITE
	);

	static void blit(
		Handle dest,
		int x_dest,
		int y_dest,
		Handle src,
		int x_src,
		int y_src,
		int w_src,
		int h_src,
		float amount=1.0,
		Color::BlendMethod bm=Color::BLEND_COMPOSITE
	);


	static void chan_mlt(ChannelLock& dest, float x);
	static void chan_mlt(ChannelLock& dest, const ChannelLockConst& x);

	static void chan_div(ChannelLock& dest, float x);
	static void chan_div(ChannelLock& dest, const ChannelLockConst& x);

	static void chan_add(ChannelLock& dest, float x);
	static void chan_add(ChannelLock& dest, const ChannelLockConst& x);

	static void chan_sub(ChannelLock& dest, float x);
	static void chan_sub(ChannelLock& dest, const ChannelLockConst& x);
}; // END of class SurfaceNew

//! \writeme
class SurfaceChannelLockConst
{
	friend class SurfaceNew;

	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

protected:

	//! \writeme
	SurfaceNew::Handle surface_;

	//! \writeme
	etl::reference_counter ref_count_;

	//! \writeme
	SurfaceChannel channel_;

	//! \writeme
	bool data_ptr_checked_out_;

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:

	SurfaceChannelLockConst();

	//! \writeme
	~SurfaceChannelLockConst();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! \writeme
	SurfaceChannel get_channel()const;

	//! \writeme
	int get_w()const;

	//! \writeme
	int get_h()const;

	//! \writeme
	float get_value(int x, int y);

	//! \writeme
	const float* get_data_ptr()const;

	//! \writeme
	int get_data_ptr_stride()const;

	//! Releases the pointer obtained with get_data_ptr()
	void release_data_ptr()const;

	//! \writeme
	operator bool()const;
}; // END of class SurfaceChannelLockConst


//! \writeme
class SurfaceChannelLock : public SurfaceChannelLockConst
{
	friend class SurfaceNew;

	using SurfaceChannelLock::get_data_ptr;

	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:

	//! \writeme
	SurfaceChannelLock();

	//! \writeme
	~SurfaceChannelLock();

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! \writeme
	void clear();

	//! \writeme
	void fill(float value);

	//! \writeme
	void set_value(int x, int y, float v);

	float* get_data_ptr();
}; // END of class ChannelLock


}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
