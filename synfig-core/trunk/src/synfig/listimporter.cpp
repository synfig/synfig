/* === S Y N F I G ========================================================= */
/*!	\file listimporter.cpp
**	\brief Template File
**
**	$Id: listimporter.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#include "listimporter.h"
#include "general.h"
#include <fstream>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define LIST_IMPORTER_CACHE_SIZE	20

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ListImporter::ListImporter(const String &filename)
{
	fps=15;

	ifstream stream(filename.c_str());

	if(!stream)
	{
		synfig::error("Unable to open "+filename);
		return;
	}
	String line;
	String prefix=etl::dirname(filename)+ETL_DIRECTORY_SEPERATOR;
	while(!stream.eof())
	{
		getline(stream,line);
		if(line.empty())
			continue;
		// If we have a framerate, then use it
		if(line.find(String("FPS "))==0)
		{
			fps=atof(String(line.begin()+4,line.end()).c_str());
			//synfig::warning("FPS=%f",fps);
			if(!fps)
				fps=15;
			continue;
		}
		filename_list.push_back(prefix+line);
	}
}

Importer*
ListImporter::create(const char *filename)
{
	return new ListImporter(filename);
}

ListImporter::~ListImporter()
{
}

bool
ListImporter::get_frame(Surface &surface,Time time, ProgressCallback *cb)
{
//			DEBUGPOINT();
	int frame=static_cast<int>(time*fps);
//			DEBUGPOINT();
	
	if(!filename_list.size())
	{
		if(cb)cb->error(_("No images in list"));
		else synfig::error(_("No images in list"));
		return false;
	}
	
//			DEBUGPOINT();
	if(frame<0)frame=0;
	if(frame>=(signed)filename_list.size())frame=filename_list.size()-1;
	
//			DEBUGPOINT();
	// See if that frame is cached
	std::list<std::pair<int,Surface> >::iterator iter;
	for(iter=frame_cache.begin();iter!=frame_cache.end();++iter)
	{
		if(iter->first==frame)
		{
//			DEBUGPOINT();
			surface.mirror(iter->second);
			return static_cast<bool>(surface);
		}
	}
		
	Importer::Handle importer(Importer::open(filename_list[frame]));
	
//	DEBUGPOINT();

	if(!importer)
	{
		if(cb)cb->error(_("Unable to open ")+filename_list[frame]);
		else synfig::error(_("Unable to open ")+filename_list[frame]);
		return false;
	}
	
//	DEBUGPOINT();

	if(!importer->get_frame(surface,0,cb))
	{
		if(cb)cb->error(_("Unable to get frame from ")+filename_list[frame]);
		else synfig::error(_("Unable to get frame from ")+filename_list[frame]);
		return false;
	}
		
//	DEBUGPOINT();

	if(frame_cache.size()>=LIST_IMPORTER_CACHE_SIZE)
		frame_cache.pop_front();

//	DEBUGPOINT();

	frame_cache.push_back(std::pair<int,Surface>(frame,surface));	

//	DEBUGPOINT();

	surface.mirror(frame_cache.back().second);

//	DEBUGPOINT();

	return static_cast<bool>(surface);
}

bool
ListImporter::is_animated()
{
	return true;
}
