/* === S Y N F I G ========================================================= */
/*!	\file listimporter.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "listimporter.h"

#include "general.h"
#include <synfig/localization.h>

#include "filesystemnative.h"
#include <synfig/rendering/software/surfacesw.h>


#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

#define LIST_IMPORTER_CACHE_SIZE	20

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(ListImporter);
SYNFIG_IMPORTER_SET_NAME(ListImporter,"lst");
SYNFIG_IMPORTER_SET_EXT(ListImporter,"lst");
SYNFIG_IMPORTER_SET_VERSION(ListImporter,"0.1");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(ListImporter, false);

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ListImporter::ListImporter(const FileSystem::Identifier &identifier):
Importer(identifier)
{
	fps=15;

	FileSystem::ReadStream::Handle stream = identifier.get_read_stream();

	// TODO(ice0): There is no way to report about error to GUI. We can't
	//  throw error from constructor (bad practice), and can't return error code.
	if(!stream)	{
		synfig::error("Unable to open " + identifier.filename.u8string());
		return;
	}

	String line;
	String prefix = identifier.filename.parent_path().u8string() + ETL_DIRECTORY_SEPARATOR;

	///! read first line and check whether it is a Papagayo lip sync file
	if(!FileSystem::safe_get_line(*stream, line).eof())
	if (line == "MohoSwitch1")
	{
		//! it is a Papagayo lipsync file
		String phoneme, prevphoneme, prevext, ext(".jpg"); // default image format
		int frame, prevframe = -1; // it means that the previous phoneme is not known

		while(!FileSystem::safe_get_line(*stream, line).eof())
		{
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
				line == "svg"  ||
				line == "tiff" )
			{
				ext = String(".") + line;
				continue;
			}
			//! find space position. The format is "frame phoneme-name".
			size_t pos = line.find(String(" "));
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

	stream->seekg(std::ios_base::beg);
	while(!FileSystem::safe_get_line(*stream, line).eof())
	{
		if(line.empty())
			continue;
		//! If we have a framerate, then use it
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


ListImporter::~ListImporter() = default;

Importer::Handle
ListImporter::get_sub_importer(const RendDesc &renddesc, Time time, ProgressCallback *cb)
{
	float document_fps=renddesc.get_frame_rate();
	int document_frame=round_to_int(time*document_fps);
	int frame = std::floor(document_frame*fps/document_fps);

	if(!filename_list.size())
	{
		if (cb) cb->error(_("No images in list"));
		else synfig::error(_("No images in list"));
		return Importer::Handle();
	}

	if(frame<0)frame=0;
	if(frame>=(signed)filename_list.size())frame=filename_list.size()-1;

	const String &filename = filename_list[frame];
	Importer::Handle importer(Importer::open(FileSystem::Identifier(FileSystemNative::instance(), filename)));
	if(!importer)
	{
		if(cb)cb->error(_("Unable to open ")+filename);
		else synfig::error(_("Unable to open ")+filename);
		return Importer::Handle();
	}

	for(std::list<Importer::Handle>::iterator i = frame_cache.begin(); i != frame_cache.end();)
		if (*i == importer) i = frame_cache.erase(i); else ++i;

	while (frame_cache.size() >= LIST_IMPORTER_CACHE_SIZE)
		frame_cache.pop_front();
	frame_cache.push_back(importer);

	return importer;
}

bool
ListImporter::get_frame(Surface &surface, const RendDesc &renddesc, Time time, ProgressCallback *cb)
{
	Importer::Handle importer = get_sub_importer(renddesc, time, cb);
	return importer && importer->get_frame(surface, renddesc, 0, cb);
}

rendering::Surface::Handle
ListImporter::get_frame(const RendDesc &renddesc, const Time &time)
{
	Importer::Handle importer = get_sub_importer(renddesc, time, nullptr);
	return importer ? importer->get_frame(renddesc, 0) : new rendering::SurfaceSW();
}

bool
ListImporter::is_animated()
{
	return true;
}
