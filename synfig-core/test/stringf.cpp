/*! ========================================================================
** Extended Template and Library Test Suite
** stringf Procedure Test
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <iostream>
#include <ETL/stringf>
#include <stdio.h>

/* === M A C R O S ========================================================= */

using namespace etl;
using namespace std;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

int basic_test(void)
{
	int ret=0;
	char mystring[80]="My formatted string!";
	string myotherstring="my other string!";

	cout<<strprintf("This is a test of >>%s<<.",mystring)<<endl;

	myotherstring="5 6.75 George 7";
	int i,i2;
	float f;

#ifndef ETL_NO_STRSCANF
	strscanf(myotherstring,"%d %f %s %d",&i, &f, mystring, &i2);
#else
	cout<<"warning: strscanf() disabled at compile time..."<<endl;
	i=5;f=6.75;i2=7;
#endif

	cout<<myotherstring+"=="+strprintf("%d %f %s %d",i, f, mystring, i2)<<endl;

	cout<<stratof(strprintf("32.5849"))<<"==32.5849"<<endl;
	return ret;
}

int base_and_dir_name_test(void)
{
	int ret=0;

	string str(unix_to_local_path("/usr/bin/bleh.exe"));
	cout<<"Test Case 1 -> "<<str<<endl;
	cout<<"basename -> "<<basename(str)<<endl;
	if(basename(str)!="bleh.exe")
		cerr<<"error:Bad basename"<<endl,ret++;
	cout<<"dirname -> "<<dirname(str)<<endl;
	if(dirname(str)!=unix_to_local_path("/usr/bin"))
		cerr<<"error:Bad dirname"<<endl,ret++;
	cout<<endl;

	str=unix_to_local_path("/usr/bin/");
	cout<<"Test Case 2 -> "<<str<<endl;
	cout<<"basename -> "<<basename(str)<<endl;
	if(basename(str)!="bin")
		cerr<<"error:Bad basename"<<endl,ret++;
	cout<<"dirname -> "<<dirname(str)<<endl;
	if(dirname(str)!=unix_to_local_path("/usr"))
		cerr<<"error:Bad dirname"<<endl,ret++;
	cout<<endl;

	str="bleh.exe";
	cout<<"Test Case 3 -> "<<str<<endl;
	cout<<"basename -> "<<basename(str)<<endl;
	if(basename(str)!="bleh.exe")
		cerr<<"error:Bad basename"<<endl,ret++;
	cout<<"dirname -> "<<dirname(str)<<endl;
	if(dirname(str)!=unix_to_local_path("."))
		cerr<<"error:Bad dirname"<<endl,ret++;
	cout<<endl;

	return ret;
}

int relative_path_test()
{
	int ret=0;

	string curr_path=unix_to_local_path("/usr/local/bin/.");
	string dest_path=unix_to_local_path("/usr/share");

	cout<<"curr_path="<<curr_path<<" dest_path="<<dest_path<<endl;
	cout<<"relative_path="<<relative_path(curr_path,dest_path)<<endl;
	if(relative_path(curr_path,dest_path)!=unix_to_local_path("../../share"))
		cerr<<"Bad relative path"<<endl,ret++;

	cout<<endl;

	curr_path=unix_to_local_path("/home/darco/projects/voria");
	dest_path=unix_to_local_path("/home/darco/projects/voria/myfile.txt");
	cout<<"curr_path="<<curr_path<<" dest_path="<<dest_path<<endl;
	cout<<"relative_path="<<relative_path(curr_path,dest_path)<<endl;
	if(relative_path(curr_path,dest_path)!=unix_to_local_path("myfile.txt"))
		cerr<<"Bad relative path"<<endl,ret++;

	cout<<endl;

	curr_path=unix_to_local_path("/home/darco/projects/voria");
	dest_path=unix_to_local_path("/home/darco/projects/voria/files/myfile.txt");
	cout<<"curr_path="<<curr_path<<" dest_path="<<dest_path<<endl;
	cout<<"relative_path="<<relative_path(curr_path,dest_path)<<endl;
	if(relative_path(curr_path,dest_path)!=unix_to_local_path("files/myfile.txt"))
		cerr<<"Bad relative path"<<endl,ret++;

	cout<<endl;

	curr_path=unix_to_local_path("/usr/local/../include/sys/../linux/linux.h");
	cout<<"dirty_path="<<curr_path<<endl;
	cout<<"clean_path="<<cleanup_path(curr_path)<<endl;

	cout<<"current_working_directory="<<current_working_directory()<<endl;
	return ret;
}


/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=basic_test();
	error+=base_and_dir_name_test();
	error+=relative_path_test();

	return error;
}
