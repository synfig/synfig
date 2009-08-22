/* === S Y N F I G ========================================================= */
/*!	\file surfacenew.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "surfacenew.h"
#include <ETL/ref_count>
#include "mutex.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

class SurfaceNew::ChannelData
{
private:
	etl::reference_counter ref_count_;

	float* data_;
	float* origin_;

	int w_,h_,stride_;

public:

	RWLock rw_lock;

	ChannelData():
		is_defined_(false),
		data_(0),
		origin_(0),
		w_(0),
		h_(0)
	{
	}

	~ChannelData()
	{
		if(ref_count_.unique())
			delete [] data_;
	}

	void set_wh(int w, int h)
	{
		w_=w;
		h_=h;
		stride_=w;

		if(data_&&ref_count_.is_unique())
			delete [] data_;

		ref_count.make_unique();
		data_=new float [w_*h_];
		origin_=data_;
		clear();
	}

	void crop(int x, int y, int w, int h)
	{
		origin_=origin+y*stride_+x;
		w_=w;
		h_=h;
	}

	int get_stride()const
	{
		return stride_;
	}

	void clear()
	{
		for(int i=0;i<h;i++)
			bzero(origin_+stride_*i,w_*sizeof(float));
	}

	void fill(float v)
	{
		float* ptr(get_data());

		for(int y=0;y<h;y++,ptr+=stride_)
			for(int i=0;i<w_;i++)
				ptr[i]=v;
	}

	float* get_data() { return origin_; }

	void make_unique()
	{
		if(!ref_count_.unique())
		{
			ref_count_.make_unique();
			float* old_data(origin_);
			int old_stride;

			data_=new float [w_*h_];
			origin_=data_;
			stride_=w_;

			for(int i=0;i<h;i++)
				memcpy(data_+i*stride_,old_data+i*old_stride,sizeof(float)*w_);
		}
	}
}; // END of class ChannelData

/* === M E T H O D S ======================================================= */

SurfaceNew::SurfaceNew():
	w_(0),
	h_(0),
	color_system_(COLORSYS_RGB),
	premult_flag_(false)
{
}

SurfaceNew~SurfaceNew()
{
}

SurfaceNew::Handle
SurfaceNew::create(int w, int h, ColorSystem sys=COLORSYS_RGB)
{
	Handle ret(new SurfaceNew);

	ret.set_wh(w,h);
	ret.set_color_system(sys);

	return ret;
}

SurfaceNew::Handle
SurfaceNew::create(const Surface&)
{
	// ***WRITEME***
	return 0;
}

SurfaceNew::Handle
SurfaceNew::create(HandleConst orig)
{
	Lock lock(orig);

	Handle ret(new SurfaceNew);

	ret.w_=orig.w_;
	ret.h_=orig.h_;
	ret.color_system_=orig.color_system_;
	ret.premult_flag_=orig.premult_flag_;
	ret.channel_map_=orig.channel_map_;

	return ret;
}

Handle
SurfaceNew::crop(HandleConst, int x, int y, int w, int h)
{
	Lock lock(orig);

	Handle ret(new SurfaceNew);

	ret.w_=orig.w_;
	ret.h_=orig.h_;
	ret.color_system_=orig.color_system_;
	ret.premult_flag_=orig.premult_flag_;
	ret.channel_map_=orig.channel_map_;

	std::map<Channel,ChannelData>::iterator iter;
	for(iter=ret.channel_map_.begin();iter!=ret.channel_map_.end();++iter)
		iter->crop(x,y,w,h);

	return ret;
}

int
SurfaceNew::get_w()const
{
	return w_;
}

int
SurfaceNew::get_h()const
{
	return h_;
}

void
SurfaceNew::set_wh(int w, int h)
{
	if(w!=w_ || h!=h_)
	{
		w_=w;
		h_=h;
		channel_map_.clear();
	}
}

SurfaceNew::ColorSystem
SurfaceNew::get_color_system()const
{
	return color_system_;
}

void
SurfaceNew::set_color_system(SurfaceNew::ColorSystem x)
{
	color_system_=x;
}

Color
SurfaceNew::get_color(int x, int y)const
{
	// This operation is rather expensive, as it should be.
	// I want to discourage people from using it all over the place.

	Color ret(
		lock_channel_const(CHAN_R).get_value(x,y),
		lock_channel_const(CHAN_G).get_value(x,y),
		lock_channel_const(CHAN_B).get_value(x,y),
		lock_channel_const(CHAN_A).get_value(x,y)
	);

	if(get_premult())
	{
		ret=ret.demult_alpha();
	}

	return ret;
}

void
SurfaceNew::lock()
{
	mutex_.lock();
}

void
SurfaceNew::unlock()
{
	mutex_.unlock();
}

bool
SurfaceNew::trylock()
{
	return mutex_.trylock();
}

SurfaceNew::ChannelLock
SurfaceNew::lock_channel(SurfaceNew::Channel chan)
{
	if(!is_channel_defined(chan)
		channel_map_[chan].set_wh(get_w(),get_h());
	else
		channel_map_[chan].make_unique();

	ChannelLockConst channel_lock;

	channel_lock.surface_=this;
	channel_lock.channel_=chan;

	channel_map_[chan].rw_lock.writer_lock();

	return channel_lock;
}

SurfaceNew::ChannelLockConst
SurfaceNew::lock_channel_const(SurfaceNew::Channel chan)const
{
	if(!is_channel_defined(chan)
		channel_map_[chan].set_wh(get_w(),get_h());

	ChannelLockConst channel_lock;

	channel_lock.surface_=this;
	channel_lock.channel_=chan;

	channel_map_[chan].rw_lock.reader_lock();

	return channel_lock;
}

SurfaceNew::ChannelLock
SurfaceNew::lock_channel_alpha(SurfaceNew::Channel chan)
{
	// Change this when per-channel alpha
	// is implemented
	return lock_channel(CHAN_A);
}

SurfaceNew::ChannelLockConst
SurfaceNew::lock_channel_alpha_const(SurfaceNew::Channel chan)const
{
	// Change this when per-channel alpha
	// is implemented
	return lock_channel_const(CHAN_A);
}

bool
SurfaceNew::is_channel_defined(Channel chan)const
{
	return channel_map_.count(chan);
}

bool
SurfaceNew::get_premult()const
{
	return premult_flag_;
}

void
SurfaceNew::set_premult(bool x)
{
	if(x==premult_flag_)
		return;

	premult_flag_=x;

	for(int i=0;i<3;i++)
	{
		Channel chan;
		if(get_color_system()==COLORSYS_RGB)switch(i)
		{
			case 0: chan=CHAN_R;
			case 1: chan=CHAN_G;
			case 2: chan=CHAN_B;
		}
		else
		if(get_color_system()==COLORSYS_YUV)switch(i)
		{
			case 0: chan=CHAN_Y;
			case 1: chan=CHAN_U;
			case 2: chan=CHAN_V;
		}

		// If this channel isn't defined, then
		// skip it and move on to the next one
		if(!is_channel_defined(chan))
			continue;

		ChannelLock color_channel(lock_channel(chan));
		ChannelLockConst alpha_channel(lock_channel_alpha_const(chan));
		const int w(get_w());
		const int h(get_h());

		float* color_ptr(color_channel.get_data_ptr());
		const float* alpha_ptr(alpha_channel.get_data_ptr());

		const int color_pitch(color_channel.get_data_ptr_stride()-w);
		const int alpha_pitch(alpha_channel.get_data_ptr_stride()-w);

		if(premult_flag_)
		{
			for(int y=0;y<h;y++,color_ptr+=color_pitch,alpha_ptr+=alpha_pitch)
				for(int x=0;x<w;x++,color_ptr++,alpha_ptr++)
					*color_ptr *= *alpha_ptr;
		}
		else
		{
			for(int y=0;y<h;y++,color_ptr+=color_pitch,alpha_ptr+=alpha_pitch)
				for(int x=0;x<w;x++,color_ptr++,alpha_ptr++)
					*color_ptr /= *alpha_ptr;
		}
	}
}

void
SurfaceNew::blit(
	Handle dest, int x_dest, int y_dest,
	HandleConst src, int x_src, int y_src, int w_src, int h_src,
	float amount=1.0, Color::BlendMethod bm=Color::BLEND_COMPOSITE
)
{
	blit(
		dest,
		x_dest,
		y_dest,
		crop(
			src,
			x,
			y,
			w,
			h
		),
		amount,
		bm
	);
}

void
SurfaceNew::blit(
	Handle dest, int x_dest, int y_dest,
	HandleConst src
	float amount=1.0, Color::BlendMethod bm=Color::BLEND_COMPOSITE
)
{
	int w(src->get_w()), h(src->get_h);

	// Clip
	{
		int x(0), y(0);

		if(x_dest+w>dest.get_w())
			w=dest.get_w()-x_dest;
		if(y_dest+h>dest.get_h())
			h=dest.get_h()-y_dest;
		if(x_dest<0)
		{
			x-=x_dest;
			w+=x_dest;
		}
		if(y_dest<0)
		{
			y-=y_dest;
			h+=y_dest;
		}
		src=crop(src,x,y,w,h);
	}

	dest=crop(dest,x_dest,y_dest,w,h);

	if(bm==Color::BLEND_STRAIGHT)
	{
		chan_mlt(dest,amount/(1.0-amount));
		chan_add(dest,src);
		chan_mlt(dest,(1.0-amount)/amount);
	}

	if(bm==Color::BLEND_COMPOSITE)
	{

	}
}



// -----------------------------------------------------------------------------------

SurfaceChannelLockConst::SurfaceChannelLockConst():
	data_ptr_checked_out_(false)
{
}

SurfaceChannelLockConst::~SurfaceChannelLockConst()
{
	if(data_ptr_checked_out_)
		release_data_ptr();

	if(surface_ && ref_count_.is_unique())
		return surface->channel_map_[channel_].rw_lock.reader_unlock();
	surface=0;
}

SurfaceChannel
SurfaceChannelLockConst::get_channel()const
{
	return channel_;
}

int
SurfaceChannelLockConst::get_w()const
{
	return surface_->get_w();
}

int
SurfaceChannelLockConst::get_h()const
{
	return surface_->get_h();
}

float
SurfaceChannelLockConst::get_value(int x, int y)
{
	// WOW! CRAZY SLOW!
	const ChannelData& channel_data(surface_->channel_map_[channel_]);
	return *(channel_data.get_data()+y*channel_data.get_stride()+x);
}

const float*
SurfaceChannelLockConst::get_data_ptr()const
{
	data_ptr_checked_out_=true;

	// WOW! CRAZY SLOW!
	return surface_->channel_map_[channel_].get_data();
}

int
SurfaceChannelLockConst::get_data_ptr_stride()const
{
	return surface_->channel_map_[channel_].get_stride();
}

void
SurfaceChannelLockConst::release_data_ptr()const
{
	data_ptr_checked_out_=false;
}

SurfaceChannelLockConst::operator bool()const
{
	return static_cast<bool>(surface_);
}

// -----------------------------------------------------------------------------------

SurfaceChannelLock::SurfaceChannelLock()
{
}

SurfaceChannelLock::~SurfaceChannelLock()
{
	if(data_ptr_checked_out_)
		release_data_ptr();

	if(surface_ && ref_count_.is_unique())
		return surface_->channel_map_[channel_].rw_lock.writer_unlock();
	surface=0;
}

void
SurfaceChannelLock::clear()
{
	return surface_->channel_map_[channel_].clear();
}

void
SurfaceChannelLock::fill(float v)
{
	return surface_->channel_map_[channel_].fill(v);
}

void
SurfaceChannelLock::set_value(int x, int y, float v)
{
	// WOW! CRAZY SLOW!
	const ChannelData& channel_data(surface_->channel_map_[channel_]);
	*(channel_data.get_data()+y*channel_data.get_stride()+x)=v;
}

float*
SurfaceChannelLock::get_data_ptr()
{
	data_ptr_checked_out_=true;

	// WOW! CRAZY SLOW!
	return surface_->channel_map_[channel_].get_data();
}



// -----------------------------------------------------------------------------------



void
SurfaceNew::chan_mlt(ChannelLock& dest, float x)
{
	float* ptr(dest.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int pitch(dest.get_data_pitch()-w);

	int(y=0;y<h;y++,ptr+=pitch)
		int(x=0;x<w;x++,ptr++)
			*ptr*=x;
}

void
SurfaceNew::chan_mlt(ChannelLock& dest, const ChannelLockConst& x)
{
	float* d_ptr(dest.get_data_ptr());
	const float* s_ptr(x.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int d_pitch(dest.get_data_stride()-w);
	const int s_pitch(x.get_data_stride()-w);

	int(y=0;y<h;y++,d_ptr+=d_pitch,s_ptr+=s_pitch)
		int(x=0;x<w;x++,d_ptr++,s_ptr++)
			*d_ptr *= *s_ptr;
}

void
SurfaceNew::chan_div(ChannelLock& dest, float x)
{
	float* ptr(dest.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int pitch(dest.get_data_pitch()-w);

	int(y=0;y<h;y++,ptr+=pitch)
		int(x=0;x<w;x++,ptr++)
			*ptr/=x;
}

void
SurfaceNew::chan_div(ChannelLock& dest, const ChannelLockConst& x)
{
	float* d_ptr(dest.get_data_ptr());
	const float* s_ptr(x.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int d_pitch(dest.get_data_stride()-w);
	const int s_pitch(x.get_data_stride()-w);

	int(y=0;y<h;y++,d_ptr+=d_pitch,s_ptr+=s_pitch)
		int(x=0;x<w;x++,d_ptr++,s_ptr++)
			*d_ptr /= *s_ptr;
}

void
SurfaceNew::chan_add(ChannelLock& dest, float x)
{
	float* ptr(dest.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int pitch(dest.get_data_pitch()-w);

	int(y=0;y<h;y++,ptr+=pitch)
		int(x=0;x<w;x++,ptr++)
			*ptr+=x;
}

void
SurfaceNew::chan_add(ChannelLock& dest, const ChannelLockConst& x)
{
	float* d_ptr(dest.get_data_ptr());
	const float* s_ptr(x.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int d_pitch(dest.get_data_stride()-w);
	const int s_pitch(x.get_data_stride()-w);

	int(y=0;y<h;y++,d_ptr+=d_pitch,s_ptr+=s_pitch)
		int(x=0;x<w;x++,d_ptr++,s_ptr++)
			*d_ptr += *s_ptr;
}

void
SurfaceNew::chan_sub(ChannelLock& dest, float x)
{
	float* ptr(dest.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int pitch(dest.get_data_pitch()-w);

	int(y=0;y<h;y++,ptr+=pitch)
		int(x=0;x<w;x++,ptr++)
			*ptr-=x;
}

void
SurfaceNew::chan_sub(ChannelLock& dest, const ChannelLockConst& x)
{
	float* d_ptr(dest.get_data_ptr());
	const float* s_ptr(x.get_data_ptr());
	const int w(dest.get_w());
	const int h(dest.get_h());
	const int d_pitch(dest.get_data_stride()-w);
	const int s_pitch(x.get_data_stride()-w);

	int(y=0;y<h;y++,d_ptr+=d_pitch,s_ptr+=s_pitch)
		int(x=0;x<w;x++,d_ptr++,s_ptr++)
			*d_ptr -= *s_ptr;
}



