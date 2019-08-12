/*! ========================================================================
** Extended Template Library
** Box Blur Template Implementation
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

#ifndef __ETL__BOXBLUR_H
#define __ETL__BOXBLUR_H

/* === H E A D E R S ======================================================= */

#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template<typename T1,typename T2> void
hbox_blur(T1 pen,int w, int h, int length, T2 outpen)
{
	int x,y;
   	typename T1::iterator_x iter, end;

	length=std::min(w,length);
	const float divisor(1.0f/(length*2+1));

	for(y=0;y<h;y++,pen.inc_y(),outpen.inc_y())
	{
		iter=pen.x();
		end=pen.end_x();

		typename T1::accumulator_type tot((typename T1::accumulator_type)(*iter)*(length+1));

		for (x=0;x<length && iter!=end;x++,++iter) tot+=*iter;
		iter=pen.x();

		for (x=0;x<w && iter!=end;x++,++iter,outpen.inc_x())
		{
			tot -= (x>length) ?
				(typename T1::accumulator_type)(iter[-length-1]) :
				(typename T1::accumulator_type)(*pen.x());
			tot += ((x+length)<w) ?
				(typename T1::accumulator_type)(iter[length]) :
				(typename T1::accumulator_type)(end[-1]);
									  
			outpen.put_value((typename T2::value_type)(tot*divisor));
		}
		outpen.dec_x(x);
	}
}

#if 1
template<typename T1,typename T2> void
vbox_blur(T1 pen,const int w, const int h, int length, T2 outpen)
{
	int x,y;
   	typename T1::iterator_y iter, end;

	length=std::min(h,length);
	const float divisor(1.0f/(length*2+1));

	for(x=0;x<w;x++,pen.inc_x(),outpen.inc_x())
	{
		iter=pen.y();
		end=pen.end_y();

		typename T1::accumulator_type tot((typename T1::accumulator_type)(*iter)*(length+1));

		for (y=0;y<length && iter!=end;y++,++iter) tot+=*iter;
		iter=pen.y();

		for (y=0;y<h && iter!=end;y++,++iter,outpen.inc_y())
		{
			tot -= (y>length) ?
			(typename T1::accumulator_type)(iter[-length-1]) :
			(typename T1::accumulator_type)(*pen.y());
			tot += ((y+length)<h) ?
			(typename T1::accumulator_type)(iter[length]) :
			(typename T1::accumulator_type)(end[-1]);

			outpen.put_value((typename T2::value_type)(tot*divisor));
		}
		outpen.dec_y(y);
	}
}

#else

template<typename T1,typename T2> void
vbox_blur(T1 pen,int w, int h, int length,T2 outpen)
{
	int x,y;
   	typename T1::iterator_y iter, end, biter,eiter;

	//print out the info I need to figure out if this is somehow a buffer overrun...
	/*char *beginptr=0,*endptr=0;
	{
		T1 ypen = pen;
		T1 endpen = pen;
		endpen.move(w,h);
		ypen.inc_y();

		T2 	open = outpen,
			oepen = outpen;
		oepen.move(w,h);
		printf("V Blur (%d,%d,s-%d) in(%p,%p,st %d) out(%p,%p)\n",
				w,h,length,(char*)pen.x(),(char*)endpen.x(),(char*)ypen.x()-(char*)pen.x(),
				(char*)open.x(),(char*)oepen.x());
	}*/
	length=min(h-1,length);

	const float divisor(1.0f/(length*2+1));
	//const int div = (length*2+1);

	//since the filter range is 2*length+1 we need h-1
	for(x=0;x<w;x++,pen.inc_x(),outpen.inc_x())
	{
		iter=pen.y();
		end=pen.end_y();

		const typename T1::value_type bval = *iter;
		const typename T1::value_type eval = end[-1];

		typename T1::accumulator_type tot(bval*(length+1));
		//beginptr = (char*)&*iter; endptr = (char*)&*end;

		//printf("\nx line %d (%p,%p)\n",x,beginptr,endptr);

		//printf("Init %.3f - ",tot);
		for (y=0;y<length && iter!=end;y++)
		{
			tot+=iter[y];
			//printf("(%d,%p,+%.3f->%.3f),",y,&iter[y],iter[y],tot);
		}
		iter=pen.y();

		//printf(" tot=%.3f\n",tot);

		biter = iter+(-length-1); //get the first one...
		eiter = iter+length;

		//y will always be > length
		//T2 open = outpen;
		for (y=0;y<h && iter!=end;y++,++iter,++biter,++eiter,outpen.inc_y())
		{
			//printf("y line %d - (%f) ",y,tot);

			if (y>length)
			{
				typename T1::value_type &v = *biter;
				/*if( (char*)&v < beginptr ||
					(char*)&v >= endptr)
					printf("crap! %d (%p off %p)\n",y,(char*)&v,(char*)&*iter);*/
				tot -= v;
				//printf("[%.3f,",v);
			}
			else
			{
				tot -= bval;
				//printf("[%.3f,",bval);
			}

			if (y+length<h)
			{
				typename T1::value_type &v = *eiter;
				/*if( (char*)&v < beginptr ||
					(char*)&v >= endptr)
					printf("crap! %d (%p off %p)\n",y,(char*)&v,(char*)&*iter);*/
				tot += v;
				//printf("%.3f]",v);
			}
			else
			{
				tot += eval;
				//printf("%.3f]",eval);
			}

			//test handled in the previous case...
			//tot -= (y>length) ? *biter : bval;
			//tot += (y+length<h) ? *eiter : eval;

			//printf(" - %.3f\n",tot);
			outpen.put_value(tot*divisor);
		}
		outpen.dec_y(y);
	}
}
#endif

template<typename T1,typename T2> void
box_blur(T1 pen,int w, int h, int blur_w, int blur_h, T2 outpen)
	{ hbox_blur(pen,w,h,blur_w,outpen); vbox_blur(pen,w,h,blur_h,outpen); }

template<typename T1,typename T2> void
box_blur(T1 pen,int w, int h, int size, T2 outpen)
	{ hbox_blur(pen,w,h,size,outpen); vbox_blur(pen,w,h,size,outpen); }

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

template<typename T1,typename T2> void
box_blur(T1 begin,T1 end, int blur_w, int blur_h,T2 outpen)
{
	typename T1::difference_type size(end-begin);
	hbox_blur(begin,size.x,size.y,blur_w,outpen); vbox_blur(begin,size.x,size.y,blur_h,outpen);
}

template<typename T1,typename T2> void
box_blur(T1 begin,T1 end, int blursize,T2 outpen)
{
	typename T1::difference_type size(end-begin);
	hbox_blur(begin,size.x,size.y,blursize,outpen); vbox_blur(begin,size.x,size.y,blursize,outpen);
}

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
