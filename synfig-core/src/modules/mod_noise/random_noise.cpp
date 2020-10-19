/* === S Y N F I G ========================================================= */
/*!	\file mod_noise/random_noise.cpp
**	\brief blehh
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "random_noise.h"
#include <synfig/quick_rng.h>
#include <cmath>
#endif

/* === M A C R O S ========================================================= */
#ifndef PI
#define PI	(3.1415927)
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
RandomNoise::set_seed(int x)
{
	seed_=x;
}

float
RandomNoise::operator()(const int salt,const int x,const int y,const int t)const
{
	static const unsigned int a(21870);
	static const unsigned int b(11213);
	static const unsigned int c(36979);
	static const unsigned int d(31337);

	quick_rng rng(
		( static_cast<unsigned int>(x+y)        * a ) ^
		( static_cast<unsigned int>(y+t)        * b ) ^
		( static_cast<unsigned int>(t+x)        * c ) ^
		( static_cast<unsigned int>(seed_+salt) * d )
	);

	return rng.f() * 2.0f - 1.0f;
}

float
RandomNoise::operator()(SmoothType smooth,int subseed,float xf,float yf,float tf,int loop)const
{
	int x((int)floor(xf));
	int y((int)floor(yf));
	int t((int)floor(tf));
	int t_1, t0, t1, t2;

	if (loop)
	{
		t0  = t % loop;	if (t0  <  0   ) t0  += loop;
		t_1 = t0 - 1;	if (t_1 <  0   ) t_1 += loop;
		t1  = t0 + 1;	if (t1  >= loop) t1  -= loop;
		t2  = t1 + 1;	if (t2  >= loop) t2  -= loop;
	}
	else
	{
		t0  = t;
		t_1 = t - 1;
		t1  = t + 1;
		t2  = t + 2;
	}

	// synfig::info("%s:%d tf %.2f loop %d fraction %.2f ( -1,0,1,2 : %2d %2d %2d %2d)", __FILE__, __LINE__, tf, loop, tf-t, t_1, t0, t1, t2);

	switch(smooth)
	{
	case SMOOTH_CUBIC:	// cubic
		{
			#define f(j,i,k)	((*this)(subseed,i,j,k))
			//Using catmull rom interpolation because it doesn't blur at all
			// ( http://www.gamedev.net/reference/articles/article1497.asp )
			//bezier curve with intermediate ctrl pts: 0.5/3(p(i+1) - p(i-1)) and similar
			float xfa [4], tfa[4];

			//precalculate indices (all clamped) and offset
			const int xa[] = {x-1,x,x+1,x+2};
			const int ya[] = {y-1,y,y+1,y+2};
			const int ta[] = {t_1,t0,t1,t2};

			const float dx(xf-x);
			const float dy(yf-y);
			const float dt(tf-t);

			//figure polynomials for each point
			const float txf[] =
			{
				0.5f*dx*(dx*(dx*(-1.f) + 2.f) - 1.f),	//-t + 2t^2 -t^3
				0.5f*(dx*(dx*(3.f*dx - 5.f)) + 2.f), 	//2 - 5t^2 + 3t^3
				0.5f*dx*(dx*(-3.f*dx + 4.f) + 1.f),		//t + 4t^2 - 3t^3
				0.5f*dx*dx*(dx-1.f)						//-t^2 + t^3
			};

			const float tyf[] =
			{
				0.5f*dy*(dy*(dy*(-1.f) + 2.f) - 1.f),	//-t + 2t^2 -t^3
				0.5f*(dy*(dy*(3.f*dy - 5.f)) + 2.f), 	//2 - 5t^2 + 3t^3
				0.5f*dy*(dy*(-3.f*dy + 4.f) + 1.f),		//t + 4t^2 - 3t^3
				0.5f*dy*dy*(dy-1.f)						//-t^2 + t^3
			};

			const float ttf[] =
			{
				0.5f*dt*(dt*(dt*(-1.f) + 2.f) - 1.f),	//-t + 2t^2 -t^3
				0.5f*(dt*(dt*(3.f*dt - 5.f)) + 2.f), 	//2 - 5t^2 + 3t^3
				0.5f*dt*(dt*(-3.f*dt + 4.f) + 1.f),		//t + 4t^2 - 3t^3
				0.5f*dt*dt*(dt-1.f)						//-t^2 + t^3
			};

			//evaluate polynomial for each row
			for(int i = 0; i < 4; ++i)
			{
				for(int j = 0; j < 4; ++j)
				{
					tfa[j] = f(ya[i],xa[j],ta[0])*ttf[0] + f(ya[i],xa[j],ta[1])*ttf[1] + f(ya[i],xa[j],ta[2])*ttf[2] + f(ya[i],xa[j],ta[3])*ttf[3];
				}
				xfa[i] = tfa[0]*txf[0] + tfa[1]*txf[1] + tfa[2]*txf[2] + tfa[3]*txf[3];
			}

			//return the cumulative column evaluation
			return xfa[0]*tyf[0] + xfa[1]*tyf[1] + xfa[2]*tyf[2] + xfa[3]*tyf[3];
#undef f
		}
		break;


	case SMOOTH_FAST_SPLINE:	// Fast Spline (non-animated)
		{
#define P(x)		(((x)>0)?((x)*(x)*(x)):0.0f)
#define R(x)		( P(x+2) - 4.0f*P(x+1) + 6.0f*P(x) - 4.0f*P(x-1) )*(1.0f/6.0f)
#define F(i,j)		((*this)(subseed,i+x,j+y)*(R((i)-a)*R(b-(j))))
#define FT(i,j,k,l)	((*this)(subseed,i+x,j+y,l)*(R((i)-a)*R(b-(j))*R((k)-c)))
#define Z(i,j)		ret+=F(i,j)
#define ZT(i,j,k,l) ret+=FT(i,j,k,l)
#define X(i,j)		// placeholder... To make box more symmetric
#define XT(i,j,k,l)	// placeholder... To make box more symmetric

		float a(xf-x), b(yf-y);

		// Interpolate
		float ret(F(0,0));
		Z(-1,-1); Z(-1, 0); Z(-1, 1); Z(-1, 2);
		Z( 0,-1); X( 0, 0); Z( 0, 1); Z( 0, 2);
		Z( 1,-1); Z( 1, 0); Z( 1, 1); Z( 1, 2);
		Z( 2,-1); Z( 2, 0); Z( 2, 1); Z( 2, 2);

		return ret;
	}

	case SMOOTH_SPLINE:	// Spline (animated)
		{
			float a(xf-x), b(yf-y), c(tf-t);

			// Interpolate
			float ret(FT(0,0,0,t0));
			ZT(-1,-1,-1,t_1); ZT(-1, 0,-1,t_1); ZT(-1, 1,-1,t_1); ZT(-1, 2,-1,t_1);
			ZT( 0,-1,-1,t_1); ZT( 0, 0,-1,t_1); ZT( 0, 1,-1,t_1); ZT( 0, 2,-1,t_1);
			ZT( 1,-1,-1,t_1); ZT( 1, 0,-1,t_1); ZT( 1, 1,-1,t_1); ZT( 1, 2,-1,t_1);
			ZT( 2,-1,-1,t_1); ZT( 2, 0,-1,t_1); ZT( 2, 1,-1,t_1); ZT( 2, 2,-1,t_1);

			ZT(-1,-1, 0,t0 ); ZT(-1, 0, 0,t0 ); ZT(-1, 1, 0,t0 ); ZT(-1, 2, 0,t0 );
			ZT( 0,-1, 0,t0 ); XT( 0, 0, 0,t0 ); ZT( 0, 1, 0,t0 ); ZT( 0, 2, 0,t0 );
			ZT( 1,-1, 0,t0 ); ZT( 1, 0, 0,t0 ); ZT( 1, 1, 0,t0 ); ZT( 1, 2, 0,t0 );
			ZT( 2,-1, 0,t0 ); ZT( 2, 0, 0,t0 ); ZT( 2, 1, 0,t0 ); ZT( 2, 2, 0,t0 );

			ZT(-1,-1, 1,t1 ); ZT(-1, 0, 1,t1 ); ZT(-1, 1, 1,t1 ); ZT(-1, 2, 1,t1 );
			ZT( 0,-1, 1,t1 ); ZT( 0, 0, 1,t1 ); ZT( 0, 1, 1,t1 ); ZT( 0, 2, 1,t1 );
			ZT( 1,-1, 1,t1 ); ZT( 1, 0, 1,t1 ); ZT( 1, 1, 1,t1 ); ZT( 1, 2, 1,t1 );
			ZT( 2,-1, 1,t1 ); ZT( 2, 0, 1,t1 ); ZT( 2, 1, 1,t1 ); ZT( 2, 2, 1,t1 );

			ZT(-1,-1, 2,t2 ); ZT(-1, 0, 2,t2 ); ZT(-1, 1, 2,t2 ); ZT(-1, 2, 2,t2 );
			ZT( 0,-1, 2,t2 ); ZT( 0, 0, 2,t2 ); ZT( 0, 1, 2,t2 ); ZT( 0, 2, 2,t2 );
			ZT( 1,-1, 2,t2 ); ZT( 1, 0, 2,t2 ); ZT( 1, 1, 2,t2 ); ZT( 1, 2, 2,t2 );
			ZT( 2,-1, 2,t2 ); ZT( 2, 0, 2,t2 ); ZT( 2, 1, 2,t2 ); ZT( 2, 2, 2,t2 );

			return ret;

/*

			float dx=xf-x;
			float dy=yf-y;
			float dt=tf-t;

			float ret=0;
			int i,j,h;
			for(h=-1;h<=2;h++)
				for(i=-1;i<=2;i++)
					for(j=-1;j<=2;j++)
						ret+=(*this)(subseed,i+x,j+y,h+t)*(R(i-dx)*R(j-dy)*R(h-dt));
			return ret;
*/
		}
		break;
#undef X
#undef Z
#undef F
#undef P
#undef R

	case SMOOTH_COSINE:
	if((float)t==tf)
	{
		int x((int)floor(xf));
		int y((int)floor(yf));
		float a=xf-x;
		float b=yf-y;
		a=(1.0f-cos(a*PI))*0.5f;
		b=(1.0f-cos(b*PI))*0.5f;
		float c=1.0-a;
		float d=1.0-b;
		int x2=x+1,y2=y+1;
		return
			(*this)(subseed,x,y,t0)*(c*d)+
			(*this)(subseed,x2,y,t0)*(a*d)+
			(*this)(subseed,x,y2,t0)*(c*b)+
			(*this)(subseed,x2,y2,t0)*(a*b);
	}
	else
	{
		float a=xf-x;
		float b=yf-y;
		float c=tf-t;

		a=(1.0f-cos(a*PI))*0.5f;
		b=(1.0f-cos(b*PI))*0.5f;

		// We don't perform this on the time axis, otherwise we won't
		// get smooth motion
		//c=(1.0f-cos(c*PI))*0.5f;

		float d=1.0-a;
		float e=1.0-b;
		float f=1.0-c;

		int x2=x+1,y2=y+1;

		return
			(*this)(subseed,x,y,t0)*(d*e*f)+
			(*this)(subseed,x2,y,t0)*(a*e*f)+
			(*this)(subseed,x,y2,t0)*(d*b*f)+
			(*this)(subseed,x2,y2,t0)*(a*b*f)+
			(*this)(subseed,x,y,t1)*(d*e*c)+
			(*this)(subseed,x2,y,t1)*(a*e*c)+
			(*this)(subseed,x,y2,t1)*(d*b*c)+
			(*this)(subseed,x2,y2,t1)*(a*b*c);
	}
	case SMOOTH_LINEAR:
	if((float)t==tf)
	{
		int x((int)floor(xf));
		int y((int)floor(yf));
		float a=xf-x;
		float b=yf-y;
		float c=1.0-a;
		float d=1.0-b;
		int x2=x+1,y2=y+1;
		return
			(*this)(subseed,x,y,t0)*(c*d)+
			(*this)(subseed,x2,y,t0)*(a*d)+
			(*this)(subseed,x,y2,t0)*(c*b)+
			(*this)(subseed,x2,y2,t0)*(a*b);
	}
	else
	{

		float a=xf-x;
		float b=yf-y;
		float c=tf-t;

		float d=1.0-a;
		float e=1.0-b;
		float f=1.0-c;

		int x2=x+1,y2=y+1;

		return
			(*this)(subseed,x,y,t0)*(d*e*f)+
			(*this)(subseed,x2,y,t0)*(a*e*f)+
			(*this)(subseed,x,y2,t0)*(d*b*f)+
			(*this)(subseed,x2,y2,t0)*(a*b*f)+
			(*this)(subseed,x,y,t1)*(d*e*c)+
			(*this)(subseed,x2,y,t1)*(a*e*c)+
			(*this)(subseed,x,y2,t1)*(d*b*c)+
			(*this)(subseed,x2,y2,t1)*(a*b*c);
	}
	default:
	case SMOOTH_DEFAULT:
		return (*this)(subseed,x,y,t0);
	}
}
