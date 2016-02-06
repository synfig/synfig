/* === S Y N F I G ========================================================= */
/*!	\file ipc.cpp
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

#include "ipc.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef __OpenBSD__
#include <errno.h>
#elif defined(HAVE_SYS_ERRNO_H)
#include <sys/errno.h>
#endif

#include <synfig/main.h>
#include "app.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "docks/dock_toolbox.h"
#include <glibmm/dispatcher.h>
#include <synfig/mutex.h>
#include <synfig/string.h>
#include <synfigapp/main.h>
#include <glibmm/thread.h>
#include <glibmm/miscutils.h>

#ifdef _WIN32
#include <windows.h>
#define BUFSIZE   128
#define read	_read
#endif

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

#ifdef _WIN32
#define WIN32_PIPE_PATH "\\\\.\\pipe\\SynfigStudio.Cmd"
static synfig::Mutex cmd_mutex;
static std::list<synfig::String> cmd_queue;
static Glib::Dispatcher* cmd_dispatcher;
static void
pipe_listen_thread()
{
	for(;;)
	{
		HANDLE pipe_handle;
		pipe_handle=CreateNamedPipe(
			WIN32_PIPE_PATH, // pipe name
			PIPE_ACCESS_INBOUND, // Access type
			PIPE_READMODE_BYTE /*|PIPE_NOWAIT*/,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL
		);
		if(pipe_handle==INVALID_HANDLE_VALUE)
		{
			synfig::error("IPC(): Call to CreateNamedPipe failed. Ignore next error. GetLastError=%d",GetLastError());
			return;
		}

		bool connected;
		connected=ConnectNamedPipe(pipe_handle,NULL)?true:(GetLastError()==ERROR_PIPE_CONNECTED);
		DWORD read_bytes;
		bool success;

		Glib::Thread::yield();

		if(connected)
		do {
			String data;
			char c;
			do
			{
				success= ReadFile(
					pipe_handle,
					&c,		// buffer pointer
					1,		// buffer size
					&read_bytes,
					NULL
				);
				if(success && read_bytes==1 && c!='\n')
					data+=c;
			}while(c!='\n');
			synfig::Mutex::Lock lock(cmd_mutex);
			cmd_queue.push_back(data);
			cmd_dispatcher->emit();
		} while(success && read_bytes);

		CloseHandle(pipe_handle);
	}
}

static void
empty_cmd_queue()
{
	synfig::Mutex::Lock lock(cmd_mutex);
	while(!cmd_queue.empty())
	{
		IPC::process_command(cmd_queue.front());
		cmd_queue.pop_front();
	}
}

#endif

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

IPC::IPC()
{
#ifdef _WIN32

	cmd_dispatcher=new Glib::Dispatcher;
	cmd_dispatcher->connect(sigc::ptr_fun(empty_cmd_queue));

	Glib::Thread::create(
		sigc::ptr_fun(pipe_listen_thread),
		false
	);

#else

	remove(fifo_path().c_str());
	fd=-1;

	if(mkfifo(fifo_path().c_str(), S_IRWXU)!=0)
	{
		synfig::error("IPC(): mkfifo failed for "+fifo_path());
	}

	{
		fd=open(fifo_path().c_str(),O_RDWR);


		if(fd<0)
		{
			synfig::error("IPC(): Failed to open fifo \"%s\". (errno=?)",fifo_path().c_str());
			//synfig::error("IPC(): Failed to open fifo \"%s\". (errno=%d)",fifo_path().c_str(),::errno);
		}
		else
		{
			file=SmartFILE(fdopen(fd,"r"));

			Glib::signal_io().connect(
				sigc::mem_fun(this,&IPC::fifo_activity),
				fd,
				Glib::IO_IN|Glib::IO_PRI|Glib::IO_ERR|Glib::IO_HUP|Glib::IO_NVAL
			);
		}
	}
#endif
}

IPC::~IPC()
{
	//if(file)
	//	fclose(file.get());

	remove(fifo_path().c_str());

	//if(fd>=0)
	//	close(fd);
}

synfig::String
IPC::fifo_path()
{
#ifdef _WIN32
	return WIN32_PIPE_PATH;
#else
	return Glib::build_filename(synfigapp::Main::get_user_app_directory(),"fifo");
#endif
}

bool
IPC::fifo_activity(Glib::IOCondition cond)
{
	if(cond&(Glib::IO_ERR|Glib::IO_HUP|Glib::IO_NVAL))
	{
		if(cond&(Glib::IO_ERR))
			synfig::error("IPC::fifo_activity(): IO_ERR");
		if(cond&(Glib::IO_HUP))
			synfig::error("IPC::fifo_activity(): IO_HUP");
		if(cond&(Glib::IO_NVAL))
			synfig::error("IPC::fifo_activity(): IO_NVAL");
		return false;
	}

	String command;
	{
		char tmp;
		do {
			if(read(fd,&tmp,sizeof(tmp))<=0)
				break;
			if(tmp!='\n')
				command+=tmp;
		} while(tmp!='\n');
	}

	synfig::info("%s:%d: fifo activity: '%s'", __FILE__, __LINE__, command.c_str());
	process_command(command);
	return true;
}

bool
IPC::process_command(const synfig::String& command_line)
{
	if(command_line.empty())
		return false;

	char cmd = command_line[0];

	String args(command_line.begin()+1,command_line.end());

	// erase leading spaces
	while (!args.empty() && args[0] == ' ')
		args.erase(args.begin());

	// erase trailing newlines and spaces
	while (!args.empty() && (args[args.size()-1] == '\n' || args[args.size()-1] == ' '))
		args.erase(args.end()-1);

	switch(toupper(cmd))
	{
		case 'F': // Focus/Foreground
			App::signal_present_all()();
			break;
		case 'N': // New file
			App::signal_present_all()();
			App::new_instance();
			break;
		case 'O': // Open <arg>
			App::signal_present_all()();
			App::open(args);
			break;
		case 'X': // Quit
		case 'Q': // Quit
			App::quit();
			break;
		default:
			synfig::warning("Received unknown command '%c' with arg '%s'",cmd,args.c_str());
			break;
	}

	return true;
}

synfig::SmartFILE
IPC::make_connection()
{
	SmartFILE ret;
#ifdef _WIN32
	HANDLE pipe_handle;
	pipe_handle=CreateFile(
		fifo_path().c_str(),
		GENERIC_WRITE, // desired access
		0, // share mode
		NULL, // security attributes
		OPEN_EXISTING, // creation disposition
		FILE_ATTRIBUTE_NORMAL, // flags and attributes
		NULL  // template file
	);
	int fd;
	if(pipe_handle==INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
#ifndef _DEBUG
		if( error != ERROR_FILE_NOT_FOUND )
#endif
			synfig::warning("IPC::make_connection(): Unable to connect to previous instance. GetLastError=%d",error);
		fd=-1;
	} else {
		fd=_open_osfhandle(reinterpret_cast<intptr_t>(pipe_handle),_O_APPEND|O_WRONLY);
	}
#else
	struct stat file_stat;
	if(stat(fifo_path().c_str(),&file_stat)!=0)
		return ret;

	if(!S_ISFIFO(file_stat.st_mode))
		return ret;

	int fd=open(fifo_path().c_str(),O_WRONLY|O_NONBLOCK);
#endif

	if(fd>=0)
		ret=SmartFILE(fdopen(fd,"w"));

#ifdef _DEBUG
	// synfig::info("uplink fd=%d",fd);
#endif

	return ret;
}
