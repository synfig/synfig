/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animatedfile.cpp
**	\brief Implementation of the "AnimatedFile" valuenode conversion.
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#include <libxml++/libxml++.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_AnimatedFile, RELEASE_VERSION_1_6_0, "animated_file", "Animation from File")

/* === C L A S S E S ======================================================= */
class ValueNode_AnimatedFile::Internal
{
public:
	Glib::RefPtr<Gio::FileMonitor> file_monitor;
};

class Parser
{
public:
	/// Check if it provides support for a given file extension
	static bool supports(const std::string& file_extension, const synfig::Type& value_type) {
		auto it = book.find(file_extension);
		if (it == book.end())
			return false;
		return value_type == type_string || !it->second.supports_string_only;
	}

	/// Method signature for parsing a stream
	/// \param[in] s
	/// \param[out] values the map of time -> value (phoneme/layer name if lipsync file format)
	/// \param[out] fields the map of metafield name -> metafield value
	/// \return true if successful. There is no guarantee about stream offset in case of failure
	typedef bool (*ParserFunction)(std::istream &, std::map<Time, String> &, std::map<String, String> &);

	/// A parser
	struct BookEntry {
		ParserFunction parse;
		bool supports_string_only;
	};

	/// maps file extension -> parser entry
	typedef std::map<const std::string, BookEntry> Book;

	/// Book of all implemented parsers
	static const Book book;

	/// Convert string to double in C locale
	static double strtodouble(const std::string& str) {
		std::istringstream iss(str);
		iss.imbue(std::locale::classic());
		double value;
		iss >> value;
		return value;
	}

	/// Parse a Papagayo file
	/// \param[in] s the Papagayo stream to be parsed
	/// \param[out] value the map of time -> layer name
	/// \param[out] fields the map of metafield -> value
	/// \return true if successful. There is no guarantee about stream offset in case of failure
	static bool parse_pgo(std::istream &s, std::map<Time, String> &values, std::map<String, String> &fields)
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
							first_frame = std::min(frame, first_frame);
							getline(s, word);
							values[Time(frame*fk)] = phoneme;
						} else unexpected_end = true;
					}
					values[Time(end_frame*fk)] = "rest";
				}
			}
		}
		if (first_frame == INT_MAX) first_frame= 1;
		values[Time((first_frame-1)*fk)] = "rest";

		if (unexpected_end)
			warning("Unexpected end of .pgo file. Unsupported format?");

		return true;
	}

	/// Parse a TSV file (Table Separated Value)
	/// \param[in] s the TSV stream to be parsed
	/// \param[out] value the map of time -> layer name
	/// \param[out] fields the map of metafield -> value
	/// \return true if successful. There is no guarantee about stream offset in case of failure
	static bool parse_tsv(std::istream &s, std::map<Time, String> &values, std::map<String, String> &/*fields*/)
	{
		try {
			size_t line_num = 0;
			std::string line;
			while (std::getline(s, line)) {
				line_num++;

				if (line.empty()) {
					warning(_("TSV file (line %zu): skipping empty line"), line_num);
					continue;
				}

				const auto tab_pos = line.find('\t');
				if (tab_pos == std::string::npos) {
					error(_("TSV file (line %zu): no TAB separator"), line_num);
					continue;
				}

				double seconds = strtodouble(line.substr(0, tab_pos));
				std::string data = line.substr(tab_pos+1);

				Time time(seconds);
				values[time] = data;
			}
		}  catch (...) {
			return false;
		}

		return s.eof() || !s.fail();
	}

	/// Parse a Rhubarb XML file (lipsync software)
	/// \param[in] s the XML stream to be parsed
	/// \param[out] value the map of time -> layer name
	/// \param[out] fields the map of metafield -> value
	/// \return true if successful. There is no guarantee about stream offset in case of failure
	static bool parse_xml(std::istream &s, std::map<Time, String> &values, std::map<String, String> &fields)
	{
		try {
			xmlpp::DomParser parser;
			parser.parse_stream(s);
			if (!parser)
				return false;
			auto doc = parser.get_document();
			if (!doc)
				return false;
			auto root = doc->get_root_node();
			if (!root)
				return false;
			if (root->get_name() != "rhubarbResult") {
				synfig::error(_("XML animated file: it only supports Rhubarb Lip Sync files. Found %s"), root->get_name().c_str());
				return false;
			}
			if (auto metadata_node = root->get_first_child("metadata")) {
				if (auto sound_node = metadata_node->get_first_child("soundFile")) {
					if (auto sound_element = dynamic_cast<xmlpp::Element*>(sound_node)) {
						if (auto sound_text_node = sound_element->get_child_text())
							fields["sound"] = sound_text_node->get_content();
					}
					if (fields.count("sound") == 0 || fields["sound"].empty())
						synfig::warning(_("XML animated file: wrong contents for Rhubarb metadata > soundFile node"));
				}
			}
			if (auto mouthcues_node = root->get_first_child("mouthCues")) {
				for (auto mouthcue_node : mouthcues_node->get_children("mouthCue")) {
					if (auto mouthcue_element = dynamic_cast<xmlpp::Element*>(mouthcue_node)) {
						if (auto mouthcue_text_node = mouthcue_element->get_child_text()) {
							std::string phoneme = mouthcue_text_node->get_content();
							if (phoneme.empty()) {
								synfig::warning(_("XML animated file: missing phoneme for Rhubarb mouthcue element: using X"));
								phoneme = 'X';
							}

							if (auto start_attribute = mouthcue_element->get_attribute("start")) {
								double seconds = strtodouble(start_attribute->get_value());
								Time time(seconds);
								values[time] = phoneme;
							} else {
								synfig::warning(_("XML animated file: missing attribute 'start' for Rhubarb mouthcue element: skipping"));
							}
						}
					}
				}
			}
			return true;
		} catch(xmlpp::internal_error &x) {
			if (std::string(x.what()) == "Couldn't create parsing context")
				synfig::error(_("XML animated file: Can't XML open file"));
			else
				synfig::error(_("XML animated file: %s"), x.what());
		} catch(const std::exception& ex) {
			synfig::error(_("XML animated file: Standard Exception: ")+String(ex.what()));
		} catch (...) {
			synfig::error(_("XML animated file: Unknown error"));
		}
		return false;
	}

};

static ValueBase
from_string(const Type &t, const std::string& str) {
	ValueBase v;
	if (t == type_string)
		v = str;
	else if (t == type_angle)
		v = Angle::deg(Parser::strtodouble(str));
	else if (t == type_bool)
		v = str!="0" && str!="false" && str!="FALSE" && str!="False";
	else if (t == type_integer)
		v = stoi(str);
	else if (t == type_real)
		v = Parser::strtodouble(str);
	else if (t == type_time)
		v = Time(str);
	else if (t == type_vector) {
		const auto comma1_pos = str.find(',');
		if (comma1_pos != std::string::npos) {
			std::string x_str, y_str;
			x_str = str.substr(0, comma1_pos);
			y_str = str.substr(comma1_pos+1);
			v = Vector(Parser::strtodouble(x_str), Parser::strtodouble(y_str));
		}
	}
	return v;
}

/* === M E T H O D S ======================================================= */

const Parser::Book Parser::book {
	{"pgo", Parser::BookEntry{&Parser::parse_pgo, true}}, // Papagayo
	{"tsv", Parser::BookEntry{&Parser::parse_tsv, false}},// TSV - Rhubarb, for example
	{"xml", Parser::BookEntry{&Parser::parse_xml, true}}  // Rhubarb XML file format
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
ValueNode_AnimatedFile::create(const ValueBase& x, etl::loose_handle<Canvas>)
	{ return new ValueNode_AnimatedFile(x.get_type()); }



void
ValueNode_AnimatedFile::file_changed()
{
	load_file(current_filename, true);
	changed();
}

void
ValueNode_AnimatedFile::load_file(const String &filename, bool force)
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

	if (current_filename == independent_filename && !force) return;
	current_filename = independent_filename;

	internal->file_monitor.clear();
	filefields.clear();
	erase_all();

	if (!full_filename.empty())
	{
		const std::string file_extension = CanvasFileNaming::filename_extension_lower(full_filename);
		// Read file
		if ( Parser::supports(file_extension, get_type()) )
		{
			FileSystem::ReadStream::Handle rs = get_parent_canvas()->get_file_system()->get_read_stream(full_filename);
			if (!rs)
				FileSystem::ReadStream::Handle rs = get_parent_canvas()->get_file_system()->get_read_stream(local_filename);

			std::map<Time, String> values; // phonemes for lipsync file formats
			if (!rs)
				error(_("Cannot open .%s file: %s"), file_extension.c_str(), full_filename.c_str());
			else
			if (Parser::book.at(file_extension).parse(*rs, values, filefields)) {
				for (std::map<Time, String>::const_iterator i = values.begin(); i != values.end(); ++i) {
					ValueBase v = from_string(get_type(), i->second);
					if (!v.is_valid() || v.get_type() == type_nil) {
						warning(_("Invalid value for type %s: %s at time %s, or value type is currently not supported"),
								get_type().description.local_name.c_str(),
								i->second.c_str(),
								i->first.get_string().c_str());
						continue;
					}
					new_waypoint(i->first, v);
				}
			}
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
