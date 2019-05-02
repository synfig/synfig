/* === S Y N F I G ========================================================= */
/*!	\file blineconvert.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include <synfig/general.h>

#include "blineconvert.h"
#include <vector>
#include <ETL/gaussian>
#include <ETL/hermite>
#include <ETL/clock>
#include <float.h>
#include <algorithm>
#include <cassert>

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define EPSILON		(1e-10)

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


//Derivative Functions for numerical approximation

//bias == 0 will get F' at f3, bias < 0 will get F' at f1, and bias > 0 will get F' at f5
template < class T >
inline void FivePointdt(T &df, const T &f1, const T &f2, const T &f3, const T &f4, const T &f5, int bias)
{
	if (bias == 0)				// middle
		df = (f1 - f2*8 + f4*8 - f5)*(1/12.0f);
	else if (bias < 0)			// left
		df = (-f1*25 + f2*48 - f3*36 + f4*16 - f5*3)*(1/12.0f);
	else						// right
		df = (f1*3 - f2*16 + f3*36 - f4*48 + f5*25)*(1/12.0f);
}

template < class T >
inline void ThreePointdt(T &df, const T &f1, const T &f2, const T &f3, int bias)
{
	if (bias == 0)				// middle
		df = (-f1 + f3)*(1/2.0f);
	else if (bias < 0)			// left
		df = (-f1*3 + f2*4 - f3)*(1/2.0f);
	else						// right
		df = (f1 - f2*4 + f3*3)*(1/2.0f);
}

// template < class T >
// inline void ThreePointddt(T &df, const T &f1, const T &f2, const T &f3, int bias)
// {
// 	// a 3 point approximation pretends to have constant acceleration,
// 	// so only one algorithm needed for left, middle, or right
// 	df = (f1 -f2*2 + f3)*(1/2.0f);
// }
//
// // WARNING -- totally broken
// template < class T >
// inline void FivePointddt(T &df, const T &f1, const T &f2, const T &f3, int bias)
// {
// 	if(bias == 0)
// 	{
// 		assert(0); // !?
// 		//middle
// 		//df = (- f1 + f2*16 - f3*30 +  f4*16 - f5)*(1/12.0f);
// 	}/*else if(bias < 0)
// 	{
// 		//left
// 		df = (f1*7 - f2*26*4 + f3*19*6 - f4*14*4 + f5*11)*(1/12.0f);
// 	}else
// 	{
// 		//right
// 		df = (f1*3 - f2*16 + f3*36 - f4*48 + f5*25)*(1/12.0f);
// 	}*/
// 	//side ones don't work, use 3 point
// }
//
// //implement an arbitrary derivative
// //dumb algorithm
// template < class T >
// void DerivativeApprox(T &df, const T f[], const Real t[], int npoints, int indexval)
// {
// 	/*
// 	Lj(x) = PI_i!=j (x - xi) / PI_i!=j (xj - xi)
//
// 	so Lj'(x) = SUM_k PI_i!=j|k (x - xi) / PI_i!=j (xj - xi)
// 	*/
//
// 	unsigned int i,j,k,i0,i1;
//
// 	Real Lpj,mult,div,tj;
// 	Real tval = t[indexval];
//
// 	//sum k
// 	for(j=0;j<npoints;++j)
// 	{
// 		Lpj = 0;
// 		div = 1;
// 		tj = t[j];
//
// 		for(k=0;k<npoints;++k)
// 		{
// 			if(k != j) //because there is no summand for k == j, since that term is missing from the original equation
// 			{
// 				//summation for k
// 				for(i=0;i<npoints;++i)
// 				{
// 					if(i != k)
// 					{
// 						mult *= tval - t[i];
// 					}
// 				}
//
// 				Lpj += mult; //add into the summation
//
// 				//since the ks follow the exact pattern we need for the divisor (use that too)
// 				div *= tj - t[k];
// 			}
// 		}
//
// 		//get the actual coefficient
// 		Lpj /= div;
//
// 		//add it in to the equation
// 		df += f[j]*Lpj;
// 	}
// }

//END numerical derivatives

// template < class T >
// inline int sign(T f, T tol)
// {
// 	if(f < -tol) return -1;
// 	if(f > tol) return 1;
// 	return 0;
// }

void GetFirstDerivatives(const std::vector<synfig::Point> &f, unsigned int left, unsigned int right, char *out, unsigned int dfstride)
{
	unsigned int current = left;

	if(right - left < 2)
		return;
	else if(right - left == 2)
	{
		synfig::Vector v = f[left+1] - f[left];

		//set both to the one we want
		*(synfig::Vector*)out = v;
		out += dfstride;
		*(synfig::Vector*)out = v;
		out += dfstride;
	}
	else if(right - left < 6/*5*/) //should use 3 point
	{
		//left then middle then right
		ThreePointdt(*(synfig::Vector*)out,f[left+0], f[left+1], f[left+2], -1);
		current++;
		out += dfstride;

		for(;current < right-1; current++, out += dfstride)
			ThreePointdt(*(synfig::Vector*)out,f[current-1], f[current], f[current+1], 0);

		ThreePointdt(*(synfig::Vector*)out,f[right-3], f[right-2], f[right-1], 1);
		current++;
		out += dfstride;

	}else //can use 5 point
	{
		//left 2 then middle bunch then right two
		//may want to use 3 point for inner edge ones

		FivePointdt(*(synfig::Vector*)out,f[left+0], f[left+1], f[left+2], f[left+3], f[left+4], -2);
		out += dfstride;
		FivePointdt(*(synfig::Vector*)out,f[left+1], f[left+2], f[left+3], f[left+4], f[left+5], -1);
		out += dfstride;
		current += 2;

		for(;current < right-2; current++, out += dfstride)
			FivePointdt(*(synfig::Vector*)out,f[current-2], f[current-1], f[current], f[current+1], f[current+2], 0);

		FivePointdt(*(synfig::Vector*)out,f[right-6], f[right-5], f[right-4], f[right-3], f[right-2], 2);
		out += dfstride;
		FivePointdt(*(synfig::Vector*)out,f[right-5], f[right-4], f[right-3], f[right-2], f[right-1], 1);
		out += dfstride;
		current += 2;
	}
}

void GetSimpleDerivatives(const std::vector<synfig::Point> &f, int left, int right,
							std::vector<synfig::Point> &df, int outleft,
							const std::vector<synfig::Real> &/*di*/)
{
	int i1,i2,i;
	int offset = 2; //df = 1/2 (f[i+o]-f[i-o])

	assert((int)df.size() >= right-left+outleft); //must be big enough

	for(i = left; i < right; ++i)
	{
		//right now indices (figure out distance later)
		i1 = std::max(left,i-offset);
		i2 = std::max(left,i+offset);

		df[outleft++] = (f[i2] - f[i1])*0.5f;
	}
}

//get the curve error from the double sample list of work points (hopefully that's enough)
Real CurveError(const synfig::Point *pts, unsigned int n, std::vector<synfig::Point> &work, int left, int right)
{
	if(right-left < 2) return -1;

	int i,j;

	//get distances to each point
	Real d,dtemp,dsum;
	//synfig::Vector v,vt;
	//synfig::Point p1,p2;
	synfig::Point pi;
	std::vector<synfig::Point>::const_iterator it;//,end = work.begin()+right;

	//unsigned int size = work.size();

	//for each line, get distance
	d = 0; //starts at 0
	for(i = 0; i < (int)n; ++i)
	{
		pi = pts[i];

		dsum = FLT_MAX;

		it = work.begin()+left;
		//p2 = *it++; //put it at left+1
		for(j = left/*+1*/; j < right; ++j,++it)
		{
			/*p1 = p2;
			p2 = *it;

			v = p2 - p1;
			vt = pi - p1;

			dtemp = v.mag_squared() > 1e-12 ? (vt*v)/v.mag_squared() : 0; //get the projected time value for the current line

			//get distance to line segment with the time value clamped 0-1
			if(dtemp >= 1)	//use p+v
			{
				vt += v; //makes it pp - (p+v)
			}else if(dtemp > 0)	//use vt-proj
			{
				vt -= v*dtemp; // vt - proj_v(vt)	//must normalize the projection vector to work
			}

			//else use p
			dtemp = vt.mag_squared();*/

			dtemp = (pi - *it).mag_squared();
			if(dtemp < dsum)
				dsum = dtemp;
		}

		//accumulate the points' min distance from the curve
		d += sqrt(dsum);
	}

	return d;
}

typedef synfigapp::BLineConverter::cpindex cpindex;

//has the index data and the tangent scale data (relevant as it may be)
int tessellate_curves(const std::vector<cpindex> &inds, const std::vector<Point> &f, const std::vector<synfig::Vector> &df, std::vector<Point> &work)
{
	if(inds.size() < 2)
		return 0;

	etl::hermite<Point>	curve;
	int ntess = 0;

	std::vector<cpindex>::const_iterator j = inds.begin(),j2, end = inds.end();

	unsigned int ibase = inds[0].curind;

	j2 = j++;
	for(; j != end; j2 = j++)
	{
		//if this curve has invalid error (in j) then retessellate its work points (requires reparametrization, etc.)
		if(j->error < 0)
		{
			//get the stepsize etc. for the number of points in here
			unsigned int n = j->curind - j2->curind + 1; //that's the number of points in the span
			unsigned int k, kend, i0, i3;
			//so reset the right chunk

			Real t, dt = 1/(Real)(n*2-2); //assuming that they own only n points

			//start at first intermediate
			t = 0;

			i0 = j2->curind; i3 = j->curind;
			k = (i0-ibase)*2; //start on first intermediary point (2x+1)
			kend = (i3-ibase)*2; //last point to set (not intermediary)

			//build hermite curve, it's easier
			curve.p1() = f[i0];
			curve.p2() = f[i3];
			curve.t1() = df[i0-ibase] * (df[i0-ibase].mag_squared() > 1e-4
										 ? j2->tangentscale/df[i0-ibase].mag()
										 : j2->tangentscale);
			curve.t2() = df[i3-ibase] * (df[i3-ibase].mag_squared() > 1e-4
										 ? j->tangentscale/df[i3-ibase].mag()
										 : j->tangentscale);
			curve.sync();

			//MUST include the end point (since we are ignoring left one)
			for(; k < kend; ++k, t += dt)
			{
				work[k] = curve(t);
			}

			work[k] = curve(1); //k == kend, t == 1 -> c(t) == p2
			++ntess;
		}
	}

	return ntess;
}

synfigapp::BLineConverter::BLineConverter()
{
	pixelwidth = 1;
	smoothness = 0.70f;
	width = 0;
};

void
synfigapp::BLineConverter::clear()
{
	point_cache.clear();
	width_cache.clear();
	ftemp.clear();
	deriv.clear();
	curvature.clear();
	break_tangents.clear();
	cum_dist.clear();
	this_dist.clear();
	work.clear();
	curind.clear();
}

void
synfigapp::BLineConverter::operator()(std::list<synfig::BLinePoint>  &blinepoints_out,
									  const std::list<synfig::Point> &points_in,
									  const std::list<synfig::Real>  &widths_in)
{
	//Profiling information
	/*etl::clock::value_type initialprocess=0, curveval=0, breakeval=0, disteval=0;
	etl::clock::value_type preproceval=0, tesseval=0, erroreval=0, spliteval=0;
	unsigned int			numpre=0, numtess=0, numerror=0, numsplit=0;
	etl::clock_realtime timer,total;*/

	//total.reset();
	if (points_in.size() < 2)
		return;

	clear();

	//removing digitization error harder than expected

	//intended to fix little pen errors caused by the way people draw (tiny juts in opposite direction)
	//Different solutions
	//	Average at both end points (will probably eliminate many points at each end of the samples)
	//	Average after the break points are found (weird points would still affect the curve)
	//	Just always get rid of breaks at the beginning and end if they are a certain distance apart
	//		This is will be current approach so all we do now is try to remove duplicate points

	//remove duplicate points - very bad for fitting

	//timer.reset();

	{
		std::list<synfig::Point>::const_iterator point_iter = points_in.begin(), end = points_in.end();
		std::list<synfig::Real>::const_iterator	width_iter = widths_in.begin();
		synfig::Point c;

		if (points_in.size() == widths_in.size())
		{
			for(bool first = true; point_iter != end; ++point_iter,++width_iter)
				if (first || *point_iter != c)		// eliminate duplicate points
				{
					first = false;
					point_cache.push_back(c = *point_iter);
					width_cache.push_back(*width_iter);
				}
		}
		else
			for(;point_iter != end; ++point_iter)
				if(*point_iter != c)		// eliminate duplicate points
					point_cache.push_back(c = *point_iter);
	}
	//initialprocess = timer();

	if (point_cache.size() < 7)
	{
		info("only %d unique points - giving up", point_cache.size());
		return;
	}

	//get curvature information
	//timer.reset();

	{
		int i_this, i_prev, i_next;
		synfig::Vector v_prev, v_next;

		curvature.resize(point_cache.size());
		curvature.front() = curvature.back() = 1;

		for (i_this = 1; i_this < (int)point_cache.size()-1; i_this++)
		{
			i_prev = std::max(0, i_this-2);
			i_next = std::min((int)(point_cache.size()-1), i_this+2);

			v_prev = point_cache[i_this] - point_cache[i_prev];
			v_next = point_cache[i_next] - point_cache[i_this];

			curvature[i_this] = (v_prev*v_next) / (v_prev.mag()*v_next.mag());
		}
	}

	//curveval = timer();
	//synfig::info("calculated curvature");

	//find corner points and interpolate inside those
	//timer.reset();
	{
		//break at sharp derivative points
		//TODO tolerance should be set based upon digitization resolution (length dependent index selection)
		Real	tol = 0;		//break tolerance, for the cosine of the change in angle (really high curvature or something)
		unsigned int i = 0;

		int		sharpest_i=-1;
		int		last=0;
		Real	sharpest_curvature = 1;

		break_tangents.push_back(0);

		// loop through the curvatures; in each continuous run of
		// curvatures that exceed the tolerance, find the one with the
		// sharpest curvature and add its index to the list of indices
		// at which to split tangents
		for (i = 1; i < curvature.size()-1; ++i)
		{
			if (curvature[i] < tol)
			{
				if(curvature[i] < sharpest_curvature)
				{
					sharpest_curvature = curvature[i];
					sharpest_i = i;
				}
			}
			else if (sharpest_i > 0)
			{
				// don't have 2 corners too close to each other
				if (sharpest_i >= last + 8) //! \todo make this configurable
				{
					//synfig::info("break: %d-%d",sharpest_i+1,curvature.size());
					break_tangents.push_back(sharpest_i);
					last = sharpest_i;
				}
				sharpest_i = -1;
				sharpest_curvature = 1;
			}
		}

		break_tangents.push_back(i);

// this section causes bug 1892566 if enabled
#if 1
		//postprocess for breaks too close to each other
		Real	fixdistsq = 4*width*width; //the distance to ignore breaks at the end points (for fixing stuff)
		Real d = 0;
		Point p = point_cache[break_tangents.front()];

		//first set
		for (i = 1; i < break_tangents.size()-1; ++i) //do not want to include end point...
		{
			d = (point_cache[break_tangents[i]] - p).mag_squared();
			if(d > fixdistsq) break; //don't want to group breaks if we ever get over the dist...
		}
		//want to erase all points before...
		if(i != 1)
			break_tangents.erase(break_tangents.begin(),break_tangents.begin()+i-1);

		//end set
		p = point_cache[break_tangents.back()];
		for(i = break_tangents.size()-2; i > 0; --i) //start at one in from the end
		{
			d = (point_cache[break_tangents[i]] - p).mag_squared();
			if(d > fixdistsq) break; //don't want to group breaks if we ever get over the dist
		}
		if(i != break_tangents.size()-2)
			break_tangents.erase(break_tangents.begin()+i+2,break_tangents.end()); //erase all points that we found... found none if i has not advanced
		//must not include the one we ended up on
#endif
	}
	//breakeval = timer();
	//synfig::info("found break points: %d",break_tangents.size());

	//get the distance calculation of the entire curve (for tangent scaling)

	//timer.reset();
	{
		synfig::Point p1,p2;

		p1=p2=point_cache[0];

		cum_dist.resize(point_cache.size()); this_dist.resize(point_cache.size());
		Real d = 0;
		for(unsigned int i = 0; i < point_cache.size();)
		{
			d += (this_dist[i] = (p2-p1).mag());
			cum_dist[i] = d;

			p1=p2;
			//! \todo is this legal?  it reads off the end of the vector
			p2=point_cache[++i];
		}
	}
	//disteval = timer();
	//synfig::info("calculated distance");

	//now break at every point - calculate new derivatives each time

	//TODO
	//must be sure that the break points are 3 or more apart
	//then must also store the breaks which are not smooth, etc.
	//and figure out tangents between there

	//for each pair of break points (as long as they are far enough apart) recursively subdivide stuff
	//ignore the detected intermediate points
	{
		unsigned int i0=0,i3=0,is=0;
		int i=0,j=0;

		bool done = false;

		Real errortol = smoothness*pixelwidth; //???? what should this value be

		BLinePoint a;
		synfig::Vector v;

		//intemp = f; //don't want to smooth out the corners

		bool breaktan = false, setwidth;
		a.set_split_tangent_both(false);
		//a.set_width(width);
		a.set_width(1.0f);

		setwidth = (point_cache.size() == width_cache.size());

		for(j = 0; j < (int)break_tangents.size() - 1; ++j)
		{
			//for b[j] to b[j+1] subdivide and stuff
			i0 = break_tangents[j];
			i3 = break_tangents[j+1];

			unsigned int size = i3-i0+1; //must include the end points

			//new derivatives
			//timer.reset();
			ftemp.assign(point_cache.begin()+i0, point_cache.begin()+i3+1);
			for(i=0;i<20;++i)
				gaussian_blur_3(ftemp.begin(),ftemp.end(),false);

			deriv.resize(size);

			// Wondering whether the modification of the deriv vector
			// using a char* pointer and pointer arithmetric was safe,
			// I looked it up...
			//
			// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2369.pdf tells me:
			//
			//	23.2.5  Class template vector [vector]
			//
			//	[...] The elements of a vector are stored contiguously,
			//	meaning that if v is a vector<T,Allocator> where T is
			//	some type other than bool, then it obeys the identity
			//	&v[n] == &v[0] + n for all 0 <= n < v.size().
			//
			GetFirstDerivatives(ftemp,0,size,(char*)&deriv[0],sizeof(deriv[0]));

			//GetSimpleDerivatives(ftemp,0,size,deriv,0,cum_dist);
			//< don't have to worry about indexing stuff as it is all being taken care of right now
			//preproceval += timer();
			//numpre++;

			work.resize(size*2-1); //guarantee that all points will be tessellated correctly (one point in between every 2 adjacent points)

			//if size of work is size*2-1, the step size should be 1/(size*2 - 2)
			//Real step = 1/(Real)(size*2 - 1);

			//start off with break points as indices
			curind.clear();
			curind.push_back(cpindex(i0,cum_dist[i3]-cum_dist[i0],0)); //0 error because no curve on the left
			curind.push_back(cpindex(i3,cum_dist[i3]-cum_dist[i0],-1)); //error needs to be reevaluated
			done = false; //we want to loop

			unsigned int dcount = 0;

			//while there are still enough points between us, and the error is too high subdivide (and invalidate neighbors that share tangents)
			while(!done)
			{
				//tessellate all curves with invalid error values
				work[0] = point_cache[i0];

				//timer.reset();
				/*numtess += */tessellate_curves(curind,point_cache,deriv,work);
				//tesseval += timer();

				//now get all error values
				//timer.reset();
				for(i = 1; i < (int)curind.size(); ++i)
				{
					if(curind[i].error < 0) //must have been retessellated, so now recalculate error value
					{
						//evaluate error from points (starting at current index)
						int size = curind[i].curind - curind[i-1].curind + 1;
						curind[i].error = CurveError(&point_cache[curind[i-1].curind], size,
													 work,(curind[i-1].curind - i0)*2,(curind[i].curind - i0)*2+1);

						/*if(curind[i].error > 1.0e5)
						{
							synfig::info("Holy crap %d-%d error %f",curind[i-1].curind,curind[i].curind,curind[i].error);
							curind[i].error = -1;
							numtess += tessellate_curves(curind,f,deriv,work);
							curind[i].error = CurveError(&point_cache[curind[i-1].curind], size,
													 work,0,work.size());//(curind[i-1].curind - i0)*2,(curind[i].curind - i0)*2+1);
						}*/
						//numerror++;
					}
				}
				//erroreval += timer();

				//assume we're done
				done = true;

				//check each error to see if it's too big, if so, then subdivide etc.
				int indsize = (int)curind.size();
				Real maxrelerror = 0;
				int maxi = -1;//, numpoints;

				//timer.reset();
				//get the maximum error and split there
				for(i = 1; i < indsize; ++i)
				{
					//numpoints = curind[i].curind - curind[i-1].curind + 1;

					if(curind[i].error > maxrelerror && curind[i].curind - curind[i-1].curind > 2) //only accept if it's valid
					{
						maxrelerror = curind[i].error;
						maxi = i;
					}
				}

				//split if error is too great
				if(maxrelerror > errortol)
				{
					//add one to the left etc
					unsigned int 	ibase = curind[maxi-1].curind, itop = curind[maxi].curind,
									ibreak = (ibase + itop)/2;
					Real scale, scale2;

					assert(ibreak < point_cache.size());

					//synfig::info("Split %d -%d- %d, error: %f", ibase,ibreak,itop,maxrelerror);

					if(ibase != itop)
					{
						//invalidate current error of the changed tangents and add an extra segment
						//enforce minimum tangents property
						curind[maxi].error = -1;
						curind[maxi-1].error = -1;
						if(maxi+1 < indsize) curind[maxi+1].error = -1; //if there is a curve segment beyond this it will be effected as well

						scale = cum_dist[itop] - cum_dist[ibreak];
						scale2 = maxi+1 < indsize ? cum_dist[curind[maxi+1].curind] - cum_dist[itop] : scale; //to the right valid?
						curind[maxi].tangentscale = std::min(scale, scale2);

						scale = cum_dist[ibreak] - cum_dist[ibase];
						scale2 = maxi >= 2 ? cum_dist[ibase] - cum_dist[curind[maxi-2].curind] : scale; // to the left valid -2 ?
						curind[maxi-1].tangentscale = std::min(scale, scale2);

						scale = std::min(cum_dist[ibreak] - cum_dist[ibase], cum_dist[itop] - cum_dist[ibreak]);

						curind.insert(curind.begin()+maxi,cpindex(ibreak, scale, -1));
						//curind.push_back(cpindex(ibreak, scale, -1));
						//std::sort(curind.begin(), curind.end());

						done = false;
						//numsplit++;
					}
				}
				//spliteval += timer();

				dcount++;
			}

			//insert the last point too (just set tangent for now
			is = curind[0].curind;

			//first point inherits current tangent status
			v = deriv[is - i0];
			if(v.mag_squared() > EPSILON)
				v *= (curind[0].tangentscale/v.mag());

			if(!breaktan)
				a.set_tangent(v);
			else a.set_tangent2(v);

			a.set_vertex(point_cache[is]);
			if(setwidth)a.set_width(width_cache[is]);

			blinepoints_out.push_back(a);
			a.set_split_tangent_both(false); //won't need to break anymore
			breaktan = false;

			for(i = 1; i < (int)curind.size()-1; ++i)
			{
				is = curind[i].curind;

				//first point inherits current tangent status
				v = deriv[is-i0];
				if(v.mag_squared() > EPSILON)
					v *= (curind[i].tangentscale/v.mag());

				a.set_tangent(v); // always inside, so guaranteed to be smooth
				a.set_vertex(point_cache[is]);
				if(setwidth)a.set_width(width_cache[is]);

				blinepoints_out.push_back(a);
			}

			//set the last point's data
			is = curind.back().curind; //should already be this

			v = deriv[is-i0];
			if(v.mag_squared() > EPSILON)
				v *= (curind.back().tangentscale/v.mag());

			a.set_tangent1(v);
			a.set_split_tangent_both(true);
			breaktan = true;

			//will get the vertex and tangent 2 from next round
		}

		a.set_vertex(point_cache[i3]);
		a.set_split_tangent_both(false);
		if(setwidth)
			a.set_width(width_cache[i3]);
		blinepoints_out.push_back(a);

		/*etl::clock::value_type totaltime = total(),
							   misctime = totaltime - initialprocess - curveval - breakeval - disteval
									  - preproceval - tesseval - erroreval - spliteval;

		synfig::info(
			"Curve Convert Profile:\n"
			"\tInitial Preprocess:    %f\n"
			"\tCurvature Calculation: %f\n"
			"\tBreak Calculation:     %f\n"
			"\tDistance Calculation:  %f\n"
			"  Algorithm: (numtimes,totaltime)\n"
			"\tPreprocess step:      (%d,%f)\n"
			"\tTessellation step:    (%d,%f)\n"
			"\tError step:           (%d,%f)\n"
			"\tSplit step:           (%d,%f)\n"
			"  Num Input: %d, Num Output: %d\n"
			"  Total time: %f, Misc time: %f\n",
			initialprocess, curveval,breakeval,disteval,
			numpre,preproceval,numtess,tesseval,numerror,erroreval,numsplit,spliteval,
			points_in.size(),blinepoints_out.size(),
			totaltime,misctime);*/

		return;
	}
}

void synfigapp::BLineConverter::EnforceMinWidth(std::list<synfig::BLinePoint> &bline, synfig::Real min_pressure)
{
	std::list<synfig::BLinePoint>::iterator	i = bline.begin(),
											end = bline.end();

	for(i = bline.begin(); i != end; ++i)
		if(i->get_width() < min_pressure)
			i->set_width(min_pressure);
}
