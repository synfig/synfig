/* === S Y N F I G ========================================================= */
/*! \file gaussian.h
** \brief Gaussian Blur Template Implementation
** \internal
**
** \legal
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
** \endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_GAUSSIAN_H
#define SYNFIG_GAUSSIAN_H

/* === H E A D E R S ======================================================= */

#include <cstring>		// for memset()

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/**
 * 2D 5x5 pixel gaussian blur
 * @param begin pen at the initial point
 * @param w width of area to be blurred
 * @param h height of area to be blurred
 * @param SC0 empty array with (width + 2) elements, used for caching and avoiding repeated memory allocation
 * @param SC1 empty array with (width + 2) elements, used for caching and avoiding repeated memory allocation
 * @param SC2 empty array with (width + 2) elements, used for caching and avoiding repeated memory allocation
 * @param SC3 empty array with (width + 2) elements, used for caching and avoiding repeated memory allocation
*/
template<typename T> void
gaussian_blur_5x5_(T pen,int w, int h,
typename T::pointer SC0,
typename T::pointer SC1,
typename T::pointer SC2,
typename T::pointer SC3)
{
	int x,y;
	typename T::value_type Tmp1,Tmp2,SR0,SR1,SR2,SR3;

	// Setup the row buffers
	SC0[0] = SC0[1] = typename T::value_type();
	for(x=0;x<w;x++)SC0[x+2]=(pen.x()[x])*24;
	memset((void *)SC1,0,(w+2)*sizeof(typename T::value_type));
	memset((void *)SC2,0,(w+2)*sizeof(typename T::value_type));
	memset((void *)SC3,0,(w+2)*sizeof(typename T::value_type));

	for(y=0;y<h+2;y++,pen.inc_y())
	{
		int yadj;
		if(y>=h)
			{yadj=(h-y)-1; SR0=(pen.y()[yadj])*1.35;}
		else
			{yadj=0; SR0=(pen.get_value())*1.35; }

		SR1=SR2=SR3=typename T::value_type();
		for(x=0;x<w+2;x++,pen.inc_x())
		{
			if(x>=w)
				Tmp1=pen[yadj][(w-x)-1];
			else
				Tmp1=*pen[yadj];

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

/**
 * 2D 3x3 pixel gaussian blur
 * @param begin pen at the initial point
 * @param w width of area to be blurred
 * @param h height of area to be blurred
 * @param SC0 empty array with (width + 1) elements, used for caching and avoiding repeated memory allocation
 * @param SC1 empty array with (width + 1) elements, used for caching and avoiding repeated memory allocation
 */
template<typename T> void
gaussian_blur_3x3(T pen,int w, int h, typename T::pointer SC0, typename T::pointer SC1)
{
	int x,y;
	typename T::value_type Tmp1,Tmp2,SR0,SR1;


	// Setup the row buffers
	SC0[0] = typename T::value_type();
	for(x=0;x<w;x++)SC0[x+1]=(pen.x()[x])*4;
	memset((void *)SC1,0,(w+1)*sizeof(typename T::value_type));

	for(y=0;y<h+1;y++,pen.inc_y())
	{
		int yadj;
		if(y>=h)
			{yadj=-1; SR1=SR0=pen.y()[yadj];}
		else
			{yadj=0; SR1=SR0=pen.get_value(); }

		for(x=0;x<w+1;x++,pen.inc_x())
		{
			if(x>=w)
				Tmp1=pen[yadj][(w-x)-2];
			else
				Tmp1=*pen[yadj];

			Tmp2=SR0+Tmp1;
			SR0=Tmp1;
			Tmp1=SR1+Tmp2;
			SR1=Tmp2;

			Tmp2=SC0[x]+Tmp1;
			SC0[x]=Tmp1;
			if(y&&x)
				pen[-1][-1]=(SC1[x]+Tmp2)/16;
			SC1[x]=Tmp2;
		}
		pen.dec_x(x);
	}

}

/**
 * 2D 3x3 pixel gaussian blur
 * @param begin pen at the initial point
 * @param end pen at the final point
 * @param SC0 empty array with (width + 1) elements, used for caching and avoiding repeated memory allocation
 * @param SC1 empty array with (width + 1) elements, used for caching and avoiding repeated memory allocation
 */
template<typename _PEN> void
gaussian_blur_3x3(_PEN begin, _PEN end, typename _PEN::pointer SC0, typename _PEN::pointer SC1)
{
	typename _PEN::difference_type size(end-begin);
	gaussian_blur_3x3(begin,size.x,size.y, SC0, SC1);
}

/**
 * 1D 3 pixel gaussian blur
 */
template<typename I> void
gaussian_blur_3(I begin, I end, bool endpts = true)
{
	auto SR0 = *begin;
	auto SR1 = SR0;
	decltype(SR0) Tmp1, Tmp2;
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

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif // SYNFIG_GAUSSIAN_H
