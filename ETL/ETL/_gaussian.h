/*! ========================================================================
** Extended Template Library
** Gaussian Blur Template Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__GAUSSIAN_H
#define __ETL__GAUSSIAN_H

/* === H E A D E R S ======================================================= */

#include <cstring>		// for memset()
#include <iterator>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template<typename T> void
gaussian_blur_5x5_(T pen,int w, int h,
typename T::accumulator_pointer SC0,
typename T::accumulator_pointer SC1,
typename T::accumulator_pointer SC2,
typename T::accumulator_pointer SC3)
{
	int x,y;
	typename T::accumulator_type Tmp1,Tmp2,SR0,SR1,SR2,SR3;

	//typename T::iterator_x iter;

	// Setup the row buffers
	for(x=0;x<w;x++)SC0[x+2]=(typename T::accumulator_type)(pen.x()[x])*24;
	memset((void *)SC1,0,(w+2)*sizeof(typename T::accumulator_type));
	memset((void *)SC2,0,(w+2)*sizeof(typename T::accumulator_type));
	memset((void *)SC3,0,(w+2)*sizeof(typename T::accumulator_type));
	/*memset(SC1,0,(w+2)*sizeof(typename T::accumulator_type));
	memset(SC2,0,(w+2)*sizeof(typename T::accumulator_type));
	memset(SC3,0,(w+2)*sizeof(typename T::accumulator_type));*/

	for(y=0;y<h+2;y++,pen.inc_y())
	{
		int yadj;
		if(y>=h)
			{yadj=(h-y)-1; SR0=(typename T::accumulator_type)(pen.y()[yadj])*1.35;}
		else
			{yadj=0; SR0=(typename T::accumulator_type)(pen.get_value())*1.35; }

		SR1=SR2=SR3=typename T::accumulator_type();
		for(x=0;x<w+2;x++,pen.inc_x())
		{
			if(x>=w)
				Tmp1=(typename T::accumulator_type)(pen[yadj][(w-x)-1]);
			else
				Tmp1=(typename T::accumulator_type)(*pen[yadj]);

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
				pen[-2][-2]=(typename T::value_type)((SC3[x]+Tmp2)/256);
			SC3[x]=Tmp2;
		}
		pen.dec_x(x);
	}

}

template<typename T> void
gaussian_blur_5x5(T pen, int w, int h)
{
	typename T::accumulator_pointer SC0=new typename T::accumulator_type[w+2];
	typename T::accumulator_pointer SC1=new typename T::accumulator_type[w+2];
	typename T::accumulator_pointer SC2=new typename T::accumulator_type[w+2];
	typename T::accumulator_pointer SC3=new typename T::accumulator_type[w+2];

	gaussian_blur_5x5_(pen,w,h,SC0,SC1,SC2,SC3);

	delete [] SC0;
	delete [] SC1;
	delete [] SC2;
	delete [] SC3;
}

template<typename T> void
gaussian_blur_5x5(T begin, T end)
{
	typename T::difference_type size(end-begin);

	typename T::accumulator_pointer SC0=new typename T::accumulator_type[size.x+2];
	typename T::accumulator_pointer SC1=new typename T::accumulator_type[size.x+2];
	typename T::accumulator_pointer SC2=new typename T::accumulator_type[size.x+2];
	typename T::accumulator_pointer SC3=new typename T::accumulator_type[size.x+2];

	gaussian_blur_5x5_(begin,size.x,size.y,SC0,SC1,SC2,SC3);

	delete [] SC0;
	delete [] SC1;
	delete [] SC2;
	delete [] SC3;
}

template<typename T> void
gaussian_blur_3x3(T pen,int w, int h)
{
	int x,y;
	typename T::accumulator_type Tmp1,Tmp2,SR0,SR1;

//	typename T::iterator_x iter;

	typename T::accumulator_pointer SC0=new typename T::accumulator_type[w+1];
	typename T::accumulator_pointer SC1=new typename T::accumulator_type[w+1];

	// Setup the row buffers
	for(x=0;x<w;x++)SC0[x+1]=(typename T::accumulator_type)(pen.x()[x])*4;
	memset((void *)SC1,0,(w+1)*sizeof(typename T::accumulator_type));

	for(y=0;y<h+1;y++,pen.inc_y())
	{
		int yadj;
		if(y>=h)
			{yadj=-1; SR1=SR0=(typename T::accumulator_type)(pen.y()[yadj]);}
		else
			{yadj=0; SR1=SR0=(typename T::accumulator_type)(pen.get_value()); }

		for(x=0;x<w+1;x++,pen.inc_x())
		{
			if(x>=w)
				Tmp1=(typename T::accumulator_type)(pen[yadj][(w-x)-2]);
			else
				Tmp1=(typename T::accumulator_type)(*pen[yadj]);

			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			Tmp1=SR1+Tmp2;
			SR1=Tmp2;

			Tmp2=SC0[x]+Tmp1;
			SC0[x]=Tmp1;
			if(y&&x)
				pen[-1][-1]=(typename T::value_type)((SC1[x]+Tmp2)/16);
			SC1[x]=Tmp2;
		}
		pen.dec_x(x);
	}

	delete [] SC0;
	delete [] SC1;
}

//! 2D 3x3 pixel gaussian blur
template<typename _PEN> void
gaussian_blur_3x3(_PEN begin, _PEN end)
{
	typename _PEN::difference_type size(end-begin);
	gaussian_blur_3x3(begin,size.x,size.y);
}

//! 1D 3 pixel gaussian blur
template<typename I> void
gaussian_blur_3(I begin, I end, bool endpts = true)
{
//	typedef typename I _itertype;
//	int i;
	typename std::iterator_traits<I>::value_type Tmp1,Tmp2,SR0,SR1;

	SR0=SR1=*begin;
	I iter,prev=begin;
	for(iter=begin;iter!=end;prev=iter++)
	{
		Tmp1=*iter;
		Tmp2=SR0+Tmp1;
		SR0=Tmp1;
		Tmp1=SR1+Tmp2;
		SR1=Tmp2;
		if(iter!=begin && ( endpts || (prev != begin) ))
			*prev=(Tmp1)/4;
	}

	if(endpts)
	{
		Tmp1=*prev;
		Tmp2=SR0+Tmp1;
		SR0=Tmp1;
		Tmp1=SR1+Tmp2;
		SR1=Tmp2;
		*prev=(Tmp1)/4;
	}
}

//! 2D 3x1 pixel gaussian blur
template<typename _PEN> void
gaussian_blur_3x1(_PEN begin, _PEN end)
{
	typename _PEN::difference_type size=end-begin;
	for(;size.y>0;size.y--, begin.inc_y())
		gaussian_blur_3(begin.x(),begin.x()+size.x);
}

//! 2D 1x3 pixel gaussian blur
template<typename _PEN> void
gaussian_blur_1x3(_PEN begin, _PEN end)
{
	typename _PEN::difference_type size=end-begin;
	for(;size.x>0;size.x--,begin.inc_x())
		gaussian_blur_3(begin.y(),begin.y()+size.y);
}

template<typename T> void
gaussian_blur(T pen, int w, int h, int blur_x, int blur_y)
{
	typename T::accumulator_pointer SC0=new typename T::accumulator_type[w+2];
	typename T::accumulator_pointer SC1=new typename T::accumulator_type[w+2];
	typename T::accumulator_pointer SC2=new typename T::accumulator_type[w+2];
	typename T::accumulator_pointer SC3=new typename T::accumulator_type[w+2];

	blur_x--;
	blur_y--;

	while(blur_x&&blur_y)
	{
		if(blur_x>=4 && blur_y>=4)
		{
			gaussian_blur_5x5_(pen,w,h,SC0,SC1,SC2,SC3);
			blur_x-=4,blur_y-=4;
		}
		else if(blur_x>=2 && blur_y>=2)
		{
			gaussian_blur_3x3(pen,w,h);
			blur_x-=2,blur_y-=2;
		}
		else
			blur_x--,blur_y--;
	}
	while(blur_x)
	{
		if(blur_x>=2)
		{
			gaussian_blur_3x1(pen,T(pen).move(w,h));
			blur_x-=2;
		}
		else
			blur_x--;
	}
	while(blur_y)
	{
		if(blur_y>=2)
		{
			gaussian_blur_1x3(pen,T(pen).move(w,h));
			blur_y-=2;
		}
		else
			blur_y--;
	}

	delete [] SC0;
	delete [] SC1;
	delete [] SC2;
	delete [] SC3;
}

template<typename T> void
gaussian_blur(T begin, T end,int w, int h)
{
	typename T::difference_type size(end-begin);
	gaussian_blur(begin,size.x,size.y,w,h);
}

template<typename T> void
gaussian_blur(T begin, T end,int w)
{
	typename T::difference_type size(end-begin);
	gaussian_blur(begin,size.x,size.y,w,w);
}

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
