/*! ========================================================================
** Extended Template Library
** \file _boxblur.h
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
**
** \note
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__BOXBLUR_H
#define __ETL__BOXBLUR_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

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

template<typename T1,typename T2> void
hbox_blur(T1 begin,T1 end, int len,T2 outpen)
{
	typename T1::difference_type size(end-begin);
	hbox_blur(begin,size.x,size.y,len,outpen);
}

template<typename T1,typename T2> void
vbox_blur(T1 begin,T1 end, int len,T2 outpen)
{
	typename T1::difference_type size(end-begin);
	vbox_blur(begin,size.x,size.y,len,outpen);
}

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
