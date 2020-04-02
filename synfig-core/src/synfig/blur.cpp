/* === S Y N F I G ========================================================= */
/*!	\file synfig/blur.cpp
**	\brief Blur Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#include <stdexcept>

#include <ETL/boxblur>
#include <ETL/gaussian>

#include "blur.h"

#include "general.h"
#include <synfig/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::Real
Blur::get_size_amplifier(int type)
{
	// measured values
	switch(type)
	{
	case Blur::BOX:
		return 19.362962/9.182808*19.362962/20.634363;
	case Blur::FASTGAUSSIAN:
		return 20.297409/6.309251*20.297409/21.081510*20.297409/21.513471;
	case Blur::CROSS:
		return 19.362962/9.182808*19.362962/20.634363;
	//case Blur::GAUSSIAN:
	//	return 20.297409/0.417297*20.297409/9.851342*20.297409/12.328201;
	case Blur::DISC:
		return 17.821498/8.778783*17.821498/17.640771;
	}
	return 1.0;
}


Point Blur::operator()(const Point &pos) const
{
	Point blurpos(pos);

	switch(type)
	{
	case CROSS:
		if(rand()%2)
		{
			if(size[0])
				blurpos[0]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[0];
		}
		else
		{
			if(size[1])
				blurpos[1]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[1];
		}
		break;

	case DISC:
		{
			Angle theta=Angle::rotations((float)rand()/(float)RAND_MAX);
			Vector::value_type mag=(float)rand()/(float)RAND_MAX;
			Vector vect((float)Angle::cos(theta).get()*mag,(float)Angle::sin(theta).get()*mag);

			blurpos[0]+=vect[0]*size[0];
			blurpos[1]+=vect[1]*size[1];
		}
		break;

	case FASTGAUSSIAN:
	case GAUSSIAN:
		// Not quite a true gaussian blur,
		// but the results are close enough for me.
		if(size[0])
		{
			blurpos[0]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[0]*3/4;
			blurpos[0]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[0]*3/4;
		}
		if(size[1])
		{
			blurpos[1]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[1]*3/4;
			blurpos[1]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[1]*3/4;
		}
		break;

	case BOX:
	default:
		if(size[0])
			blurpos[0]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[0];
		if(size[1])
			blurpos[1]+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) * size[1];
		break;
	}

	return blurpos;
}

Point Blur::operator()(synfig::Real x, synfig::Real y) const
{
	return (*this)(Point(x,y));
}

//blur functions to make my life easier

template <typename T>
static inline T zero()
{
	return (T)0;
}

template <>
inline Color zero<Color>()
{
	return Color::alpha();
}

template <>
inline CairoColorAccumulator zero<CairoColorAccumulator>()
{
	return CairoColorAccumulator(0);
}

template <typename T,typename AT,class VP>
static void GaussianBlur_2x2(etl::surface<T,AT,VP> &surface)
{
	int x,y,w,h;
	AT Tmp1,Tmp2,SR0;

	w=surface.get_w();
	h=surface.get_h();
	
	AT *SC0=new AT[w];

	memcpy(SC0,surface[0],w*sizeof(AT));

	for(y=0;y<h;y++)
	{
		SR0=surface[y][0];
		for(x=0;x<w;x++)
		{
			Tmp1=(AT)(surface[y][x]);
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			surface[y][x]=(SC0[x]+Tmp2)/4;
			SC0[x]=Tmp2;
		}
	}
	delete [] SC0;
}

template <typename T,typename AT,class VP>
static void GaussianBlur_3x3(etl::surface<T,AT,VP> &surface)
{
	int x,y,u,v,w,h;
	AT Tmp1,Tmp2,SR0,SR1;

	w=surface.get_w();
	h=surface.get_h();

	AT *SC0=new AT[w+1];
	AT *SC1=new AT[w+1];

	// Setup the row buffers
	for(x=0;x<w;x++)SC0[x]=(AT)(surface[0][x])*4;

	for(y=0;y<=h;y++)
	{
		if(y>=h)
			v=h-1;
		else
			v=y;

		SR0=SR1=surface[y][0];
		for(x=0;x<=w;x++)
		{
			if(x>=w)
				u=w-1;
			else
				u=x;

			// Row Machine
			Tmp1=surface[v][u];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			Tmp1=SR1+Tmp2;
			SR1=Tmp2;

			// Column Machine
			Tmp2=SC0[x]+Tmp1;
			SC0[x]=Tmp1;
			if(y&&x)
				surface[y-1][x-1]=(SC1[x]+Tmp2)/16;
			SC1[x]=Tmp2;
		}
	}

	delete [] SC0;
	delete [] SC1;
}

template <typename T,typename AT,class VP>
inline static void GaussianBlur_5x5_(etl::surface<T,AT,VP> &surface,AT *SC0,AT *SC1,AT *SC2,AT *SC3)
{
	int x,y,u,v,w,h;
	AT Tmp1,Tmp2,SR0,SR1,SR2,SR3;

	w=surface.get_w();
	h=surface.get_h();

	// Setup the row buffers
	for(x=0;x<w;x++)SC0[x+2]=(AT)(surface[0][x])*24;

	for(y=0;y<h+2;y++)
	{
		if(y>=h)
			v=h-1;
		else
			v=y;

		SR0=SR1=SR2=SR3=0;
		SR0=(AT)(surface[v][0])*1.5;
		for(x=0;x<w+2;x++)
		{
			if(x>=w)
				u=w-1;
			else
				u=x;

			// Row Machine
			Tmp1=surface[v][u];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			Tmp1=SR1+Tmp2;
			SR1=Tmp2;
			Tmp2=SR2+Tmp1;
			SR2=Tmp1;
			Tmp1=SR3+Tmp2;
			SR3=Tmp2;

			// Column Machine
			Tmp2=SC0[x]+Tmp1;
			SC0[x]=Tmp1;
			Tmp1=SC1[x]+Tmp2;
			SC1[x]=Tmp2;
			Tmp2=SC2[x]+Tmp1;
			SC2[x]=Tmp1;
			if(y>1&&x>1)
				surface[y-2][x-2]=(SC3[x]+Tmp2)/256;
			SC3[x]=Tmp2;
		}
	}

}

template <typename T,typename AT,class VP>
inline static void GaussianBlur_5x5(etl::surface<T,AT,VP> &surface)
{
	int w2=surface.get_w() + 2;

	AT *SC0=new AT[w2];
	AT *SC1=new AT[w2];
	AT *SC2=new AT[w2];
	AT *SC3=new AT[w2];

	GaussianBlur_5x5_(surface,SC0,SC1,SC2,SC3);

	delete [] SC0;
	delete [] SC1;
	delete [] SC2;
	delete [] SC3;
}

template <typename T,typename AT,class VP>
static void GaussianBlur_nxn(etl::surface<T,AT,VP> &surface,int n)
{
	int x,y,u,v,w,h;
	int half_n=n/2,i;
	float inv_divisor=pow(2.0,(n-1));
	AT Tmp1,Tmp2;
	inv_divisor=1.0/(inv_divisor*inv_divisor);

	w=surface.get_w();
	h=surface.get_h();
	int w_half_n=w+half_n;
    AT SR[n-1];
	AT *SC[n-1];

	for(i=0;i<n-1;i++)
	{
		SC[i]=new AT[w_half_n];
		if(!SC[i])
		{
			throw(runtime_error(strprintf(__FILE__":%d:Malloc failure",__LINE__)));
			return;
		}
	}

	// Setup the first row
//	for(x=0;x<w;x++)SC[0][x+half_n]=surface[0][x]*550.0;//*pow(2.0,(n-1))*(2.0/n);

	for(y=0;y<h+half_n;y++)
	{
		if(y>=h)
			v=h-1;
		else
			v=y;

		if(y!=0)
			memset(SR,0,(n-1)*sizeof(AT));

//		SR[0]=surface[v][0]*(2.0-1.9/n);

		for(x=0;x<w_half_n;x++)
		{
			if(x>=w)
				u=w-1;
			else
				u=x;

			Tmp1=surface[v][u];
			// Row Machine
			for(i=0;i<half_n;i++)
			{
				int idouble = i*2;
				Tmp2=SR[idouble]+Tmp1;
				SR[idouble]=Tmp1;
				Tmp1=SR[idouble+1]+Tmp2;
				SR[idouble+1]=Tmp2;
			}

			// Column Machine
			for(i=0;i<half_n-1;i++)
			{
				int idouble = i*2;
				Tmp2=SC[idouble][x]+Tmp1;
				SC[idouble][x]=Tmp1;
				Tmp1=SC[idouble+1][x]+Tmp2;
				SC[idouble+1][x]=Tmp2;
			}
			Tmp2=SC[n-3][x]+Tmp1;
			SC[n-3][x]=Tmp1;
			if(y>=half_n&&x>=half_n)
				surface[y-half_n][x-half_n]=(SC[n-2][x]+Tmp2)*inv_divisor;
			SC[n-2][x]=Tmp2;
		}
	}

	for(i=0;i<n-1;i++)
		delete [] SC[i];
}

template <typename T,typename AT,class VP>
static void GaussianBlur_2x1(etl::surface<T,AT,VP> &surface)
{
	int x,y,w,h;
	AT Tmp1,Tmp2,SR0;

	w = surface.get_w();
	h = surface.get_h();
	
	for(y=0;y<h;y++)
	{
		SR0=surface[y][0];
		for(x=0;x<w;x++)
		{
			Tmp1=surface[y][x];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			surface[y][x]=(Tmp2)/2;
		}
	}
}

template <typename T,typename AT,class VP>
static void GaussianBlur_3x1(etl::surface<T,AT,VP> &surface)
{
	int x,y,w,h;
	AT Tmp1,Tmp2,SR0,SR1;
	w = surface.get_w();
	h = surface.get_h();
	
	for(y=0;y<h;y++)
	{
		SR0=SR1=surface[y][0];
		for(x=0;x<w;x++)
		{
			// Row Machine
			Tmp1=surface[y][x];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			Tmp1=SR1+Tmp2;
			SR1=Tmp2;

			if(x)
				surface[y][x-1]=(Tmp1)/4;
		}
	}
}

template <typename T,typename AT,class VP>
static void GaussianBlur_1x2(etl::surface<T,AT,VP> &surface)
{
	int x,y;
	AT Tmp1,Tmp2,SR0;

	for(x=0;x<surface.get_w();x++)
	{
		SR0 = zero<AT>();
		for(y=0;y<surface.get_h();y++)
		{
			Tmp1=surface[y][x];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			surface[y][x]=(Tmp2)/2;
		}
	}
}

template <typename T,typename AT,class VP>
static void GaussianBlur_1x3(etl::surface<T,AT,VP> &surface)
{
	int x,y;
	AT Tmp1,Tmp2,SR0,SR1;

	for(x=0;x<surface.get_w();x++)
	{
		SR0=SR1=surface[0][x];
		for(y=0;y<surface.get_h();y++)
		{
			// Row Machine
			Tmp1=surface[y][x];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			Tmp1=SR1+Tmp2;
			SR1=Tmp2;

			if(y)
				surface[y-1][x]=(Tmp1)/4;
		}
	}
}

//THE GOOD ONE!!!!!!!!!
bool Blur::operator()(const Surface &surface,
					  const Vector &resolution,
					  Surface &out) const
{
	int w = surface.get_w(),
		h = surface.get_h();

	if(w == 0 || h == 0 || resolution[0] == 0 || resolution[1] == 0) return false;

	const Real	pw = resolution[0]/w,
				ph = resolution[1]/h;

	int	halfsizex = (int) (abs(size[0]*.5/pw) + 1),
		halfsizey = (int) (abs(size[1]*.5/ph) + 1);

	int x,y;

	SuperCallback blurcall(cb,0,5000,5000);

	Surface worksurface(w,h);

	//synfig::info("Blur: check surface = %s", surface_valid(surface)?"true":"false");

	// Premultiply the alpha
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			Color a = surface[y][x];
			float aa = a.get_a();
			a.set_r(a.get_r()*aa);
			a.set_g(a.get_g()*aa);
			a.set_b(a.get_b()*aa);
			worksurface[y][x] = a;
		}
	}

	switch(type)
	{
	case Blur::DISC:	// D I S C ----------------------------------------------------------
		{
			int bw = halfsizex;
			int bh = halfsizey;

			if(size[0] && size[1] && w*h>2)
			{
				int x2,y2;
				Surface tmp_surface(worksurface);

				for(y=0;y<h;y++)
				{
					for(x=0;x<w;x++)
					{
						//accumulate all the pixels in an ellipse of w,h about the current pixel
						Color color=Color::alpha();
						int total=0;

						for(y2=-bh;y2<=bh;y2++)
						{
							for(x2=-bw;x2<=bw;x2++)
							{
								//get the floating point distance away from the origin pixel in relative coords
								float tmp_x=(float)x2/bw;
								float tmp_y=(float)y2/bh;
								tmp_x*=tmp_x;
								tmp_y*=tmp_y;

								//ignore if it's outside of the disc
								if( tmp_x+tmp_y>1.0)
									continue;

								//cap the pixel indices to inside the surface
								int u= x+x2,
									v= y+y2;

								if( u < 0 )					u = 0;
								if( u >= w ) u = w-1;

								if( v < 0 ) 				v = 0;
								if( v >= h ) v = h-1;

								//accumulate the color, and # of pixels added in
								color += tmp_surface[v][u];
								total++;
							}
						}

						//blend the color with the original color
						//if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
							worksurface[y][x]=color/total;
						//else
						//	worksurface[y][x]=Color::blend(color/total,tmp_surface[y][x],get_amount(),get_blend_method());
					}
					if(!blurcall.amount_complete(y,h))
					{
						//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
						return false;
					}
				}
				break;
			}

			//if we don't qualify for disc blur just use box blur
		}

	case Blur::BOX: // B O X -------------------------------------------------------
		{
			//horizontal part
			//synfig::info("Blur: Starting Box blur (surface valid %d)", (int)surface_valid(worksurface));

			Surface temp_surface;
			temp_surface.set_wh(w,h);

			if(size[0])
			{
				int length = halfsizex;
				length=std::max(1,length);

				//synfig::info("Blur: hbox blur work -> temp %d", length);
				etl::hbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface.begin());
			}
			else temp_surface = worksurface;
			//synfig::info("Blur: hbox finished");

			//vertical part
			//Surface temp_surface2;
			//temp_surface2.set_wh(w,h);

			if(size[1])
			{
				int length = halfsizey;
				length = std::max(1,length);

				//synfig::info("Blur: vbox blur temp -> work %d",length);
				etl::vbox_blur(temp_surface.begin(),temp_surface.end(),length,worksurface.begin());
			}
			else worksurface = temp_surface;
			//synfig::info("Blur: vbox finished");

			//blend with the original surface
			/*int x,y;
			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					worksurface[y][x]=temp_surface2[y][x];//Color::blend(temp_surface2[y][x],worksurface[y][x],get_amount(),get_blend_method());
				}
			}*/
		}
		break;

	case Blur::FASTGAUSSIAN:	// F A S T G A U S S I A N ----------------------------------------------
		{
			//fast gaussian is treated as a 3x3 type of thing, except expanded to work with the length

			/*	1	2	1
				2	4	2
				1	2	1
			*/

			Surface temp_surface;
			temp_surface.set_wh(w,h);

			//Surface temp_surface2;
			//temp_surface2.set_wh(w,h);

			//horizontal part
			if(size[0])
			{
				Real length=abs((float)w/(resolution[0]))*size[0]*0.5+1;
				length=std::max(1.0,length);

				//two box blurs produces: 1 2 1
				etl::hbox_blur(worksurface.begin(),w,h,(int)(length*3/4),temp_surface.begin());
				etl::hbox_blur(temp_surface.begin(),w,h,(int)(length*3/4),worksurface.begin());
			}
			//else temp_surface2=worksurface;

			//vertical part
			if(size[1])
			{
				Real length=abs((float)h/(resolution[1]))*size[1]*0.5+1;
				length=std::max(1.0,length);

				//two box blurs produces: 1 2 1 on the horizontal 1 2 1
				etl::vbox_blur(worksurface.begin(),w,h,(int)(length*3/4),temp_surface.begin());
				etl::vbox_blur(temp_surface.begin(),w,h,(int)(length*3/4),worksurface.begin());
			}
			//else temp_surface2=temp_surface2;

			/*int x,y;
			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					worksurface[y][x]=temp_surface2[y][x];//Color::blend(temp_surface2[y][x],worksurface[y][x],get_amount(),get_blend_method());
				}
			}*/
		}
		break;

	case Blur::CROSS: // C R O S S  -------------------------------------------------------
		{
			//horizontal part
			Surface temp_surface;
			temp_surface.set_wh(worksurface.get_w(),worksurface.get_h());

			if(size[0])
			{
				int length = halfsizex;
				length = std::max(1,length);

				etl::hbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface.begin());
			}
			else temp_surface = worksurface;

			//vertical part
			Surface temp_surface2;
			temp_surface2.set_wh(worksurface.get_w(),worksurface.get_h());

			if(size[1])
			{
				int length = halfsizey;
				length = std::max(1,length);

				etl::vbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface2.begin());
			}
			else temp_surface2 = worksurface;

			//blend the two together
			int x,y;

			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					worksurface[y][x] = (temp_surface[y][x]+temp_surface2[y][x])/2;//Color::blend((temp_surface[y][x]+temp_surface2[y][x])/2,worksurface[y][x],get_amount(),get_blend_method());
				}
			}

			break;
		}

	case Blur::GAUSSIAN:	// G A U S S I A N ----------------------------------------------
		{
			#ifndef	GAUSSIAN_ADJUSTMENT
			#define GAUSSIAN_ADJUSTMENT		(0.05)
			#endif

			Real	pw = (Real)w/(resolution[0]);
			Real 	ph = (Real)h/(resolution[1]);

			Surface temp_surface;
			Surface *gauss_surface;

			//synfig::warning("Didn't crash yet b1");

			//if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				gauss_surface = &worksurface;
			/*else
			{
				temp_surface = worksurface;
				gauss_surface = &temp_surface;
			}*/

            /* Squaring the pw and ph values
			   is necessary to insure consistent
			   results when rendered to different
			   resolutions.
			   Unfortunately, this automatically
			   squares our rendertime.
			   There has got to be a faster way...
			*/
			pw=pw*pw;
			ph=ph*ph;

			int bw = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			int bh = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
			int max=bw+bh;

			ColorAccumulator *SC0=new ColorAccumulator[w+2];
			ColorAccumulator *SC1=new ColorAccumulator[w+2];
			ColorAccumulator *SC2=new ColorAccumulator[w+2];
			ColorAccumulator *SC3=new ColorAccumulator[w+2];

			//synfig::warning("Didn't crash yet b2");
			//int i = 0;

			while(bw&&bh)
			{
				if(!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}

				if(bw>=4 && bh>=4)
				{
					etl::gaussian_blur_5x5_(gauss_surface->begin(),gauss_surface->get_w(),gauss_surface->get_h(),SC0,SC1,SC2,SC3);
					bw-=4,bh-=4;
				}
				else
				if(bw>=2 && bh>=2)
				{
					etl::gaussian_blur_3x3(gauss_surface->begin(),gauss_surface->end());
					bw-=2,bh-=2;
				}
				else
				if(bw>=1 && bh>=1)
				{
					GaussianBlur_2x2(*gauss_surface);
					bw--,bh--;
				}

				//synfig::warning("Didn't crash yet bi - %d",i++);
			}
			while(bw)
			{
				if(!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;
					return false;
				}
				if(bw>=2)
				{
					GaussianBlur_3x1(*gauss_surface);
					bw-=2;
				}
				else
				if(bw>=1)
				{
					GaussianBlur_2x1(*gauss_surface);
					bw--;
				}
				//synfig::warning("Didn't crash yet bi - %d",i++);
			}
			while(bh)
			{
				if(!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;
					return false;
				}
				if(bh>=2)
				{
					GaussianBlur_1x3(*gauss_surface);
					bh-=2;
				}
				else
				if(bh>=1)
				{
					GaussianBlur_1x2(*gauss_surface);
					bh--;
				}
				//synfig::warning("Didn't crash yet bi - %d",i++);
			}

			delete [] SC0;
			delete [] SC1;
			delete [] SC2;
			delete [] SC3;

			/*if(get_amount()!=1.0 || get_blend_method()!=Color::BLEND_STRAIGHT)
			{
				int x,y;
				for(y=0;y<renddesc.get_h();y++)
					for(x=0;x<renddesc.get_w();x++)
						worksurface[y][x]=Color::blend(temp_surface[y][x],worksurface[y][x],get_amount(),get_blend_method());
			}*/
			//synfig::warning("Didn't crash yet b end",i++);
		}
		break;

		default:
		break;
	}

	// Scale up the alpha

	//be sure the surface is of the correct size
	//surface->set_wh(renddesc.get_w(),renddesc.get_h());
	out.set_wh(w,h);

	//divide out the alpha
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			Color a = worksurface[y][x];
			if(a.get_a())
			{
				a.set_r(a.get_r()/a.get_a());
				a.set_g(a.get_g()/a.get_a());
				a.set_b(a.get_b()/a.get_a());
				out[y][x]=a;
			}
			else out[y][x]=Color::alpha();
		}
	}

	//we are FRIGGGIN done....
	blurcall.amount_complete(100,100);

	return true;
}

//////
bool Blur::operator()(cairo_surface_t *surface,
					  const Vector &resolution,
					  cairo_surface_t *out) const
{

	CairoSurface cairosurface(surface);
	if(!cairosurface.map_cairo_image())
	{
		synfig::info("cairosurface map cairo image failed");
		return false;
	}

	int w = cairosurface.get_w(),
	h = cairosurface.get_h();
	
	if(w == 0 || h == 0 || resolution[0] == 0 || resolution[1] == 0)
	{
		cairosurface.unmap_cairo_image();
		return false;
	}
	
	const Real	pw = resolution[0]/w,
	ph = resolution[1]/h;
	
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 1),
	halfsizey = (int) (abs(size[1]*.5/ph) + 1);
	
	int x,y;
	
	SuperCallback blurcall(cb,0,5000,5000);
	
	CairoSurface cairoout(out);
	if(!cairoout.map_cairo_image())
	{
		synfig::info("cairoout map cairo image failed");
		cairosurface.unmap_cairo_image();
		return false;
	}

	switch(type)
	{
		case Blur::DISC:	// D I S C ----------------------------------------------------------
		{
			int bw = halfsizex;
			int bh = halfsizey;
			
			if(size[0] && size[1] && w*h>2)
			{
				int x2,y2;
				
				for(y=0;y<h;y++)
				{
					for(x=0;x<w;x++)
					{
						//accumulate all the pixels in an ellipse of w,h about the current pixel
						Color color=Color::alpha();
						int total=0;
						
						for(y2=-bh;y2<=bh;y2++)
						{
							for(x2=-bw;x2<=bw;x2++)
							{
								//get the floating point distance away from the origin pixel in relative coords
								float tmp_x=(float)x2/bw;
								float tmp_y=(float)y2/bh;
								tmp_x*=tmp_x;
								tmp_y*=tmp_y;
								
								//ignore if it's outside of the disc
								if( tmp_x+tmp_y>1.0)
									continue;
								
								//cap the pixel indices to inside the surface
								int u= x+x2,
								v= y+y2;
								
								if( u < 0 )					u = 0;
								if( u >= w ) u = w-1;
								
								if( v < 0 ) 				v = 0;
								if( v >= h ) v = h-1;
								
								//accumulate the color, and # of pixels added in
								color += Color(cairosurface[v][u]);
								total++;
							}
						}
						
						//blend the color with the original color
						cairoout[y][x]=CairoColor(color/total);
					}
					if(!blurcall.amount_complete(y,h))
					{
						if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
						cairosurface.unmap_cairo_image();
						cairoout.unmap_cairo_image();
						return false;
					}
				}
				break;
			}
			//if we don't qualify for disc blur just use box blur
		}
			
		case Blur::BOX: // B O X -------------------------------------------------------
		{
			cairo_surface_t* temp=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, w, h);
			CairoSurface cairotemp(temp);
			if(!cairotemp.map_cairo_image())
			{
				synfig::info("cairotemp map cairo image failed");
				return false;
			}
			//horizontal part
			if(size[0])
			{
				int length = halfsizex;
				length=std::max(1,length);
				etl::hbox_blur(cairosurface.begin(),cairosurface.end(),length,cairotemp.begin());
			}
			else cairotemp.copy(cairosurface);
			
			//vertical part
			if(size[1])
			{
				int length = halfsizey;
				length = std::max(1,length);
				etl::vbox_blur(cairotemp.begin(),cairotemp.end(),length,cairoout.begin());
			}
			else cairoout.copy(cairotemp);
			cairotemp.unmap_cairo_image();
			cairo_surface_destroy(temp);
		}
			break;
			
		case Blur::FASTGAUSSIAN:	// F A S T G A U S S I A N ----------------------------------------------
		{
			//fast gaussian is treated as a 3x3 type of thing, except expanded to work with the length
			
			/*	1	2	1
			 2	4	2
			 1	2	1
			 */
			cairo_surface_t* temp=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, w, h);
			CairoSurface cairotemp(temp);
			if(!cairotemp.map_cairo_image())
			{
				synfig::info("cairotemp map cairo image failed. Fast Gaussian");
				return false;
			}
			
			//horizontal part
			if(size[0])
			{
				Real length=abs((float)w/(resolution[0]))*size[0]*0.5+1;
				length=std::max(1.0,length);
				
				//two box blurs produces: 1 2 1
				etl::hbox_blur(cairosurface.begin(),w,h,(int)(length*3/4),cairotemp.begin());
				etl::hbox_blur(cairotemp.begin(),w,h,(int)(length*3/4),cairoout.begin());
			}
			else cairoout.copy(cairosurface);
			
			// Interchange result with temp
			cairotemp.copy(cairoout);
			//vertical part
			if(size[1])
			{
				Real length=abs((float)h/(resolution[1]))*size[1]*0.5+1;
				length=std::max(1.0,length);
				
				//two box blurs produces: 1 2 1 on the horizontal 1 2 1
				etl::vbox_blur(cairoout.begin(),w,h,(int)(length*3/4),cairotemp.begin());
				etl::vbox_blur(cairotemp.begin(),w,h,(int)(length*3/4),cairoout.begin());
			}
			else cairoout.copy(cairotemp);
			cairotemp.unmap_cairo_image();
			cairo_surface_destroy(temp);			
		}
			break;
			
		case Blur::CROSS: // C R O S S  -------------------------------------------------------
		{
			//horizontal part
			cairo_surface_t* temp=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, w, h);
			CairoSurface cairotemp(temp);
			if(!cairotemp.map_cairo_image())
			{
				synfig::info("cairotemp map cairo image failed. Cross");
				cairo_surface_destroy(temp);
				return false;
			}
			
			if(size[0])
			{
				int length = halfsizex;
				length = std::max(1,length);
				
				etl::hbox_blur(cairosurface.begin(),cairosurface.end(),length,cairotemp.begin());
			}
			else cairotemp.copy(cairosurface);
			
			//vertical part
			cairo_surface_t* temp2=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, w, h);
			CairoSurface cairotemp2(temp2);
			if(!cairotemp2.map_cairo_image())
			{
				synfig::info("cairotemp2 map cairo image failed. Cross");
				cairotemp.unmap_cairo_image();
				cairo_surface_destroy(temp);
				cairo_surface_destroy(temp2);
				return false;
			}
			
			if(size[1])
			{
				int length = halfsizey;
				length = std::max(1,length);
				
				etl::vbox_blur(cairosurface.begin(),cairosurface.end(),length,cairotemp2.begin());
			}
			else cairotemp2.copy(cairosurface);
			
			//blend the two together
			int x,y;
			
			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					cairoout[y][x] = cairotemp[y][x]*0.5+cairotemp2[y][x]*0.5;
				}
			}
			cairotemp.unmap_cairo_image();
			cairo_surface_destroy(temp);
			cairotemp2.unmap_cairo_image();
			cairo_surface_destroy(temp2);
			break;
		}
			
		case Blur::GAUSSIAN:	// G A U S S I A N ----------------------------------------------
		{
#ifndef	GAUSSIAN_ADJUSTMENT
#define GAUSSIAN_ADJUSTMENT		(0.05)
#endif
			
			Real	pw = (Real)w/(resolution[0]);
			Real 	ph = (Real)h/(resolution[1]);
			
			CairoSurface *gauss_surface;
			cairoout.copy(cairosurface);
			gauss_surface = &cairoout;
            /* Squaring the pw and ph values
			 is necessary to insure consistent
			 results when rendered to different
			 resolutions.
			 Unfortunately, this automatically
			 squares our rendertime.
			 There has got to be a faster way...
			 */
			pw=pw*pw;
			ph=ph*ph;
			
			int bw = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			int bh = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
			int max=bw+bh;
			
			CairoColorAccumulator *SC0=new class CairoColorAccumulator[w+2];
			CairoColorAccumulator *SC1=new class CairoColorAccumulator[w+2];
			CairoColorAccumulator *SC2=new class CairoColorAccumulator[w+2];
			CairoColorAccumulator *SC3=new class CairoColorAccumulator[w+2];
						
			while(bw&&bh)
			{
				if(!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}
				
				if(bw>=4 && bh>=4)
				{
					etl::gaussian_blur_5x5_(gauss_surface->begin(),gauss_surface->get_w(),gauss_surface->get_h(),SC0,SC1,SC2,SC3);
					bw-=4,bh-=4;
				}
				else
					if(bw>=2 && bh>=2)
					{
						etl::gaussian_blur_3x3(gauss_surface->begin(),gauss_surface->end());
						bw-=2,bh-=2;
					}
					else
						if(bw>=1 && bh>=1)
						{
							GaussianBlur_2x2(*gauss_surface);
							bw--,bh--;
						}				
			}
			while(bw)
			{
				if(!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}
				if(bw>=2)
				{
					GaussianBlur_3x1(*gauss_surface);
					bw-=2;
				}
				else
					if(bw>=1)
					{
						GaussianBlur_2x1(*gauss_surface);
						bw--;
					}
			}
			while(bh)
			{
				if(!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}
				if(bh>=2)
				{
					GaussianBlur_1x3(*gauss_surface);
					bh-=2;
				}
				else
					if(bh>=1)
					{
						GaussianBlur_1x2(*gauss_surface);
						bh--;
					}
			}
			
			delete [] SC0;
			delete [] SC1;
			delete [] SC2;
			delete [] SC3;
		}
			break;
			
		default:
			break;
	}

	//we are FRIGGGIN done....
	blurcall.amount_complete(100,100);
	
	cairosurface.unmap_cairo_image();
	cairoout.unmap_cairo_image();
	
	return true;
}

//////

bool Blur::operator()(const etl::surface<float> &surface,
					  const synfig::Vector &resolution,
					  etl::surface<float> &out) const
{
	int w = surface.get_w(),
		h = surface.get_h();

	if(w == 0 || h == 0 || resolution[0] == 0 || resolution[1] == 0) return false;

	const Real	pw = resolution[0]/w,
				ph = resolution[1]/h;

	int	halfsizex = (int) (abs(size[0]*.5/pw) + 1),
		halfsizey = (int) (abs(size[1]*.5/ph) + 1);
	int x,y;

	SuperCallback blurcall(cb,0,5000,5000);

	etl::surface<float> worksurface(surface);

	//don't need to premultiply because we are dealing with ONLY alpha

	switch(type)
	{
	case Blur::DISC:	// D I S C ----------------------------------------------------------
		{
			int bw = halfsizex;
			int bh = halfsizey;

			if(size[0] && size[1] && w*h>2)
			{
				int x2,y2;
				etl::surface<float> tmp_surface(worksurface);

				for(y=0;y<h;y++)
				{
					for(x=0;x<w;x++)
					{
						//accumulate all the pixels in an ellipse of w,h about the current pixel
						float a = 0;
						int total=0;

						for(y2=-bh;y2<=bh;y2++)
						{
							for(x2=-bw;x2<=bw;x2++)
							{
								//get the floating point distance away from the origin pixel in relative coords
								float tmp_x=(float)x2/bw;
								float tmp_y=(float)y2/bh;
								tmp_x*=tmp_x;
								tmp_y*=tmp_y;

								//ignore if it's outside of the disc
								if( tmp_x+tmp_y>1.0)
									continue;

								//cap the pixel indices to inside the surface
								int u= x+x2,
									v= y+y2;

								if( u < 0 )					u = 0;
								if( u >= w ) u = w-1;

								if( v < 0 ) 				v = 0;
								if( v >= h ) v = h-1;

								//accumulate the color, and # of pixels added in
								a += tmp_surface[v][u];
								total++;
							}
						}

						//blend the color with the original color
						//if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
							worksurface[y][x]=a/total;
						//else
						//	worksurface[y][x]=Color::blend(color/total,tmp_surface[y][x],get_amount(),get_blend_method());
					}
					if(!blurcall.amount_complete(y,h))
					{
						//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
						return false;
					}
				}
				break;
			}

			//if we don't qualify for disc blur just use box blur
		}

	case Blur::BOX: // B O X -------------------------------------------------------
		{
			//horizontal part
			etl::surface<float> temp_surface;
			temp_surface.set_wh(w,h);

			if(size[0])
			{
				int length = halfsizex;
				length=std::max(1,length);

				etl::hbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface.begin());
			}
			else temp_surface = worksurface;

			//vertical part
			//etl::surface<float> temp_surface2;
			//temp_surface2.set_wh(w,h);

			if(size[1])
			{
				int length = halfsizey;
				length = std::max(1,length);
				etl::vbox_blur(temp_surface.begin(),temp_surface.end(),length,worksurface.begin());
			}
			else worksurface = temp_surface;

			//blend with the original surface
			/*int x,y;
			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					worksurface[y][x]=temp_surface2[y][x];//Color::blend(temp_surface2[y][x],worksurface[y][x],get_amount(),get_blend_method());
				}
			}*/
		}
		break;

	case Blur::FASTGAUSSIAN:	// F A S T G A U S S I A N ----------------------------------------------
		{
			//fast gaussian is treated as a 3x3 type of thing, except expanded to work with the length

			/*	1	2	1
				2	4	2
				1	2	1
			*/

			etl::surface<float> temp_surface;
			temp_surface.set_wh(w,h);

			//etl::surface<float> temp_surface2;
			//temp_surface2.set_wh(w,h);

			//horizontal part
			if(size[0])
			{
				Real length=abs((float)w/(resolution[0]))*size[0]*0.5+1;
				length=std::max(1.0,length);

				//two box blurs produces: 1 2 1
				etl::hbox_blur(worksurface.begin(),w,h,(int)(length*3/4),temp_surface.begin());
				etl::hbox_blur(temp_surface.begin(),w,h,(int)(length*3/4),worksurface.begin());
			}
			//else temp_surface2=worksurface;

			//vertical part
			if(size[1])
			{
				Real length=abs((float)h/(resolution[1]))*size[1]*0.5+1;
				length=std::max(1.0,length);

				//two box blurs produces: 1 2 1 on the horizontal 1 2 1
				etl::vbox_blur(worksurface.begin(),w,h,(int)(length*3/4),temp_surface.begin());
				etl::vbox_blur(temp_surface.begin(),w,h,(int)(length*3/4),worksurface.begin());
			}
			//else temp_surface2=temp_surface2;

			/*int x,y;
			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					worksurface[y][x]=temp_surface2[y][x];//Color::blend(temp_surface2[y][x],worksurface[y][x],get_amount(),get_blend_method());
				}
			}*/
		}
		break;

	case Blur::CROSS: // C R O S S  -------------------------------------------------------
		{
			//horizontal part
			etl::surface<float> temp_surface;
			temp_surface.set_wh(worksurface.get_w(),worksurface.get_h());

			if(size[0])
			{
				int length = halfsizex;
				length = std::max(1,length);

				etl::hbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface.begin());
			}
			else temp_surface = worksurface;

			//vertical part
			etl::surface<float> temp_surface2;
			temp_surface2.set_wh(worksurface.get_w(),worksurface.get_h());

			if(size[1])
			{
				int length = halfsizey;
				length = std::max(1,length);

				etl::vbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface2.begin());
			}
			else temp_surface2 = worksurface;

			//blend the two together
			int x,y;

			for(y=0;y<h;y++)
			{
				for(x=0;x<w;x++)
				{
					worksurface[y][x] = (temp_surface[y][x]+temp_surface2[y][x])/2;//Color::blend((temp_surface[y][x]+temp_surface2[y][x])/2,worksurface[y][x],get_amount(),get_blend_method());
				}
			}

			break;
		}

	case Blur::GAUSSIAN:	// G A U S S I A N ----------------------------------------------
		{
			#ifndef	GAUSSIAN_ADJUSTMENT
			#define GAUSSIAN_ADJUSTMENT		(0.05)
			#endif

			Real	pw = (Real)w/(resolution[0]);
			Real 	ph = (Real)h/(resolution[1]);

			//etl::surface<float> temp_surface;
			etl::surface<float> *gauss_surface;

			//if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
				gauss_surface = &worksurface;
			/*else
			{
				temp_surface = worksurface;
				gauss_surface = &temp_surface;
			}*/

            /* Squaring the pw and ph values
			   is necessary to insure consistent
			   results when rendered to different
			   resolutions.
			   Unfortunately, this automatically
			   squares our rendertime.
			   There has got to be a faster way...
			*/
			pw=pw*pw;
			ph=ph*ph;

			int bw = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			int bh = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
			int max=bw+bh;

			float *SC0=new float[w+2];
			float *SC1=new float[w+2];
			float *SC2=new float[w+2];
			float *SC3=new float[w+2];

			memset(SC0,0,(w+2)*sizeof(float));
			memset(SC0,0,(w+2)*sizeof(float));
			memset(SC0,0,(w+2)*sizeof(float));
			memset(SC0,0,(w+2)*sizeof(float));

			//int i = 0;

			while(bw&&bh)
			{
				if (!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}

				if(bw>=4 && bh>=4)
				{
					etl::gaussian_blur_5x5_(gauss_surface->begin(),gauss_surface->get_w(),gauss_surface->get_h(),SC0,SC1,SC2,SC3);
					bw-=4,bh-=4;
				}
				else
				if(bw>=2 && bh>=2)
				{
					etl::gaussian_blur_3x3(gauss_surface->begin(),gauss_surface->end());
					bw-=2,bh-=2;
				}
				else
				if(bw>=1 && bh>=1)
				{
					GaussianBlur_2x2(*gauss_surface);
					bw--,bh--;
				}
			}

			while(bw)
			{
				if (!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}
				if(bw>=2)
				{
					GaussianBlur_3x1(*gauss_surface);
					bw-=2;
				}
				else
				if(bw>=1)
				{
					GaussianBlur_2x1(*gauss_surface);
					bw--;
				}
			}

			while(bh)
			{
				if (!blurcall.amount_complete(max-(bw+bh),max)) {
					delete [] SC0;
					delete [] SC1;
					delete [] SC2;
					delete [] SC3;

					return false;
				}
				if(bh>=2)
				{
					GaussianBlur_1x3(*gauss_surface);
					bh-=2;
				}
				else
				if(bh>=1)
				{
					GaussianBlur_1x2(*gauss_surface);
					bh--;
				}
			}

			delete [] SC0;
			delete [] SC1;
			delete [] SC2;
			delete [] SC3;

			/*if(get_amount()!=1.0 || get_blend_method()!=Color::BLEND_STRAIGHT)
			{
				int x,y;
				for(y=0;y<renddesc.get_h();y++)
					for(x=0;x<renddesc.get_w();x++)
						worksurface[y][x]=Color::blend(temp_surface[y][x],worksurface[y][x],get_amount(),get_blend_method());
			}*/
		}
		break;

		default:
			break;
	}

	//be sure the surface is of the correct size
	//surface->set_wh(renddesc.get_w(),renddesc.get_h());
	out.set_wh(w,h);

	//divide out the alpha - don't need to cause we rock
	out = worksurface;

	//we are FRIGGGIN done....
	blurcall.amount_complete(100,100);

	return true;
}

/* === E N T R Y P O I N T ================================================= */
