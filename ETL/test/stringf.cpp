/*! ========================================================================
** Extended Template and Library Test Suite
** stringf Procedure Test
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <iostream>
#include <ETL/stringf>
#include <stdio.h>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */


/* === P R O C E D U R E S ================================================= */

int base_and_dir_name_test(void)
{
	int ret=0;

	std::string str("/usr/bin/bleh.exe");
	std::cout<<"Test Case 1 -> "<<str<<std::endl;
	std::cout<<"basename -> "<<basename(str)<<std::endl;
	if(basename(str)!="bleh.exe")
		std::cerr<<"error:Bad basename"<<std::endl,ret++;
	std::cout<<"dirname -> "<<dirname(str)<<std::endl;
	if(dirname(str)!="/usr/bin")
		std::cerr<<"error:Bad dirname"<<std::endl,ret++;
	std::cout<<std::endl;

	str="/usr/bin/";
	std::cout<<"Test Case 2 -> "<<str<<std::endl;
	std::cout<<"basename -> "<<basename(str)<<std::endl;
	if(basename(str)!="bin")
		std::cerr<<"error:Bad basename"<<std::endl,ret++;
	std::cout<<"dirname -> "<<dirname(str)<<std::endl;
	if(dirname(str)!="/usr")
		std::cerr<<"error:Bad dirname"<<std::endl,ret++;
	std::cout<<std::endl;

	str="bleh.exe";
	std::cout<<"Test Case 3 -> "<<str<<std::endl;
	std::cout<<"basename -> "<<basename(str)<<std::endl;
	if(basename(str)!="bleh.exe")
		std::cerr<<"error:Bad basename"<<std::endl,ret++;
	std::cout<<"dirname -> "<<dirname(str)<<std::endl;
	if(dirname(str)!=".")
		std::cerr<<"error:Bad dirname"<<std::endl,ret++;
	std::cout<<std::endl;

	return ret;
}

int relative_path_test()
{
	int ret=0;

	std::string curr_path="/usr/local/bin/.";
	std::string dest_path="/usr/share";

	std::cout<<"curr_path="<<curr_path<<" dest_path="<<dest_path<<std::endl;
	std::cout<<"relative_path="<<relative_path(curr_path,dest_path)<<std::endl;
	if(relative_path(curr_path,dest_path)!="../../share")
		std::cerr<<"Bad relative path"<<std::endl,ret++;

	std::cout<<std::endl;

	curr_path="/home/darco/projects/voria";
	dest_path="/home/darco/projects/voria/myfile.txt";
	std::cout<<"curr_path="<<curr_path<<" dest_path="<<dest_path<<std::endl;
	std::cout<<"relative_path="<<relative_path(curr_path,dest_path)<<std::endl;
	if(relative_path(curr_path,dest_path)!="myfile.txt")
		std::cerr<<"Bad relative path"<<std::endl,ret++;

	std::cout<<std::endl;

	curr_path="/home/darco/projects/voria";
	dest_path="/home/darco/projects/voria/files/myfile.txt";
	std::cout<<"curr_path="<<curr_path<<" dest_path="<<dest_path<<std::endl;
	std::cout<<"relative_path="<<relative_path(curr_path,dest_path)<<std::endl;
	if(relative_path(curr_path,dest_path)!="files/myfile.txt")
		std::cerr<<"Bad relative path"<<std::endl,ret++;

	std::cout<<std::endl;

	curr_path="/usr/local/../include/sys/../linux/linux.h";
	std::cout<<"dirty_path="<<curr_path<<std::endl;
	std::cout<<"clean_path="<<cleanup_path(curr_path)<<std::endl;

	std::cout<<"current_working_directory="<<current_working_directory()<<std::endl;
	return ret;
}


/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=base_and_dir_name_test();
	error+=relative_path_test();

	return error;
}
