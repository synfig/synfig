/* === S I N F G =========================================================== */
/*!	\file blineconvert.cpp
**	\brief Template File
**
**	$Id: blineconvert.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "blineconvert.h"
#include <vector>
#include <ETL/gaussian>
#include <ETL/hermite>
#include <ETL/clock>
#include <float.h>
#include <algorithm>
#include <sinfg/general.h>
#include <cassert>



#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

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
	if(bias == 0)
	{
		//middle
		df = (f1 - f2*8 + f4*8 - f5)*(1/12.0f);
	}else if(bias < 0)
	{
		//left
		df = (-f1*25 + f2*48 - f3*36 + f4*16 - f5*3)*(1/12.0f);
	}else
	{
		//right
		df = (f1*3 - f2*16 + f3*36 - f4*48 + f5*25)*(1/12.0f);
	}
}

template < class T >
inline void ThreePointdt(T &df, const T &f1, const T &f2, const T &f3, int bias)
{
	if(bias == 0)
	{
		//middle
		df = (-f1 + f3)*(1/2.0f);
	}else if(bias < 0)
	{
		//left
		df = (-f1*3 + f2*4 - f3)*(1/2.0f);
	}else
	{
		//right
		df = (f1 - f2*4 + f3*3)*(1/2.0f);
	}
}

template < class T >
inline void ThreePointddt(T &df, const T &f1, const T &f2, const T &f3, int bias)
{
	//a 3 point approximation pretends to have constant acceleration, so only one algorithm needed for left, middle, or right
	df = (f1 -f2*2 + f3)*(1/2.0f);
}

// WARNING -- totaly broken
template < class T >
inline void FivePointddt(T &df, const T &f1, const T &f2, const T &f3, int bias)
{
	if(bias == 0)
	{
		assert(0); // !?
		//middle
		//df = (- f1 + f2*16 - f3*30 +  f4*16 - f5)*(1/12.0f);
	}/*else if(bias < 0)
	{
		//left
		df = (f1*7 - f2*26*4 + f3*19*6 - f4*14*4 + f5*11)*(1/12.0f);
	}else
	{
		//right
		df = (f1*3 - f2*16 + f3*36 - f4*48 + f5*25)*(1/12.0f);
	}*/
	//side ones don't work, use 3 point
}

//implement an arbitrary derivative
//dumb algorithm
template < class T >
void DerivativeApprox(T &df, const T f[], const Real t[], int npoints, int indexval)
{
	/*
	Lj(x) = PI_i!=j (x - xi) / PI_i!=j (xj - xi)
	
	so Lj'(x) = SUM_k PI_i!=j|k (x - xi) / PI_i!=j (xj - xi)
	*/
	
	unsigned int i,j,k,i0,i1;
	
	Real Lpj,mult,div,tj;
	Real tval = t[indexval];
				
	//sum k	
	for(j=0;j<npoints;++j)
	{
		Lpj = 0;
		div = 1;
		tj = t[j];
		
		for(k=0;k<npoints;++k)
		{
			if(k != j) //because there is no summand for k == j, since that term is missing from the original equation
			{
				//summation for k
				for(i=0;i<npoints;++i)
				{
					if(i != k)
					{
						mult *= tval - t[i];
					}
				}
				
				Lpj += mult; //add into the summation
				
				//since the ks follow the exact patern we need for the divisor (use that too)
				div *= tj - t[k];
			}
		}
		
		//get the actual coefficient
		Lpj /= div;
		
		//add it in to the equation
		df += f[j]*Lpj;
	}
}

//END numerical derivatives

template < class T >
inline int sign(T f, T tol)
{
	if(f < -tol) return -1;
	if(f > tol) return 1;
	return 0;
}

void GetFirstDerivatives(const std::vector<sinfg::Point> &f, unsigned int left, unsigned int right, char *out, unsigned int dfstride)
{
	unsigned int current = left;

	if(right - left < 2)
		return;
	else if(right - left < 3)
	{
		sinfg::Vector v = f[left+1] - f[left];
		
		//set both to the one we want
		*(sinfg::Vector*)out = v;
		out += dfstride;
		*(sinfg::Vector*)out = v;
		out += dfstride;
	}
	else if(right - left < 6/*5*/) //should use 3 point
	{
		//left then middle then right
		ThreePointdt(*(sinfg::Vector*)out,f[left+0], f[left+1], f[left+2], -1);
		current += 1;
		out += dfstride;
		
		for(;current < right-1; current++, out += dfstride)
		{
			ThreePointdt(*(sinfg::Vector*)out,f[current-1], f[current], f[current+1], 0);
		}

		ThreePointdt(*(sinfg::Vector*)out,f[right-3], f[right-2], f[right-1], 1);
		current++;
		out += dfstride;
		
	}else //can use 5 point
	{
		//left 2 then middle bunch then right two
		//may want to use 3 point for inner edge ones
		
		FivePointdt(*(sinfg::Vector*)out,f[left+0], f[left+1], f[left+2], f[left+3], f[left+4], -2);
		out += dfstride;
		FivePointdt(*(sinfg::Vector*)out,f[left+1], f[left+2], f[left+3], f[left+4], f[left+5], -1);
		out += dfstride;
		current += 2;
		
		for(;current < right-2; current++, out += dfstride)
		{
			FivePointdt(*(sinfg::Vector*)out,f[current-2], f[current-1], f[current], f[current+1], f[current+2], 0);
		}

		FivePointdt(*(sinfg::Vector*)out,f[right-5], f[right-4], f[right-3], f[right-2], f[right-1], 1);
		out += dfstride;
		FivePointdt(*(sinfg::Vector*)out,f[right-6], f[right-5], f[right-4], f[right-3], f[right-2], 2);		
		out += dfstride;
		current += 2;
	}
}

void GetSimpleDerivatives(const std::vector<sinfg::Point> &f, int left, int right, 
							std::vector<sinfg::Point> &df, int outleft,
							const std::vector<sinfg::Real> &di)
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
Real CurveError(const sinfg::Point *pts, unsigned int n, std::vector<sinfg::Point> &work, int left, int right)
{
	if(right-left < 2) return -1;
		
	int i,j;
	
	//get distances to each point
	Real d,dtemp,dsum;
	//sinfg::Vector v,vt;
	//sinfg::Point p1,p2;
	sinfg::Point pi;
	std::vector<sinfg::Point>::const_iterator it;//,end = work.begin()+right;
	
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

typedef sinfgapp::BLineConverter::cpindex cpindex;

//has the index data and the tangent scale data (relevant as it may be)
int tesselate_curves(const std::vector<cpindex> &inds, const std::vector<Point> &f, const std::vector<sinfg::Vector> &df, std::vector<Point> &work)
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
		//if this curve has invalid error (in j) then retesselate it's work points (requires reparametrization, etc.)
		if(j->error < 0)
		{
			//get the stepsize etc. for the number of points in here
			unsigned int n = j->curind - j2->curind + 1; //thats the number of points in the span
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
			curve.t1() = df[i0]*(df[i0].mag_squared() > 1e-4 ? j2->tangentscale/df[i0].mag() : j2->tangentscale);
			curve.t2() = df[i3]*(df[i3].mag_squared() > 1e-4 ? j->tangentscale/df[i3].mag() : j->tangentscale);
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

sinfgapp::BLineConverter::BLineConverter()
{
	pixelwidth = 1;
	smoothness = 0.70f;
	width = 0;
};

void 
sinfgapp::BLineConverter::clear()
{
	f.clear();
	f_w.clear();
	ftemp.clear();
	df.clear();
	cvt.clear();
	brk.clear();
	di.clear();
	d_i.clear();
	work.clear();
	curind.clear();
}

void
sinfgapp::BLineConverter::operator () (std::list<sinfg::BLinePoint> &out, const std::list<sinfg::Point> &in,const std::list<sinfg::Real> &in_w)
{	
	//Profiling information
	/*etl::clock::value_type initialprocess=0, curveval=0, breakeval=0, disteval=0;
	etl::clock::value_type preproceval=0, tesseval=0, erroreval=0, spliteval=0;
	unsigned int			numpre=0, numtess=0, numerror=0, numsplit=0;
	etl::clock_realtime timer,total;*/

	//total.reset();
	if(in.size()<=1)
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
		std::list<sinfg::Point>::const_iterator i = in.begin(), end = in.end();
		std::list<sinfg::Real>::const_iterator	iw = in_w.begin();
		sinfg::Point 	c;
		
		if(in.size() == in_w.size())
		{
			for(;i != end; ++i,++iw)
			{	
				//eliminate duplicate points
				if(*i != c)
				{
					f.push_back(c = *i);
					f_w.push_back(*iw);
				}
			}
		}else
		{
			for(;i != end; ++i)
			{	
				//eliminate duplicate points
				if(*i != c)
				{
					f.push_back(c = *i);
				}
			}
		}
	}
	//initialprocess = timer();
	
	if(f.size()<=6)
		return;
	
	//get curvature information
	//timer.reset();
	
	{
		int i,i0,i1;
		sinfg::Vector v1,v2;
		
		cvt.resize(f.size());
		
		cvt.front() = 1;
		cvt.back() = 1;
		
		for(i = 1; i < (int)f.size()-1; ++i)
		{
			i0 = std::max(0,i - 2);
			i1 = std::min((int)(f.size()-1),i + 2);
			
			v1 = f[i] - f[i0];
			v2 = f[i1] - f[i];
	
			cvt[i] = (v1*v2)/(v1.mag()*v2.mag());
		}
	}
	
	//curveval = timer();
	//sinfg::info("calculated curvature");
	
	//find corner points and interpolate inside those
	//timer.reset();
	{		
		//break at sharp derivative points
		//TODO tolerance should be set based upon digitization resolution (length dependent index selection)
		Real	tol = 0;		//break tolerance, for the cosine of the change in angle (really high curvature or something)
		Real	fixdistsq = 4*width*width; //the distance to ignore breaks at the end points (for fixing stuff)
		unsigned int i = 0;
		
		int		maxi = -1, last=0;
		Real	minc = 1;
		
		brk.push_back(0);
		
		for(i = 1; i < cvt.size()-1; ++i)
		{			
			//insert if too sharp (we need to break the tangents to insert onto the break list)
			
			if(cvt[i] < tol)
			{
				if(cvt[i] < minc)
				{
					minc = cvt[i];
					maxi = i;
				}
			}else if(maxi >= 0)
			{
				if(maxi >= last + 8)
				{
					//sinfg::info("break: %d-%d",maxi+1,cvt.size());						
					brk.push_back(maxi);
					last = maxi;
				}
				maxi = -1;
				minc = 1;
			}
		}
		
		brk.push_back(i);
		
		//postprocess for breaks too close to eachother
		Real d = 0;
		Point p = f[brk.front()];
		
		//first set
		for(i = 1; i < brk.size()-1; ++i) //do not want to include end point...
		{
			d = (f[brk[i]] - p).mag_squared();
			if(d > fixdistsq) break; //don't want to group breaks if we ever get over the dist... 
		}
		//want to erase all points before...
		if(i != 1)
			brk.erase(brk.begin(),brk.begin()+i-1); 
		
		//end set
		p = f[brk.back()];
		for(i = brk.size()-2; i > 0; --i) //start at one in from the end
		{
			d = (f[brk[i]] - p).mag_squared();
			if(d > fixdistsq) break; //don't want to group breaks if we ever get over the dist
		}
		if(i != brk.size()-2)
			brk.erase(brk.begin()+i+2,brk.end()); //erase all points that we found... found none if i has not advanced
		//must not include the one we ended up on
	}
	//breakeval = timer();
	//sinfg::info("found break points: %d",brk.size());
	
	//get the distance calculation of the entire curve (for tangent scaling)

	//timer.reset();
	{
		sinfg::Point p1,p2;
		
		p1=p2=f[0];
		
		di.resize(f.size()); d_i.resize(f.size());
		Real d = 0;
		for(unsigned int i = 0; i < f.size();)
		{
			d += (d_i[i] = (p2-p1).mag());
			di[i] = d;
			
			p1=p2;
			p2=f[++i];
		}
	}
	//disteval = timer();
	//sinfg::info("calculated distance");
		
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
		
		Real errortol = smoothness*pixelwidth; //???? what the hell should this value be
		
		BLinePoint a;
		sinfg::Vector v;
		
		//intemp = f; //don't want to smooth out the corners
		
		bool breaktan = false, setwidth;
		a.set_split_tangent_flag(false);
		//a.set_width(width);
		a.set_width(1.0f);
		
		setwidth = (f.size() == f_w.size());
		
		for(j = 0; j < (int)brk.size() - 1; ++j)
		{
			//for b[j] to b[j+1] subdivide and stuff
			i0 = brk[j];
			i3 = brk[j+1];
			
			unsigned int size = i3-i0+1; //must include the end points
			
			//new derivatives
			//timer.reset();
			ftemp.assign(f.begin()+i0, f.begin()+i3+1);
			for(i=0;i<20;++i)
				gaussian_blur_3(ftemp.begin(),ftemp.end(),false);
			
			df.resize(size);
			GetFirstDerivatives(ftemp,0,size,(char*)&df[0],sizeof(df[0]));
			//GetSimpleDerivatives(ftemp,0,size,df,0,di); 
			//< don't have to worry about indexing stuff as it is all being taken car of right now
			//preproceval += timer();
			//numpre++;
			
			work.resize(size*2-1); //guarantee that all points will be tesselated correctly (one point inbetween every 2 adjacent points)
			
			//if size of work is size*2-1, the step size should be 1/(size*2 - 2)
			//Real step = 1/(Real)(size*2 - 1);
			
			//start off with break points as indices
			curind.clear();
			curind.push_back(cpindex(i0,di[i3]-di[i0],0)); //0 error because no curve on the left
			curind.push_back(cpindex(i3,di[i3]-di[i0],-1)); //error needs to be reevaluated
			done = false; //we want to loop
			
			unsigned int dcount = 0;
			
			//while there are still enough points between us, and the error is too high subdivide (and invalidate neighbors that share tangents)		
			while(!done)
			{					
				//tesselate all curves with invalid error values
				work[0] = f[i0];
				
				//timer.reset();
				/*numtess += */tesselate_curves(curind,f,df,work);
				//tesseval += timer();
				
				//now get all error values
				//timer.reset();
				for(i = 1; i < (int)curind.size(); ++i)
				{
					if(curind[i].error < 0) //must have been retesselated, so now recalculate error value
					{
						//evaluate error from points (starting at current index)
						int size = curind[i].curind - curind[i-1].curind + 1;
						curind[i].error = CurveError(&f[curind[i-1].curind], size,
													 work,(curind[i-1].curind - i0)*2,(curind[i].curind - i0)*2+1);
						
						/*if(curind[i].error > 1.0e5)
						{
							sinfg::info("Holy crap %d-%d error %f",curind[i-1].curind,curind[i].curind,curind[i].error);
							curind[i].error = -1;
							numtess += tesselate_curves(curind,f,df,work);
							curind[i].error = CurveError(&f[curind[i-1].curind], size,
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
					
					assert(ibreak < f.size());
					
					//sinfg::info("Split %d -%d- %d, error: %f", ibase,ibreak,itop,maxrelerror);
					
					if(ibase != itop)
					{
						//invalidate current error of the changed tangents and add an extra segment
						//enforce minimum tangents property
						curind[maxi].error = -1;
						curind[maxi-1].error = -1;
						if(maxi+1 < indsize) curind[maxi+1].error = -1; //if there is a curve segment beyond this it will be effected as well
						
						scale = di[itop] - di[ibreak];
						scale2 = maxi+1 < indsize ? di[curind[maxi+1].curind] - di[itop] : scale; //to the right valid?
						curind[maxi].tangentscale = std::min(scale, scale2);
												
						scale = di[ibreak] - di[ibase];
						scale2 = maxi >= 2 ? di[ibase] - di[curind[maxi-2].curind] : scale; // to the left valid -2 ?
						curind[maxi-1].tangentscale = std::min(scale, scale2);
						
						scale = std::min(di[ibreak] - di[ibase], di[itop] - di[ibreak]);
						
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
			v = df[is - i0];
			if(v.mag_squared() > EPSILON)
				v *= (curind[0].tangentscale/v.mag());
							
			if(!breaktan)
				a.set_tangent(v);
			else a.set_tangent2(v);
			
			a.set_vertex(f[is]);
			if(setwidth)a.set_width(f_w[is]);
			
			out.push_back(a);
			a.set_split_tangent_flag(false); //won't need to break anymore
			breaktan = false;
			
			for(i = 1; i < (int)curind.size()-1; ++i)
			{
				is = curind[i].curind;
				
				//first point inherits current tangent status
				v = df[is-i0];
				if(v.mag_squared() > EPSILON)
					v *= (curind[i].tangentscale/v.mag());
								
				a.set_tangent(v); // always inside, so guaranteed to be smooth
				a.set_vertex(f[is]);
				if(setwidth)a.set_width(f_w[is]);
				
				out.push_back(a);
			}
			
			//set the last point's data
			is = curind.back().curind; //should already be this
			
			v = df[is-i0];
			if(v.mag_squared() > EPSILON)
				v *= (curind.back().tangentscale/v.mag());
			
			a.set_tangent1(v);
			a.set_split_tangent_flag(true);
			breaktan = true;
			
			//will get the vertex and tangent 2 from next round
		}
		
		a.set_vertex(f[i3]);
		a.set_split_tangent_flag(false);
		if(setwidth)
			a.set_width(f_w[i3]);
		out.push_back(a);
		
		/*etl::clock::value_type totaltime = total(),
							   misctime = totaltime - initialprocess - curveval - breakeval - disteval
									  - preproceval - tesseval - erroreval - spliteval;
		
		sinfg::info(
			"Curve Convert Profile:\n"
			"\tInitial Preprocess:    %f\n"
			"\tCurvature Calculation: %f\n"
			"\tBreak Calculation:     %f\n"
			"\tDistance Calculation:  %f\n"
			"  Algorithm: (numtimes,totaltime)\n"
			"\tPreprocess step:      (%d,%f)\n"
			"\tTesselation step:     (%d,%f)\n"
			"\tError step:           (%d,%f)\n"
			"\tSplit step:           (%d,%f)\n"
			"  Num Input: %d, Num Output: %d\n"
			"  Total time: %f, Misc time: %f\n",
			initialprocess, curveval,breakeval,disteval,
			numpre,preproceval,numtess,tesseval,numerror,erroreval,numsplit,spliteval,
			in.size(),out.size(),
			totaltime,misctime);*/
		
		return;
	}
}

void sinfgapp::BLineConverter::EnforceMinWidth(std::list<sinfg::BLinePoint> &bline, sinfg::Real min_pressure)
{
	std::list<sinfg::BLinePoint>::iterator	i = bline.begin(),
											end = bline.end();
	
	for(i = bline.begin(); i != end; ++i)
	{
		if(i->get_width() < min_pressure)
		{
			i->set_width(min_pressure);
		}
	}
}
