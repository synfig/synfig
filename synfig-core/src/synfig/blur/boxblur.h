/* === S Y N F I G ========================================================= */
/*! \file boxblur.h
** \brief Box Blur Template Implementation
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

#ifndef SYNFIG_BOXBLUR_H
#define SYNFIG_BOXBLUR_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/**
 * Blur every row of a sample block based only on the samples of the same horizontal line.
 *
 * The formula for a blured sample is: ?
 *
 * @param pen the initial point of the sample block
 * @param w the block width
 * @param h the block height
 * @param length how many adjacent samples influence the sample bluring. 0 means no blur
 * @param outpen where to write the blurred block
 */
template<typename T1,typename T2> void
hbox_blur(T1 pen,int w, int h, int length, T2 outpen)
{
	int x,y;
   	typename T1::iterator_x iter, end;

	if (w < length)
		length = w;
	const float divisor(1.0f/(length*2+1));

	for(y=0;y<h;y++,pen.inc_y(),outpen.inc_y())
	{
		iter=pen.x();
		end=pen.end_x();

		typename T1::value_type tot((*iter)*(length+1));

		for (x=0;x<length && iter!=end;x++,++iter) tot+=*iter;
		iter=pen.x();

		for (x=0;x<w && iter!=end;x++,++iter,outpen.inc_x())
		{
			tot -= (x>length) ?
				iter[-length-1] :
				*pen.x();
			tot += ((x+length)<w) ?
				iter[length] :
				end[-1];
									  
			outpen.put_value((typename T2::value_type)(tot*divisor));
		}
		outpen.dec_x(x);
	}
}

/**
 * Blur every column of a sample block based only on the samples of the same vertical line.
 *
 * The formula for a blured sample is: ?
 *
 * @param pen the initial point of the sample block
 * @param w the block width
 * @param h the block height
 * @param length how many adjacent samples influence the sample bluring
 * @param outpen where to write the blurred block
 */
template<typename T1,typename T2> void
vbox_blur(T1 pen,const int w, const int h, int length, T2 outpen)
{
	int x,y;
   	typename T1::iterator_y iter, end;

	if (h < length)
		length = h;
	const float divisor(1.0f/(length*2+1));

	for(x=0;x<w;x++,pen.inc_x(),outpen.inc_x())
	{
		iter=pen.y();
		end=pen.end_y();

		typename T1::value_type tot((*iter)*(length+1));

		for (y=0;y<length && iter!=end;y++,++iter) tot+=*iter;
		iter=pen.y();

		for (y=0;y<h && iter!=end;y++,++iter,outpen.inc_y())
		{
			tot -= (y>length) ?
				iter[-length-1] :
				*pen.y();
			tot += ((y+length)<h) ?
				iter[length] :
				end[-1];

			outpen.put_value((typename T2::value_type)(tot*divisor));
		}
		outpen.dec_y(y);
	}
}

/**
 * Blur every row of a sample block based only on the samples of the same horizontal line.
 *
 * @param begin the pen on the initial point of the sample block
 * @param end the pen on the final point of the sample block
 * @param len how many adjacent samples influence the sample bluring
 * @param outpen where to write the blurred block
 *
 * @see void hbox_blur(T1 pen,const int w, const int h, int length, T2 outpen)
 */
template<typename T1,typename T2> void
hbox_blur(T1 begin,T1 end, int len,T2 outpen)
{
	typename T1::difference_type size(end-begin);
	hbox_blur(begin,size.x,size.y,len,outpen);
}

/**
 * Blur every column of a sample block based only on the samples of the same vertical line.
 *
 * @param begin the pen on the initial point of the sample block
 * @param end the pen on the final point of the sample block
 * @param len how many adjacent samples influence the sample bluring
 * @param outpen where to write the blurred block
 *
 * @see void vbox_blur(T1 pen,const int w, const int h, int length, T2 outpen)
 */
template<typename T1,typename T2> void
vbox_blur(T1 begin,T1 end, int len,T2 outpen)
{
	typename T1::difference_type size(end-begin);
	vbox_blur(begin,size.x,size.y,len,outpen);
}

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif // SYNFIG_BOXBLUR_H
