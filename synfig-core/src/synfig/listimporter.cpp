/* === S Y N F I G ========================================================= */
/*!	\file listimporter.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "listimporter.h"
#include "general.h"
#include "filesystemnative.h"
#include <fstream>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define LIST_IMPORTER_CACHE_SIZE	20

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(ListImporter);
SYNFIG_IMPORTER_SET_NAME(ListImporter,"lst");
SYNFIG_IMPORTER_SET_EXT(ListImporter,"lst");
SYNFIG_IMPORTER_SET_VERSION(ListImporter,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(ListImporter,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(ListImporter, false);

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ListImporter::ListImporter(const FileSystem::Identifier &identifier):
Importer(identifier)
{
	fps=15;

	ifstream stream(identifier.filename.c_str());

	if(!stream)
	{
		synfig::error("Unable to open "+identifier.filename);
		return;
	}
	String line;
	String prefix=etl::dirname(identifier.filename)+ETL_DIRECTORY_SEPARATOR;
	getline(stream,line);		// read first line and check whether it is a Papagayo lip sync file

	if (line == "MohoSwitch1")	// it is a Papagayo lipsync file
	{
		String phoneme, prevphoneme, prevext, ext(".jpg"); // default image format
		int frame, prevframe = -1; // it means that the previous phoneme is not known

		while(!stream.eof())
		{
			getline(stream,line);

			if(line.find(String("FPS ")) == 0)
			{
				float f = atof(String(line.begin()+4,line.end()).c_str());
				if (f) fps = f;
				continue;
			}

			if (line == "bmp"  ||
				line == "gif"  ||
				line == "jpg"  ||
				line == "png"  ||
				line == "ppm"  ||
				line == "tiff" )
			{
				ext = String(".") + line;
				continue;
			}

			size_t pos = line.find(String(" ")); // find space position. The format is "frame phoneme-name".
			if(pos != String::npos)
			{
				frame = atoi(String(line.begin(),line.begin()+pos).c_str());
				phoneme = String(line.begin()+pos+1, line.end());

				if (prevframe != -1)
					while (prevframe < frame)
					{
						filename_list.push_back(prefix + prevphoneme + prevext);
						synfig::info("frame %d, phoneme = %s, path = '%s'", prevframe, prevphoneme.c_str(), (prefix + prevphoneme + prevext).c_str());
						prevframe++;
					}

				prevext = ext;
				prevframe = frame;
				prevphoneme = phoneme;
			}
		}

		filename_list.push_back(prefix + prevphoneme + prevext);	// do it one more time for the last phoneme
		synfig::info("finally, frame %d, phoneme = %s, path = '%s'", prevframe, prevphoneme.c_str(), (prefix + prevphoneme + prevext).c_str());

		return;
	}

	stream.seekg(ios_base::beg);
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


ListImporter::~ListImporter()
{
}

bool
ListImporter::get_frame(Surface &surface, const RendDesc &renddesc, Time time, ProgressCallback *cb)
{
	float document_fps=renddesc.get_frame_rate();
	int document_frame=round_to_int(time*document_fps);
	int frame=floor_to_int(document_frame*fps/document_fps);

	if(!filename_list.size())
	{
		if(cb)cb->error(_("No images in list"));
		else synfig::error(_("No images in list"));
		return false;
	}

	if(frame<0)frame=0;
	if(frame>=(signed)filename_list.size())frame=filename_list.size()-1;

	// See if that frame is cached
	std::list<std::pair<String,Surface> >::iterator iter;
	for(iter=frame_cache.begin();iter!=frame_cache.end();++iter)
	{
		if(iter->first==filename_list[frame])
		{
			surface.mirror(iter->second);
			return static_cast<bool>(surface);
		}
	}

	Importer::Handle importer(Importer::open(FileSystem::Identifier(FileSystemNative::instance(), filename_list[frame])));

	if(!importer)
	{
		if(cb)cb->error(_("Unable to open ")+filename_list[frame]);
		else synfig::error(_("Unable to open ")+filename_list[frame]);
		return false;
	}

	if(!importer->get_frame(surface,renddesc,0,cb))
	{
		if(cb)cb->error(_("Unable to get frame from ")+filename_list[frame]);
		else synfig::error(_("Unable to get frame from ")+filename_list[frame]);
		return false;
	}

	if(frame_cache.size()>=LIST_IMPORTER_CACHE_SIZE)
		frame_cache.pop_front();

	frame_cache.push_back(std::pair<String,Surface>(filename_list[frame],surface));

	surface.mirror(frame_cache.back().second);

	return static_cast<bool>(surface);
}

bool
ListImporter::is_animated()
{
	return true;
}
