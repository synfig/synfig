/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animatedfile.cpp
**	\brief Implementation of the "AnimatedFile" valuenode conversion.
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#include <climits>

#include <istream>
#include <map>

#include <giomm.h>

#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/general.h>
#include <synfig/canvasfilenaming.h>

#include "valuenode_animatedfile.h"
#include "valuenode_const.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_AnimatedFile, RELEASE_VERSION_1_0_2, "animated_file", "Animation from File")

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

class ValueNode_AnimatedFile::Internal
{
public:
	Glib::RefPtr<Gio::FileMonitor> file_monitor;
};

class ValueNode_AnimatedFile::Parser
{
public:
	static bool parse_pgo(istream &s, map<Time, String> &value, map<String, String> &fields)
	{
		String word;
		bool unexpected_end = false;

		String id, version;
		s >> id >> word >>  version;
		if (id != "lipsync" || word != "version")
			{ error("Wrong format of .pgo file"); return false; }
		if (version != "1")
			warning("Unknown version of .pgo file: '%s'", version.c_str());
		getline(s, word);
		getline(s, word);
		fields["sound"] = word;

		int first_frame = INT_MAX;
		Real fps;
		s >> fps;
		if (fps <= 1e-10)
			return false;
		Real fk = 1.0/fps;
		getline(s, word);
		getline(s, word);

		int voices = 0;
		s >> voices;
		getline(s, word);
		for(int i = 0; i < voices; ++i)
		{
			getline(s, word);
			getline(s, word);

			int phrases = 0;
			s >> phrases;
			getline(s, word);
			for(int j = 0; j < phrases; ++j)
			{
				getline(s, word);
				getline(s, word);
				getline(s, word);

				int words = 0;
				s >> words;
				getline(s, word);
				for(int k = 0; k < words; ++k)
				{
					int end_frame = 0;
					int phonemes = 0;
					s >> word >> word >> end_frame >> phonemes;
					getline(s, word);
					for(int l = 0; l < phonemes; ++l)
					{
						int frame = 0;
						String phoneme;
						if (s)
						{
							s >> frame >> phoneme;
							first_frame = min(frame, first_frame);
							getline(s, word);
							value[Time(frame*fk)] = phoneme;
						} else unexpected_end = true;
					}
					value[Time(end_frame*fk)] = "rest";
				}
			}
		}
		if (first_frame == INT_MAX) first_frame= 1;
		value[Time((first_frame-1)*fk)] = "rest";

		if (unexpected_end)
			warning("Unexpected end of .pgo file. Unsupported format?");

		return true;
	}
};

ValueNode_AnimatedFile::ValueNode_AnimatedFile(Type &t):
	ValueNode_AnimatedInterfaceConst(*(ValueNode*)this),
	internal(new Internal()),
	filename(ValueNode_Const::create(String("")))
{
	ValueNode_AnimatedInterfaceConst::set_interpolation(INTERPOLATION_CONSTANT);
	ValueNode_AnimatedInterfaceConst::set_type(t);
	set_children_vocab(get_children_vocab());
	set_link("filename", ValueNode_Const::create(String()));
}

ValueNode_AnimatedFile::~ValueNode_AnimatedFile()
{
	delete internal;
}

bool
ValueNode_AnimatedFile::check_type(Type & /* type */)
	{ return true; }

LinkableValueNode*
ValueNode_AnimatedFile::create_new() const
	{ return new ValueNode_AnimatedFile(get_type()); }

ValueNode_AnimatedFile*
ValueNode_AnimatedFile::create(const ValueBase &x)
	{ return new ValueNode_AnimatedFile(x.get_type()); }



void
ValueNode_AnimatedFile::file_changed()
{
	load_file(current_filename, true);
	changed();
}

void
ValueNode_AnimatedFile::load_file(const String &filename, bool forse)
{
	if ( !get_parent_canvas()
	  || !get_parent_canvas()->get_file_system() ) return;

	String optimized_filename = filename;

	// Get rid of any %20 crap
	for(String::size_type n; (n = optimized_filename.find("%20")) != String::npos;)
		optimized_filename.replace(n,3," ");

	String full_filename = CanvasFileNaming::make_full_filename(get_parent_canvas()->get_file_name(), optimized_filename);
	String local_filename = CanvasFileNaming::make_local_filename(get_parent_canvas()->get_file_name(), full_filename);
	String independent_filename = CanvasFileNaming::make_canvas_independent_filename(get_parent_canvas()->get_file_name(), full_filename);

	if (current_filename == independent_filename && !forse) return;
	current_filename = independent_filename;

	internal->file_monitor.clear();
	filefields.clear();
	erase_all();

	if (!full_filename.empty())
	{
		// Read papagayo file
		if ( CanvasFileNaming::filename_extension_lower(full_filename) == "pgo"
		  && get_type() == type_string )
		{
			FileSystem::ReadStream::Handle rs = get_parent_canvas()->get_file_system()->get_read_stream(full_filename);
			if (!rs)
				FileSystem::ReadStream::Handle rs = get_parent_canvas()->get_file_system()->get_read_stream(local_filename);

			map<Time, String> phonemes;
			if (!rs)
				error("Cannot open .pgo file: %s", full_filename.c_str());
			else
			if (Parser::parse_pgo(*rs, phonemes, filefields))
				for(map<Time, String>::const_iterator i = phonemes.begin(); i != phonemes.end(); ++i)
					new_waypoint(i->first, i->second);
		}

		String uri = get_parent_canvas()->get_identifier().file_system->get_real_uri(full_filename);
		if (!uri.empty())
		{
			internal->file_monitor = Gio::File::create_for_uri(uri.c_str())->monitor_file();
			internal->file_monitor->signal_changed().connect(
				sigc::hide( sigc::hide( sigc::hide(
					sigc::mem_fun(*this, &ValueNode_AnimatedFile::file_changed) ))));
		}
	}

	ValueNode_AnimatedInterfaceConst::on_changed();
}

String
ValueNode_AnimatedFile::get_file_field(Time t, const String &field_name) const
{
	(*this)(t);
	std::map<String, String>::const_iterator i = filefields.find(field_name);
	return i == filefields.end() ? String() : i->second;
}


void
ValueNode_AnimatedFile::on_changed()
{
	ValueNode::on_changed();
	ValueNode_AnimatedInterfaceConst::on_changed();
}

ValueBase
ValueNode_AnimatedFile::operator()(Time t) const
{
	const_cast<ValueNode_AnimatedFile*>(this)->load_file((*filename)(t).get(String()));
	return ValueNode_AnimatedInterfaceConst::operator()(t);
}

ValueNode::LooseHandle
ValueNode_AnimatedFile::get_link_vfunc(int i) const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return filename;

	return ValueNode::LooseHandle();
}

bool
ValueNode_AnimatedFile::set_link_vfunc(int i, ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(filename, type_string);
	}
	return false;
}

LinkableValueNode::Vocab
ValueNode_AnimatedFile::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
		.set_description(_("File with waypoints"))
		.set_hint("filename")
	);

	return ret;
}

void
ValueNode_AnimatedFile::get_values_vfunc(std::map<Time, ValueBase> &x) const
{
	ValueNode::Handle filename = get_link("filename");
	if (!filename)
	{
		(*this)(0);
		ValueNode_AnimatedInterfaceConst::get_values_vfunc(x);
		return;
	}

	std::map<Time, ValueBase> m;
	filename->get_values(m);
	for(std::map<Time, ValueBase>::const_iterator i = m.begin(); i != m.end(); ++i)
	{
		std::map<Time, ValueBase>::const_iterator ii = i; ++ii;
		bool first = i == m.begin();
		bool last = ii == m.end();
		Time begin = i->first;
		Time end = ii->first;

		std::map<Time, ValueBase> mm;
		ValueNode::add_value_to_map(x, begin, (*this)(begin));
		ValueNode_AnimatedInterfaceConst::get_values_vfunc(mm);
		for(std::map<Time, ValueBase>::const_iterator j = mm.begin(); j != mm.end(); ++j)
			if ( (first || j->first >= begin)
			  && (last  || j->first <  end) )
				ValueNode::add_value_to_map(x, j->first, j->second);
	}
}
