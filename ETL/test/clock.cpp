/*! ========================================================================
** Extended Template and Library Test Suite
** Clock Test
**
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/clock>
#include <stdio.h>
#include <chrono>
#include <thread>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

int basic_test(void)
{
	int ret=0;

	etl::clock timer;
	float amount, total;

	for(amount=3.0;amount>=0.00015;amount/=2.0)
	{
		if(amount*1000000.0<1000.0f)
			fprintf(stderr,"waiting %f microseconds...\n",amount*1000000.0);
		else if(amount*1000.0<400.0f)
			fprintf(stderr,"waiting %f milliseconds...\n",amount*1000.0);
		else
			fprintf(stderr,"waiting %f seconds...\n",amount);

		timer.reset();
		std::this_thread::sleep_for(std::chrono::microseconds ((std::int64_t)(amount*1000000.f)));
		total=timer();
		if((total-amount)*1000000.0<1000.0f)
			fprintf(stderr," ** I waited %f seconds, error of %f microseconds\n",total,(total-amount)*1000000);
		else if((total-amount)*1000.0<400.0f)
			fprintf(stderr," ** I waited %f seconds, error of %f milliseconds\n",total,(total-amount)*1000);
		else
			fprintf(stderr," ** I waited %f seconds, error of %f seconds\n",total,total-amount);

	}
	return ret;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=basic_test();

	return error;
}

