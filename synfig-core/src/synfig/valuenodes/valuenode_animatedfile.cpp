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

#include <istream>
#include <map>

#include <synfig/localization.h>
#include <synfig/general.h>

#include "valuenode_animatedfile.h"
#include "valuenode_const.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

class ValueNode_AnimatedFile::Parser {
public:
	static bool parse_pgo(istream &s, map<Time, String> &value)
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
					int phonemes = 0;
					s >> word >> word >> word >> phonemes;
					getline(s, word);
					for(int l = 0; l < phonemes; ++l)
					{
						int frame = 0;
						String phoneme;
						if (s)
						{
							s >> frame >> phoneme;
							getline(s, word);
							value[Time(frame*fk)] = phoneme;
						} else unexpected_end = true;
					}
				}
			}
		}

		if (unexpected_end)
			warning("Unexpected end of .pgo file. Unsupported format?");

		return true;
	}
};

ValueNode_AnimatedFile::ValueNode_AnimatedFile(Type &t):
	ValueNode_AnimatedInterfaceConst(*(ValueNode*)this),
	filename(ValueNode_Const::create(String("")))
{
	ValueNode_AnimatedInterfaceConst::set_interpolation(INTERPOLATION_CONSTANT);
	ValueNode_AnimatedInterfaceConst::set_type(t);
	set_children_vocab(get_children_vocab());
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

String
ValueNode_AnimatedFile::get_name()const
	{ return "animated_file"; }

String
ValueNode_AnimatedFile::get_local_name()const
	{ return _("Animation from File"); }

void
ValueNode_AnimatedFile::load_file(const String &filename)
{
	if (current_filename == filename) return;
	if ( !get_parent_canvas()
	  || !get_parent_canvas()->get_identifier().file_system ) return;

	erase_all();
	if (!filename.empty())
	{
		// Read papagayo file
		if (filename_extension(filename) == ".pgo")
		{
			FileSystem::ReadStreamHandle rs = get_parent_canvas()->get_identifier().file_system->get_read_stream(filename);
			map<Time, String> phonemes;
			if (!rs)
				error("Cannot open .pgo file: %s", filename.c_str());
			else
			if (Parser::parse_pgo(*rs, phonemes))
				for(map<Time, String>::const_iterator i = phonemes.begin(); i != phonemes.end(); ++i)
					new_waypoint(i->first, i->second);
		}
	}

	ValueNode_AnimatedInterfaceConst::on_changed();
	current_filename = filename;
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

