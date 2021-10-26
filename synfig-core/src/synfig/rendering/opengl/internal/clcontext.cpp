/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/clcontext.cpp
**	\brief ClContext
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <cassert>

#include <vector>

#include <synfig/general.h>
#include <synfig/localization.h>

#include "clcontext.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::ClContext::ClContext():
	context(),
	device(),
	queue()
{

	// platform

	cl_uint platform_count = 0;
	clGetPlatformIDs(0, NULL, &platform_count);
	assert(platform_count);
	std::vector<cl_platform_id> platforms(platform_count);
	clGetPlatformIDs(platforms.size(), &platforms.front(), NULL);
	cl_platform_id platform = platforms.front();
	assert(platform);

	//char vendor[256] = { };
	//clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
    //char platform_version[256];
    //clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(platform_version), platform_version, NULL);

	// devices

	cl_uint device_count = 0;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &device_count);
    assert(device_count);
    std::vector<cl_device_id> devices(device_count);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, devices.size(), &devices.front(), NULL);
    device = devices.front();
    assert(device);

    //char device_name[256];
    //clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    //char device_version[256];
    //clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(device_version), device_version, NULL);

    // context

    cl_context_properties context_props[] = {
    	CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		CL_NONE };
    context = clCreateContext(context_props, 1, &device, callback, NULL, NULL);
    assert(context);

	// command queue

	queue = clCreateCommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, NULL);
	assert(queue);
}

gl::ClContext::~ClContext()
{
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}

void
gl::ClContext::callback(const char *, const void *, size_t, void *)
{
}

cl_program
gl::ClContext::load_program(const String &source)
{
	cl_int err = 0;

	const char *source_pointer = source.c_str();
	cl_program program = clCreateProgramWithSource(context, 1, &source_pointer, NULL, NULL);
	assert(program);

	err = clBuildProgram(program, 1, &device, "", NULL, NULL);
	if (err) {
		size_t size;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
		String log(size, ' ');
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, &log[0], NULL);
		warning(log);
	}
	assert(!err);

	return program;
}

/* === E N T R Y P O I N T ================================================= */
