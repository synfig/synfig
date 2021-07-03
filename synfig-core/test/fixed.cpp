/*! ========================================================================
** Extended Template and Library Test Suite
** Fixed-Point Math Test
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define ETL_FIXED_BITS		12

#include <ETL/fixed>
#include <stdio.h>
#include <ETL/clock>

/* === M A C R O S ========================================================= */

#ifndef PI
# define PI (3.1415926535897932384626433832795029L)
#endif

#define ADD_SUB_TEST	20000000
#define MUL_TEST 		10000000
#define DIV_TEST		1048573	// at 1048573, fixed point numbers wrap around to zero
using namespace etl;

/* === C L A S S E S ======================================================= */

template <class value_type>
struct speed_test
{
	double add_sub_test(void)
	{
		value_type a=1;
		value_type b=2;
		value_type c=3;
		int i;
		etl::clock MyTimer;
		MyTimer.reset();
		for(i=0;i<ADD_SUB_TEST;i++)
		{
			a+=a; a-=b; a-=c; a+=a;
			a+=a; a-=b; a-=c; a+=a;
			a+=a; a-=b; a-=c; a+=a;
			a+=a; a-=b; a-=c; a+=a;
			a+=a; a-=b; a-=c; a+=a;
			a+=a; a-=b; a-=c; a+=a;
			a+=a; a-=b; a-=c; a+=a;
		}

		fprintf(stderr, "[%1d]...", int(int(a)/1e9)+5); // so the compiler doesn't optimize everything out
		return MyTimer();
	}

	double mul_test(void)
	{
		value_type a,b,c,d;
		a=value_type(2.25);
		b=value_type(2);
		c=value_type(4.5);
		d=value_type(1);
		const value_type one_and_a_half(static_cast<value_type>(1.5));
		int i;
		etl::clock MyTimer;
		MyTimer.reset();
		for(i=1;i<MUL_TEST;i++)
		{
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
			d*=a;d*=b;d*=c;d*=3;d*=i; b*=c;b*=d;b*=d;b*=3; c*=d;c*=one_and_a_half;c*=a;c*=b;c*=3; a*=c;a*=b;a*=3;
		}

		fprintf(stderr, "[%1d]...", int(int(a)/1e9)+5); // so the compiler doesn't optimize everything out
		return MyTimer();
	}

	double div_test(void)
	{
		value_type a(30);
		value_type b(40);
		value_type acc(0);
		int i, j;
		etl::clock MyTimer;
		MyTimer.reset();
		for(j=0;j<10;j++)
			for(i=1;i<DIV_TEST;i++)
			{
				a=3+i; b=40+i; b/=a; a/=(i%20)+1; acc+= a;
				a=3+i; b=40+i; b/=a; a/=(i%20)+1; acc+= a;
				a=3+i; b=40+i; b/=a; a/=(i%20)+1; acc+= a;
				a=3+i; b=40+i; b/=a; a/=(i%20)+1; acc+= a;
				a=3+i; b=40+i; b/=a; a/=(i%20)+1; acc+= a;
				a=3+i; b=40+i; b/=a; a/=(i%20)+1; acc+= a;
			}

		fprintf(stderr, "[%1d]...", int(int(acc)/1e9)+5); // so the compiler doesn't optimize everything out
		return MyTimer();
	}
};

/* === P R O C E D U R E S ================================================= */

int basic_test(void)
{
	int ret=0;

	fixed a,b,c;
	double d;

	a=-1;
	a=std::abs(a);
	if(a!=fixed(1))
	{
		fprintf(stderr, "fixed: abs() failure on line %d in " __FILE__ ".\n", __LINE__);
		ret++;
	}

	d=(double)(fixed(2.5)*fixed(3.0f)/7)-(2.5f*3.0f/7.0f);
	fprintf(stderr,"fixed: 2.5 * 2 / 7 --- Difference: %f\n",d);
	if(d<0.0)d=-d;
	if( d>0.0005)
	{
		fprintf(stderr, "fixed: Failed test on line %d in " __FILE__ ".\n", __LINE__);
		ret++;
	}

	a=1043;d=1043;
	a/=27;d/=27;
	a+=10.42;d+=10.42;
	a/=6;d/=6;
	a*=PI;d*=PI;
	d-=(double)a;
	fprintf(stderr,"fixed: ( 1043 / 27 + 10.42 ) / 6 * PI --- Difference: %f\n",d);
	if(d<0.0)d=-d;
#ifdef ROUND_TO_NEAREST_INTEGER
	if( d>0.0005)
#else
	if( d>0.0025)
#endif
	{
		fprintf(stderr,"fixed: Failed test on line %d in " __FILE__ ".\n", __LINE__);
		ret++;
	}

	return ret;
}

int char_test(void)
{
	int ret=0;

	fixed_base<unsigned char,8> fix;
	double flt;

	if(sizeof(fix)!=sizeof(unsigned char))
	{
		ret++;
		fprintf(stderr,"fixed: Size of fixed_base<unsigned char,8> is wrong!\n");
	}

	flt=1.0;
	fix=1.0;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt*=0.7;
	fix*=0.7;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt*=0.7;
	fix*=0.7;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt*=0.7;
	fix*=0.7;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt*=0.7;
	fix*=0.7;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt*=0.7;
	fix*=0.7;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	//flt/=0.7;
	//fix/=0.7;
	//fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt+=0.3;
	fix+=0.3;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);

	flt*=2;
	fix*=2;
	fprintf(stderr,"fixed: value=%f, data=%d, shouldbe=%f, error=%f\n",(float)fix,fix.data(),flt,(float)fix-flt);


	return ret;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=basic_test();
	error+=char_test();

	speed_test<float> float_test;
	speed_test<int> int_test;
	speed_test<fixed> fixed_test;

	{
		double flt,fix,inte;
		fprintf(stderr,"\nAddition/subtraction test...\n");

		fprintf(stderr,"  calculating float.....");
		flt=float_test.add_sub_test();
		fprintf(stderr,"   float time: %f sec\n",flt);

		fprintf(stderr,"  calculating fixed.....");
		fix=fixed_test.add_sub_test();
		fprintf(stderr,"   fixed time: %f sec\n",fix);

		fprintf(stderr,"  calculating integer...");
		inte=int_test.add_sub_test();
		fprintf(stderr," integer time: %f sec\n",inte);

		if(flt>fix)
			fprintf(stderr,"Fixed point wins by %f seconds! (%f%% faster)\n",flt-fix,flt/fix*100.0f-100.0f);
		else
			fprintf(stderr,"Floating point wins by %f seconds! (%f%% faster)\n",fix-flt,fix/flt*100.0f-100.0f);

	}

	{
		double flt,fix,inte;
		fprintf(stderr,"\nProduct test...\n");
		fprintf(stderr,"  calculating float.....");
		flt=float_test.mul_test();
		fprintf(stderr,"   float time: %f sec\n",flt);
		fprintf(stderr,"  calculating fixed.....");
		fix=fixed_test.mul_test();
		fprintf(stderr,"   fixed time: %f sec\n",fix);
		fprintf(stderr,"  calculating integer...");
		inte=int_test.mul_test();
		fprintf(stderr," integer time: %f sec\n",inte);
		if(flt>fix)
			fprintf(stderr,"Fixed point wins by %f seconds! (%f%% faster)\n",flt-fix,flt/fix*100.0f-100.0f);
		else
			fprintf(stderr,"Floating point wins by %f seconds! (%f%% faster)\n",fix-flt,fix/flt*100.0f-100.0f);
	}

	{
		double flt,fix,inte;
		fprintf(stderr,"\nDivision test...\n");
		fprintf(stderr,"  calculating float.....");
		flt=float_test.div_test();
		fprintf(stderr,"   float time: %f sec\n",flt);
		fprintf(stderr,"  calculating fixed.....");
		fix=fixed_test.div_test();
		fprintf(stderr,"   fixed time: %f sec\n",fix);
		fprintf(stderr,"  calculating integer...");
		inte=int_test.div_test();
		fprintf(stderr," integer time: %f sec\n",inte);
		if(flt>fix)
			fprintf(stderr,"Fixed point wins by %f seconds! (%f%% faster)\n",flt-fix,flt/fix*100.0f-100.0f);
		else
			fprintf(stderr,"Floating point wins by %f seconds! (%f%% faster)\n",fix-flt,fix/flt*100.0f-100.0f);
		fprintf(stderr,"\n");
	}

	return error;
}
