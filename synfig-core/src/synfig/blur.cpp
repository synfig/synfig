/* === S Y N F I G ========================================================= */
/*!	\file synfig/blur.cpp
**	\brief Blur Implementation File
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
			Angle theta=Angle::rot((float)rand()/(float)RAND_MAX);
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

template <typename T,class VP>
static void GaussianBlur_2x2(synfig::surface<T,VP>& surface)
{
	int x,y,w,h;
	T Tmp1,Tmp2,SR0;

	w=surface.get_w();
	h=surface.get_h();
	
	T *SC0=new T[w];

	memcpy(static_cast<void*>(SC0), surface[0], w*sizeof(T));

	for(y=0;y<h;y++)
	{
		SR0=surface[y][0];
		for(x=0;x<w;x++)
		{
			Tmp1=surface[y][x];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			surface[y][x]=(SC0[x]+Tmp2)/4;
			SC0[x]=Tmp2;
		}
	}
	delete [] SC0;
}

template <typename T,class VP>
static void GaussianBlur_2x1(synfig::surface<T,VP> &surface)
{
	int x,y,w,h;
	T Tmp1,Tmp2,SR0;

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

template <typename T,class VP>
static void GaussianBlur_3x1(synfig::surface<T,VP> &surface)
{
	int x,y,w,h;
	T Tmp1,Tmp2,SR0,SR1;
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

template <typename T,class VP>
static void GaussianBlur_1x2(synfig::surface<T,VP> &surface)
{
	int x,y;
	T Tmp1,Tmp2,SR0;

	for(x=0;x<surface.get_w();x++)
	{
		SR0 = zero<T>();
		for(y=0;y<surface.get_h();y++)
		{
			Tmp1=surface[y][x];
			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			surface[y][x]=(Tmp2)/2;
		}
	}
}

template <typename T,class VP>
static void GaussianBlur_1x3(synfig::surface<T,VP> &surface)
{
	int x,y;
	T Tmp1,Tmp2,SR0,SR1;

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

	int	halfsizex = (int) (std::fabs(size[0]*.5/pw) + 1),
		halfsizey = (int) (std::fabs(size[1]*.5/ph) + 1);

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
				Real length=std::fabs((float)w/(resolution[0]))*size[0]*0.5+1;
				length=std::max(1.0,length);

				//two box blurs produces: 1 2 1
				etl::hbox_blur(worksurface.begin(),w,h,(int)(length*3/4),temp_surface.begin());
				etl::hbox_blur(temp_surface.begin(),w,h,(int)(length*3/4),worksurface.begin());
			}
			//else temp_surface2=worksurface;

			//vertical part
			if(size[1])
			{
				Real length=std::fabs((float)h/(resolution[1]))*size[1]*0.5+1;
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

			int bw = (int)(std::fabs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			int bh = (int)(std::fabs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
			int max=bw+bh;

			Color* SC0=new Color[w+2];
			Color* SC1=new Color[w+2];
			Color* SC2=new Color[w+2];
			Color* SC3=new Color[w+2];

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

bool Blur::operator()(const synfig::surface<float> &surface,
					  const synfig::Vector &resolution,
					  synfig::surface<float> &out) const
{
	int w = surface.get_w(),
		h = surface.get_h();

	if(w == 0 || h == 0 || resolution[0] == 0 || resolution[1] == 0) return false;

	const Real	pw = resolution[0]/w,
				ph = resolution[1]/h;

	int	halfsizex = (int) (std::fabs(size[0]*.5/pw) + 1),
		halfsizey = (int) (std::fabs(size[1]*.5/ph) + 1);
	int x,y;

	SuperCallback blurcall(cb,0,5000,5000);

	synfig::surface<float> worksurface(surface);

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
				synfig::surface<float> tmp_surface(worksurface);

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
			synfig::surface<float> temp_surface;
			temp_surface.set_wh(w,h);

			if(size[0])
			{
				int length = halfsizex;
				length=std::max(1,length);

				etl::hbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface.begin());
			}
			else temp_surface = worksurface;

			//vertical part
			//synfig::surface<float> temp_surface2;
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

			synfig::surface<float> temp_surface;
			temp_surface.set_wh(w,h);

			//synfig::surface<float> temp_surface2;
			//temp_surface2.set_wh(w,h);

			//horizontal part
			if(size[0])
			{
				Real length=std::fabs((float)w/(resolution[0]))*size[0]*0.5+1;
				length=std::max(1.0,length);

				//two box blurs produces: 1 2 1
				etl::hbox_blur(worksurface.begin(),w,h,(int)(length*3/4),temp_surface.begin());
				etl::hbox_blur(temp_surface.begin(),w,h,(int)(length*3/4),worksurface.begin());
			}
			//else temp_surface2=worksurface;

			//vertical part
			if(size[1])
			{
				Real length=std::fabs((float)h/(resolution[1]))*size[1]*0.5+1;
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
			synfig::surface<float> temp_surface;
			temp_surface.set_wh(worksurface.get_w(),worksurface.get_h());

			if(size[0])
			{
				int length = halfsizex;
				length = std::max(1,length);

				etl::hbox_blur(worksurface.begin(),worksurface.end(),length,temp_surface.begin());
			}
			else temp_surface = worksurface;

			//vertical part
			synfig::surface<float> temp_surface2;
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

			//synfig::surface<float> temp_surface;
			synfig::surface<float> *gauss_surface;

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

			int bw = (int)(std::fabs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			int bh = (int)(std::fabs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
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
