/* === S Y N F I G ========================================================= */
/*!	\file loadcanvas.cpp
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
**	Copyright (c) 2009 Nikita Kitaev
**	Copyright (c) 2011, 2012 Carlos López
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <map>
#include <vector>
#include <stdexcept>

#include <libxml++/libxml++.h>
#include <sigc++/bind.h>

#include "loadcanvas.h"

#include "general.h"
#include "localization.h"

#include "blur.h"
#include "dashitem.h"
#include "exception.h"
#include "gradient.h"
#include "layer.h"
#include "string.h"
#include "valuenode.h"
#include "valuenode_registry.h"
#include "valueoperations.h"
#include "zstreambuf.h"
#include "segment.h"

#include "layers/layer_group.h"

#include "valuenodes/valuenode_add.h"
#include "valuenodes/valuenode_animated.h"
#include "valuenodes/valuenode_bline.h"
#include "valuenodes/valuenode_bone.h"
#include "valuenodes/valuenode_bonelink.h"
#include "valuenodes/valuenode_boneweightpair.h"
#include "valuenodes/valuenode_composite.h"
#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_dilist.h"
#include "valuenodes/valuenode_dynamiclist.h"
#include "valuenodes/valuenode_exp.h"
#include "valuenodes/valuenode_scale.h"
#include "valuenodes/valuenode_weightedaverage.h"
#include "valuenodes/valuenode_wplist.h"
#include "valuenodes/valuenode_average.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

#define VALUENODE_COMPATIBILITY_URL "https://wiki.synfig.org/Convert#Compatibility"

inline bool is_whitespace(char x) { return ((x)=='\n' || (x)=='\t' || (x)==' '); }

inline bool is_true(const std::string& s) { return s=="1" || s=="true" || s=="TRUE" || s=="True"; }
inline bool is_false(const std::string& s) { return s=="0" || s=="false" || s=="FALSE" || s=="False"; }

std::set<FileSystem::Identifier> CanvasParser::loading_;

/* === P R O C E D U R E S ================================================= */

OpenCanvasMap& synfig::get_open_canvas_map()
{
	static OpenCanvasMap open_canvas_map_;
	return open_canvas_map_;
}

static void _remove_from_open_canvas_map(Canvas *x) {
    auto& map = get_open_canvas_map();
    const auto& found = map.find(x);
    if (found == map.end()) {
        synfig::error(_("Cannot find canvas for delete '%s'"), x->get_file_name().c_str());
        return;
    }
    map.erase(found);
}

static void _canvas_file_name_changed(Canvas *x)
{
	auto& canvas_map = get_open_canvas_map();
	if (canvas_map.find(x) == canvas_map.end()) {
		return;
	}
	canvas_map[x] = filesystem::Path::absolute_path(x->get_file_name());
}

Canvas::Handle
synfig::open_canvas_as(const FileSystem::Identifier &identifier,const String &as,String &errors,String &warnings)
{
	String filename = FileSystem::fix_slashes(as);
	if (CanvasParser::loading_.count(identifier))
	{
		String warning(strprintf(_("cannot load '%s' recursively"), identifier.filename.c_str()));
		synfig::warning(warning);
		warnings = "  * " + warning + "\n";
		Canvas::Handle canvas(Canvas::create());
		canvas->set_identifier(identifier);
		canvas->set_file_name(filename);
		Layer::Handle paste(Layer_Group::create());
		canvas->push_back(paste);
		paste->set_description(warning);
		return canvas;
	}

	Canvas::Handle canvas;
	CanvasParser parser;
	parser.set_allow_errors(true);

	try
	{
		CanvasParser::loading_.insert(identifier);
		canvas=parser.parse_from_file_as(identifier,filename,errors);
	}
	catch (...)
	{
		CanvasParser::loading_.erase(identifier);
		throw;
	}
	CanvasParser::loading_.erase(identifier);

	warnings = parser.get_warnings_text();

	if(parser.error_count())
	{
		errors = parser.get_errors_text();
		return Canvas::Handle();
	}

	return canvas;
}

/* === M E T H O D S ======================================================= */

void
CanvasParser::error_unexpected_element(xmlpp::Node *element,const String &got, const String &expected)
{
	error(element,strprintf(_("Unexpected element <%s>, Expected <%s>"),got.c_str(),expected.c_str()));
}

void
CanvasParser::error_unexpected_element(xmlpp::Node *element,const String &got)
{
	error(element,strprintf(_("Unexpected element <%s>"),got.c_str()));
}

void
CanvasParser::warning(xmlpp::Node *element, const String &text)
{
	std::string str=strprintf("%s:<%s>:%d: ",filename.c_str(),element->get_name().c_str(),element->get_line())+text;

	synfig::warning(str);
	// cerr<<str<<endl;

	total_warnings_++;
	warnings_text += "  * " + str + "\n";
	if(total_warnings_>=max_warnings_)
		fatal_error(element, _("Too many warnings"));
}

void
CanvasParser::error(xmlpp::Node *element, const String &text)
{
	std::string str=strprintf("%s:<%s>:%d: error: ",filename.c_str(),element->get_name().c_str(),element->get_line())+text;
	total_errors_++;
	errors_text += "  * " + str + "\n";
	if(!allow_errors_)
		throw std::runtime_error(str);
	std::cerr<<str.c_str()<<std::endl;
	//	synfig::error(str);
}

void
CanvasParser::fatal_error(xmlpp::Node *element, const String &text)
{
	std::string str=strprintf("%s:<%s>:%d:",filename.c_str(),element->get_name().c_str(),element->get_line())+text;
	throw std::runtime_error(str);
}



Keyframe
CanvasParser::parse_keyframe(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="keyframe");

	if(!element->get_attribute("time"))
	{
		error(element,strprintf(_("<%s> is missing \"%s\" attribute"),"real","time"));
		return Keyframe();
	}

	Keyframe ret(Time(element->get_attribute("time")->get_value(),canvas->rend_desc().get_frame_rate()));


	if(!element->get_children().empty())
		if(!element->get_child_text()->get_content().empty())
			ret.set_description(element->get_child_text()->get_content());
	
	bool active=true;
	if(element->get_attribute("active")) 
	{
		std::string val=element->get_attribute("active")->get_value();
		if(is_false(val))
			active=false;
	}
	ret.set_active(active);
		
	return ret;
}


Real
CanvasParser::parse_real(xmlpp::Element *element)
{
	assert(element->get_name()=="real");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"real"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"real"));
		return false;
	}

	std::string val=element->get_attribute("value")->get_value();

	return atof(val.c_str());
}

Time
CanvasParser::parse_time(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="time");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"time"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"time"));
		return false;
	}

	std::string val=element->get_attribute("value")->get_value();

	return Time(val,canvas->rend_desc().get_frame_rate());
}

int
CanvasParser::parse_integer(xmlpp::Element *element)
{
	assert(element->get_name()=="integer");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"integer"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"integer"));
		return false;
	}

	std::string val=element->get_attribute("value")->get_value();

	return atoi(val.c_str());
}

GUID
CanvasParser::parse_guid(xmlpp::Element *element)
{
	assert(element->get_name()=="guid");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"guid"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"guid"));
		return GUID::zero();
	}

	std::string val=element->get_attribute("value")->get_value();

	return GUID(val);
}

Vector
CanvasParser::parse_vector(xmlpp::Element *element)
{
	assert(element->get_name()=="vector");

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <vector>");
		return Vector();
	}

	Vector vect;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child=dynamic_cast<xmlpp::Element*>((xmlpp::Node*)*iter);
		if(!child)
			continue;
		else
		if(child->get_name()=="x")
		{
			if(child->get_children().empty())
			{
				error(element, "Undefined value in <x>");
				return Vector();
			}
			vect[0]=atof(child->get_child_text()->get_content().c_str());
		}
		else
		if(child->get_name()=="y")
		{
			if(child->get_children().empty())
			{
				error(element, "Undefined value in <y>");
				return Vector();
			}
			vect[1]=atof(child->get_child_text()->get_content().c_str());
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}
	return vect;
}

Color
CanvasParser::parse_color(xmlpp::Element *element)
{
	assert(element->get_name()=="color");

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <color>");
		return Color();
	}

	Color color(0);

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="r")
		{
			if(child->get_children().empty())
			{
				error(element, "Undefined value in <r>");
				return Color();
			}
			color.set_r(atof(child->get_child_text()->get_content().c_str()));
		}
		else
		if(child->get_name()=="g")
		{
			if(child->get_children().empty())
			{
				error(element, "Undefined value in <g>");
				return Color();
			}
			color.set_g(atof(child->get_child_text()->get_content().c_str()));
		}
		else
		if(child->get_name()=="b")
		{
			if(child->get_children().empty())
			{
				error(element, "Undefined value in <b>");
				return Color();
			}
			color.set_b(atof(child->get_child_text()->get_content().c_str()));
		}
		else
		if(child->get_name()=="a")
		{
			if(child->get_children().empty())
			{
				error(element, "Undefined value in <a>");
				return Color();
			}
			color.set_a(atof(child->get_child_text()->get_content().c_str()));
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}

	return color;
}

synfig::String
CanvasParser::parse_string(xmlpp::Element *element)
{
	assert(element->get_name()=="string");
	if(element->get_children().empty())
		return synfig::String();
	return element->get_child_text()->get_content();
}

bool
CanvasParser::parse_bool(xmlpp::Element *element)
{
	assert(element->get_name()=="bool");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"bool"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"bool"));
		return false;
	}

	std::string val=element->get_attribute("value")->get_value();

	if(is_true(val))
		return true;
	if(is_false(val))
		return false;

	error(element,strprintf(_("Bad value \"%s\" in <%s>"),val.c_str(),"bool"));

	return false;
}

Gradient
CanvasParser::parse_gradient(xmlpp::Element *node)
{
	assert(node->get_name()=="gradient");
	Gradient ret;

	xmlpp::Element::NodeList list = node->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		{
			Gradient::CPoint cpoint;
			cpoint.color=parse_color(child);

			if(!child->get_attribute("pos"))
			{
				error(child,strprintf(_("<%s> is missing \"pos\" attribute"),"gradient"));
				return Gradient();
			}

			cpoint.pos=atof(child->get_attribute("pos")->get_value().c_str());

			ret.push_back(cpoint);
		}
	}
	ret.sort();
	return ret;
}

ValueBase
CanvasParser::parse_list(xmlpp::Element *element,Canvas::Handle canvas)
{
	std::vector<ValueBase> value_list;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		{
			value_list.push_back(parse_value(child,canvas));
			if(!value_list.back().is_valid())
			{
				value_list.pop_back();
				error(child,"Bad ValueBase");
				continue;
			}
		}
	}
	return value_list;
}

Segment
CanvasParser::parse_segment(xmlpp::Element *element)
{
	assert(element->get_name()=="segment");

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <segment>");
		return Segment();
	}

	Segment seg;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="p1")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <p1>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			seg.p1=parse_vector(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="t1")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <t1>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			seg.t1=parse_vector(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="p2")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <p2>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			seg.p2=parse_vector(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="t2")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <t2>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			seg.t2=parse_vector(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}
	return seg;
}

BLinePoint
CanvasParser::parse_bline_point(xmlpp::Element *element)
{
	assert(element->get_name()=="bline_point");
	if(element->get_children().empty())
	{
		error(element, "Undefined value in <bline_point>");
		return BLinePoint();
	}

	BLinePoint ret;
	ret.set_split_tangent_both(false);

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		// Vertex
		if(child->get_name()[0]=='v' || child->get_name()=="p1")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <vertex>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			ret.set_vertex(parse_vector(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Tangent 1
		if(child->get_name()=="t1" || child->get_name()=="tangent")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <t1>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			ret.set_tangent1(parse_vector(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Tangent 2
		if(child->get_name()=="t2")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <t2>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			ret.set_tangent2(parse_vector(dynamic_cast<xmlpp::Element*>(*iter)));
			ret.set_split_tangent_both(true);
		}
		else
		// width
		if(child->get_name()=="width")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <width>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_width(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// origin
		if(child->get_name()=="origin")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <origin>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_origin(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}
	return ret;
}

Transformation
CanvasParser::parse_transformation(xmlpp::Element *element)
{
	assert(element->get_name()=="transformation");

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <transformation>");
		return Transformation();
	}

	Transformation transformation;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="offset")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <offset>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			transformation.offset=parse_vector(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="angle")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <angle>");
				continue;
			}

			if((*iter)->get_name()!="angle")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"angle");
				continue;
			}

			transformation.angle=parse_angle(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="skew_angle")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <angle>");
				continue;
			}

			if((*iter)->get_name()!="angle")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"angle");
				continue;
			}

			transformation.skew_angle=parse_angle(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="scale")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <scale>");
				continue;
			}

			if((*iter)->get_name()!="vector")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"vector");
				continue;
			}

			transformation.scale=parse_vector(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}
	return transformation;
}

WidthPoint
CanvasParser::parse_width_point(xmlpp::Element *element)
{
	assert(element->get_name()=="width_point");
	if(element->get_children().empty())
	{
		error(element, "Undefined value in <width_point>");
		return WidthPoint();
	}

	WidthPoint ret;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		// Position
		if(child->get_name()=="position")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <position>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_position(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Width
		if(child->get_name()=="width")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <width>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_width(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Side type before
		if(child->get_name()=="side_before")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <side_before>");
				continue;
			}

			if((*iter)->get_name()!="integer")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"integer");
				continue;
			}

			ret.set_side_type_before(parse_integer(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Side type after
		if(child->get_name()=="side_after")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <side_after>");
				continue;
			}
			if((*iter)->get_name()!="integer")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"integer");
				continue;
			}
			ret.set_side_type_after(parse_integer(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Lower Boundary
		if(child->get_name()=="lower_bound")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <lower_bound>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_lower_bound(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		// Upper Boundary
		else
		if(child->get_name()=="upper_bound")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <upper_bound>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_upper_bound(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
			error_unexpected_element(child,child->get_name());
	}
	return ret;
}

DashItem
CanvasParser::parse_dash_item(xmlpp::Element *element)
{
	assert(element->get_name()=="dash_item");
	if(element->get_children().empty())
	{
		error(element, "Undefined value in <dash_item>");
		return DashItem();
	}

	DashItem ret;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		// Offset
		if(child->get_name()=="offset")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <offset>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_offset(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Length
		if(child->get_name()=="length")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <length>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			ret.set_length(parse_real(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Side type before
		if(child->get_name()=="side_before")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <side_before>");
				continue;
			}

			if((*iter)->get_name()!="integer")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"integer");
				continue;
			}

			ret.set_side_type_before(parse_integer(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
		// Side type after
		if(child->get_name()=="side_after")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <side_after>");
				continue;
			}
			if((*iter)->get_name()!="integer")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"integer");
				continue;
			}
			ret.set_side_type_after(parse_integer(dynamic_cast<xmlpp::Element*>(*iter)));
		}
		else
			error_unexpected_element(child,child->get_name());
	}
	return ret;
}



Angle
CanvasParser::parse_angle(xmlpp::Element *element)
{
	assert(element->get_name()=="angle");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"angle"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"angle"));
		return Angle();
	}

	std::string val=element->get_attribute("value")->get_value();

	return Angle::deg(atof(val.c_str()));
}

ValueBase
CanvasParser::parse_weighted_value(xmlpp::Element *element, types_namespace::TypeWeightedValueBase &type, Canvas::Handle canvas)
{
	assert(element->get_name()==type.description.name);

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <" + type.description.name + ">");
		return ValueBase();
	}

	Real weight = 0.0;
	ValueBase value;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="weight")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <weight>");
				continue;
			}

			if((*iter)->get_name()!="real")
			{
				error_unexpected_element((*iter),(*iter)->get_name(),"real");
				continue;
			}

			weight = parse_real(dynamic_cast<xmlpp::Element*>(*iter));
		}
		else
		if(child->get_name()=="value")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <value>");
				continue;
			}

			if((*iter)->get_name()!=type.get_contained_type().description.name)
			{
				error_unexpected_element((*iter),(*iter)->get_name(),type.get_contained_type().description.name);
				continue;
			}

			value = parse_value(dynamic_cast<xmlpp::Element*>(*iter),canvas);
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}

	return type.create_weighted_value(weight, value);
}

ValueBase
CanvasParser::parse_pair(xmlpp::Element *element, types_namespace::TypePairBase &type, Canvas::Handle canvas)
{
	assert(element->get_name()==type.description.name);

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <" + type.description.name + ">");
		return ValueBase();
	}

	ValueBase first;
	ValueBase second;

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="first")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <first>");
				continue;
			}

			if((*iter)->get_name()!=type.get_first_type().description.name)
			{
				error_unexpected_element((*iter),(*iter)->get_name(),type.get_first_type().description.name);
				continue;
			}

			first = parse_value(dynamic_cast<xmlpp::Element*>(*iter),canvas);
		}
		else
		if(child->get_name()=="second")
		{
			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(element, "Undefined value in <second>");
				continue;
			}

			if((*iter)->get_name()!=type.get_second_type().description.name)
			{
				error_unexpected_element((*iter),(*iter)->get_name(),type.get_second_type().description.name);
				continue;
			}

			second = parse_value(dynamic_cast<xmlpp::Element*>(*iter),canvas);
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}

	return type.create_value(first, second);
}

Interpolation
CanvasParser::parse_interpolation(xmlpp::Element *element,String attribute)
{
	if(!element->get_attribute(attribute))
		return INTERPOLATION_UNDEFINED;

	std::string val=element->get_attribute(attribute)->get_value();
	if(val=="halt")
		return INTERPOLATION_HALT;
	else if(val=="constant")
		return INTERPOLATION_CONSTANT;
	else if(val=="linear")
		return INTERPOLATION_LINEAR;
	else if(val=="manual")
		return INTERPOLATION_MANUAL;
	else if(val=="auto")
		return INTERPOLATION_TCB;
	else if(val=="clamped")
		return INTERPOLATION_CLAMPED;
	else
		error(element,strprintf(_("Bad value \"%s\" in <%s>"),val.c_str(),attribute.c_str()));

	return INTERPOLATION_UNDEFINED;
}

bool
CanvasParser::parse_static(xmlpp::Element *element)
{
	if(!element->get_attribute("static"))
		return false;

	std::string val=element->get_attribute("static")->get_value();

	if(is_true(val))
		return true;
	if(is_false(val))
		return false;

	error(element,strprintf(_("Bad value \"%s\" in <%s>"),val.c_str(),"bool"));

	return false;
}


ValueBase
CanvasParser::parse_value(xmlpp::Element *element,Canvas::Handle canvas)
{
	if(element->get_name()=="real")
	{
		ValueBase ret;
		ret.set(parse_real(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="time")
	{
		ValueBase ret;
		ret.set(parse_time(element,canvas));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="integer")
	{
		ValueBase ret;
		ret.set(parse_integer(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="string")
	{
		ValueBase ret;
		ret.set(parse_string(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="vector")
	{
		ValueBase ret;
		ret.set(parse_vector(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="color")
	{
		ValueBase ret;
		ret.set(parse_color(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="segment")
	{
		ValueBase ret;
		ret.set(parse_segment(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="list")
		return parse_list(element,canvas);
	else
	if(element->get_name()=="gradient")
	{
		ValueBase ret;
		ret.set(parse_gradient(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="bool")
	{
		ValueBase ret;
		ret.set(parse_bool(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="angle" || element->get_name()=="degrees" || element->get_name()=="radians" || element->get_name()=="rotations")
	{
		ValueBase ret;
		ret.set(parse_angle(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="bline_point")
		return parse_bline_point(element);
	else
	if(element->get_name()=="guid")
		return parse_guid(element).get_string();
	else
	if(element->get_name()=="width_point")
		return parse_width_point(element);
	else
	if(element->get_name()=="dash_item")
		return parse_dash_item(element);
	else
	if(element->get_name()=="transformation")
	{
		ValueBase ret;
		ret.set(parse_transformation(element));
		ret.set_static(parse_static(element));
		ret.set_interpolation(parse_interpolation(element,"interpolation"));
		return ret;
	}
	else
	if(element->get_name()=="canvas")
	{
		ValueBase ret;
		ret.set(parse_canvas(element,canvas,true));
		ret.set_static(parse_static(element));
		return ret;
	}
	else
	{
		// template types
		Type *type = Type::try_get_type_by_name(element->get_name());

		{ // weighted value
			types_namespace::TypeWeightedValueBase *type_weighted_value =
				dynamic_cast<types_namespace::TypeWeightedValueBase*>(type);
			if (type_weighted_value)
			{
				ValueBase ret = parse_weighted_value(element, *type_weighted_value, canvas);
				ret.set_static(parse_static(element));
				ret.set_interpolation(parse_interpolation(element,"interpolation"));
				return ret;
			}
		}

		{ // pair
			types_namespace::TypePairBase *type_pair =
				dynamic_cast<types_namespace::TypePairBase*>(type);
			if (type_pair)
			{
				ValueBase ret = parse_pair(element, *type_pair, canvas);
				ret.set_static(parse_static(element));
				ret.set_interpolation(parse_interpolation(element,"interpolation"));
				return ret;
			}
		}

		// else
		printf("%s:%d\n", __FILE__, __LINE__);
		error_unexpected_element(element,element->get_name());
	}

	return ValueBase();
}




ValueNode_Animated::Handle
CanvasParser::parse_animated(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="hermite" || element->get_name()=="animated");

	if(!element->get_attribute("type"))
	{
		error(element,"Missing attribute \"type\" in <animated>");
		return ValueNode_Animated::Handle();
	}

	Type &type = ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(type == type_nil)
	{
		error(element,"Bad type in <animated>");
		return ValueNode_Animated::Handle();
	}

	ValueNode_Animated::Handle value_node=ValueNode_Animated::create(type);

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <animated> with type \"%s\""), type.description.local_name.c_str()));
		return ValueNode_Animated::Handle();
	}

	value_node->set_root_canvas(canvas->get_root());
	
	if(element->get_attribute("interpolation"))
	{
		value_node->set_interpolation(parse_interpolation(element, "interpolation"));
	}

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="waypoint")
		{
			if(!child->get_attribute("time"))
			{
				error(child,_("<waypoint> is missing attribute \"time\""));
				continue;
			}

			Time time(child->get_attribute("time")->get_value(),canvas->rend_desc().get_frame_rate());


			ValueNode::Handle waypoint_value_node;
			xmlpp::Element::NodeList list = child->get_children();

			if(child->get_attribute("use"))
			{
				if(!list.empty())
					warning(child,_("Found \"use\" attribute for <waypoint>, but it wasn't empty. Ignoring contents..."));

				// the waypoint might look like this, in which case we won't find "mycanvas" in the list of valuenodes, 'cos it's a canvas
				//
				//      <animated type="canvas">
				//        <waypoint time="0s" use="mycanvas"/>
				//      </animated>
				if (type==type_canvas)
				{
					String warnings;
					waypoint_value_node=ValueNode_Const::create(canvas->surefind_canvas(child->get_attribute("use")->get_value(), warnings));
					warnings_text += warnings;
				}
				else
					waypoint_value_node=canvas->surefind_value_node(child->get_attribute("use")->get_value());
				if(PlaceholderValueNode::Handle::cast_dynamic(waypoint_value_node))
					error(child, strprintf(_("Unknown ID (%s) referenced in waypoint"),child->get_attribute("use")->get_value().c_str()));
			}
			else
			{
				if(child->get_children().empty())
				{
					error(child, strprintf(_("<%s> is missing its data"),"waypoint"));
					continue;
				}

				xmlpp::Element::NodeList::iterator iter;

				// Search for the first non-text XML element
				for(iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::Element*>(*iter)) break;

				if(iter==list.end())
				{
					error(child, strprintf(_("<%s> is missing its data"),"waypoint"));
					continue;
				}

				waypoint_value_node=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				/*
				ValueBase data=parse_value(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!data.is_valid())
				{
					error(child,_("Bad data for <waypoint>"));
					continue;
				}
				*/
				if(!waypoint_value_node)
				{
					error(child,_("Bad data for <waypoint>"));
					continue;
				}

				/*! HACK -- This is a temporary fix to help repair some
				**	weirdness that is currently going on (10-21-2004).
				**	This short circuits the linking of waypoints,
				**	a feature which is so obscure that we can get
				**	away with something like this pretty easily.
				*/
// doo			waypoint_value_node=waypoint_value_node->clone(canvas);
				//note: commented out as part of bones branch merge

				// Warn if there is trash after the param value
				for (++iter; iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::Element*>(*iter))
						warning((*iter),strprintf(_("Unexpected element <%s> after <waypoint> data, ignoring..."),(*iter)->get_name().c_str()));
			}


			try {
				ValueNode_Animated::WaypointList::iterator waypoint=value_node->new_waypoint(time,waypoint_value_node);

			if(child->get_attribute("tension"))
			{
				synfig::String str(child->get_attribute("tension")->get_value());
				waypoint->set_tension(atof(str.c_str()));
			}
			if(child->get_attribute("temporal-tension"))
			{
				synfig::String str(child->get_attribute("temporal-tension")->get_value());
				waypoint->set_temporal_tension(atof(str.c_str()));
			}
			if(child->get_attribute("continuity"))
			{
				synfig::String str(child->get_attribute("continuity")->get_value());
				waypoint->set_continuity(atof(str.c_str()));
			}
			if(child->get_attribute("bias"))
			{
				synfig::String str(child->get_attribute("bias")->get_value());
				waypoint->set_bias(atof(str.c_str()));
			}

			if(child->get_attribute("before"))
			{
				waypoint->set_before(parse_interpolation(child,"before"));
			}

			if(child->get_attribute("after"))
			{
				waypoint->set_after(parse_interpolation(child,"after"));
			}
			}
			catch(Exception::BadTime& x)
			{
				warning(child, x.what());
			}
			continue;

		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}

	// in canvas version 0.1, angles used to wrap, so to get from -179
	// degrees to 180 degrees meant a 1 degree change
	// in canvas version 0.2 they don't, so that's a 359 degree change

	// when loading a version 0.1 canvas, modify constant angle
	// waypoints to that they are within 180 degrees of the previous
	// waypoint's value
	if (type == type_angle)
	{
		if (canvas->get_version() == "0.1")
		{
			bool first = true;
			Real angle, prev = 0;
			WaypointList &wl = value_node->editable_waypoint_list();
			for (WaypointList::iterator iter = wl.begin(); iter != wl.end(); ++iter) {
				angle = Angle::deg(iter->get_value(iter->get_time()).get(Angle())).get();
				if (first)
					first = false;
				else if (iter->get_value_node()->get_name() == "constant")
				{
					if (angle - prev > 180)
					{
						while (angle - prev > 180) angle -= 360;
						iter->set_value(Angle::deg(angle));
					}
					else if (prev - angle > 180)
					{
						while (prev - angle > 180) angle += 360;
						iter->set_value(Angle::deg(angle));
					}
				}
				prev = angle;
			}
		}
	}

	value_node->changed();
	return value_node;
}

etl::handle<LinkableValueNode>
CanvasParser::parse_linkable_value_node(xmlpp::Element *element,Canvas::Handle canvas)
{
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_linkable_value_node\n", __FILE__, __LINE__);

	// Determine the type
	if(!element->get_attribute("type"))
	{
		error(element, strprintf(_("Missing attribute \"type\" in <%s>"), element->get_name().c_str()));
		printf("%s:%d parse_linkable_value_node done missing attr\n", __FILE__, __LINE__);
		return 0;
	}
	Type &type = ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(type == type_nil)
	{
		error(element, strprintf(_("Bad type in <%s>"), element->get_name().c_str()));
		printf("%s:%d parse_linkable_value_node done bad type\n", __FILE__, __LINE__);
		return 0;
	}

	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d creating linkable '%s' type '%s'\n", __FILE__, __LINE__, element->get_name().c_str(), type.description.name.c_str());
	LinkableValueNode::Handle value_node=ValueNodeRegistry::create(element->get_name(),type);
	//ValueNode::Handle c[value_node->link_count()]; changed because of clang complain
	std::vector<ValueNode::Handle> c(value_node->link_count());

	if(!value_node)
	{
		error(element, strprintf(_("Error creating ValueNode <%s> with type '%s'. Refer to '%s'"),
								 element->get_name().c_str(),
								 type.description.local_name.c_str(),
								 VALUENODE_COMPATIBILITY_URL));
		printf("%s:%d parse_linkable_value_node done error creating\n", __FILE__, __LINE__);
		return 0;
	}

	if(value_node->get_type()!=type)
	{
		error(element, strprintf(_("<%s> did not accept type '%s'"),
								 element->get_name().c_str(),
								 type.description.local_name.c_str()));
		printf("%s:%d parse_linkable_value_node unacceptable type\n", __FILE__, __LINE__);
		return 0;
	}

	// handle exported valuenodes
	{
		int index;
		String id, name;
		xmlpp::Element::AttributeList attrib_list(element->get_attributes());
		for (xmlpp::Element::AttributeList::iterator iter = attrib_list.begin(); iter != attrib_list.end(); ++iter) {
			name = (*iter)->get_name();
			id = (*iter)->get_value();

			if (name == "guid" || name == "id" || name == "type")
				continue;

			try {
				bool load_old_weighted_bonelink = false;
				if (ValueNode_BoneLink::Handle::cast_dynamic(value_node) && name == "bone_weight_list")
				{
					name = "bone";
					load_old_weighted_bonelink = true;
				}

				index = value_node->get_link_index_from_name(name);

				if(c[index])
				{
					error(element,strprintf(_("'%s' was already defined in <%s>"),
											name.c_str(),
											element->get_name().c_str()));
					continue;
				}
				int placeholders(canvas->value_node_list().placeholder_count());
				c[index] = canvas->surefind_value_node(id);
				// Don't accept links for unsolved exported Value Nodes.
				// Except if it is parsing <bones>, as this section is defined before <defs>
				if (!in_bones_section) {
					if(placeholders == canvas->value_node_list().placeholder_count())
						if(PlaceholderValueNode::Handle::cast_dynamic(c[index]) )
							throw Exception::IDNotFound("parse_linkable_value_noode()");
				}

				if (!c[index])
				{
					error(element, strprintf(_("'%s' attribute in <%s> references unknown ID '%s'"),
											 name.c_str(),
											 element->get_name().c_str(),
											 id.c_str()));
					continue;
				}

				if (load_old_weighted_bonelink)
				{
					ValueNode_StaticList::Handle list = ValueNode_StaticList::Handle::cast_dynamic(c[index]);
					ValueNode_BoneWeightPair::Handle wp = ValueNode_BoneWeightPair::Handle::cast_dynamic(list->get_link(0));
					ValueNode::Handle bone = wp->get_link(0);
					
					c[index] = bone;
				}

				if(!value_node->set_link(index, c[index]))
				{
					error(element, strprintf(_("Unable to set link '\"%s\" to ValueNode \"%s\" (link #%d in \"%s\")"),
											 value_node->link_name(index).c_str(),
											 id.c_str(),
											 index,
											 element->get_name().c_str()));
					continue;
				}

				// printf("  <%s> set link %d (%s) using exported value\n", element->get_name().c_str(), index, name.c_str());
			}
			catch (Exception::BadLinkName&)
			{
				warning(element, strprintf("Bad link name '%s'", name.c_str()));
			}
			catch(Exception::IDNotFound&)
			{
				error(element,"Unable to resolve " + id);
			}
			catch(Exception::FileNotFound&)
			{
				error(element,"Unable to open file referenced in " + id);
			}
			catch(...)
			{
				error(element,strprintf(_("Unknown Exception thrown when referencing ValueNode \"%s\""), id.c_str()));
				throw;
			}
		}
	}

	// handle inline valuenodes
	{
		int index;
		String child_name;
		xmlpp::Element::NodeList list = element->get_children();
		for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
		{
			xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
			try
			{
				if(!child)
					continue;

				child_name = child->get_name();

				bool load_old_weighted_bonelink = false;
				if (ValueNode_BoneLink::Handle::cast_dynamic(value_node) && child_name == "bone_weight_list"){
					synfig::info("!!!");
					child_name = "bone";
					load_old_weighted_bonelink = true;
				}

				index = value_node->get_link_index_from_name(child_name);

				if(c[index])
				{
					error(child, strprintf(_("'%s' was already defined in <%s>"),
										   child_name.c_str(),
										   element->get_name().c_str()));
					break;
				}

				xmlpp::Element::NodeList list = child->get_children();
				xmlpp::Element::NodeList::iterator iter;

				// Search for the first non-text XML element
				for(iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::Element*>(*iter)) break;

				if(iter==list.end())
				{
					error(child,strprintf(_("element <%s> is missing its contents"),
										  child_name.c_str()));
					continue;
				}

				c[index]=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!c[index])
				{
					error((*iter),strprintf(_("Parse of '%s' failed"),
											child_name.c_str()));
					continue;
				}

				if (load_old_weighted_bonelink)
				{
					ValueNode_StaticList::Handle list = ValueNode_StaticList::Handle::cast_dynamic(c[index]);
					ValueNode_BoneWeightPair::Handle wp = ValueNode_BoneWeightPair::Handle::cast_dynamic(list->get_link(0));
					ValueNode::Handle bone = wp->get_link(0);
					
					c[index] = bone;
				}

				if(!value_node->set_link(index,c[index]))
				{
					error(child,strprintf(_("Unable to connect value node ('%s' of type '%s') to link %d (%s)"),
										  c[index]->get_name().c_str(),
										  c[index]->get_type().description.local_name.c_str(),
										  index,
										  value_node->link_name(index).c_str()));
					continue;
				}

				// \todo do a search for more elements and warn if they are found

				// printf("  <%s> set link %d (%s) using inline value\n", element->get_name().c_str(), index, child_name.c_str());
			}
			catch(Exception::BadLinkName&)
			{
				warning(child, strprintf("Bad link name for <%s>", element->get_name().c_str()));
			}
			catch(...)
			{
				error(child, strprintf(_("Unknown Exception thrown when working on element \"%s\""),child_name.c_str()));
				throw;
			}
		}
	}

	String version(canvas->get_version());
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d link_count() is %d\n", __FILE__, __LINE__, value_node->link_count());
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d value_node is %s\n", __FILE__, __LINE__, value_node->get_string().c_str());
	for (int i = 0; i < value_node->link_count(); i++)
	{
		if (!c[i])
		{
			// the 'width' parameter of <stripes> wasn't always present in version 0.1 canvases
			if (version == "0.1" && element->get_name() == "stripes" && value_node->link_name(i) == "width")
				continue;

			// these 3 blinecalctangent parameters didn't appear until canvas version 0.5
			if ((version == "0.1" || version == "0.2" || version == "0.3" || version == "0.4") &&
				element->get_name() == "blinecalctangent" &&
				(value_node->link_name(i) == "offset" ||
				 value_node->link_name(i) == "scale" ||
				 value_node->link_name(i) == "fixed_length"))
				continue;

			// 'scale' was added while canvas version 0.5 was in use
			if ((version == "0.3" || version == "0.4" || version == "0.5") &&
				element->get_name() == "blinecalcwidth" &&
				value_node->link_name(i) == "scale")
				continue;

			// 'loop' was added while canvas version 0.5 was in use, as was the 'gradientcolor' node type
			if (version == "0.5" &&
				element->get_name() == "gradientcolor" &&
				value_node->link_name(i) == "loop")
				continue;

			// 'loop' was added while canvas version 0.6 was in use; the 'random' node was added back when 0.1 was in use
			if ((version == "0.1" || version == "0.2" || version == "0.3" || version == "0.4" || version == "0.5" || version == "0.6") &&
				element->get_name() == "random" &&
				value_node->link_name(i) == "loop")
				continue;

			// todo: remove this - it's temporary; accept bones with 'scalel' missing - it's new
			// todo: remove this AFTER release, which is subsequent to 0.64.1
			if (element->get_name() == "bone" &&
				(value_node->link_name(i) == "scalel" ||
				 value_node->link_name(i) == "scalelx" ||
				 value_node->link_name(i) == "scalely" ||
				 value_node->link_name(i) == "width" ||
				 value_node->link_name(i) == "tipwidth" ||
				 value_node->link_name(i) == "bone_depth"))
				continue;

			// 'homogeneous' was added while canvas version 0.7 was in use and the BLineCalcVertex,
			// BLineCalcTangent and BLineCalcWidth have been modified since canvas version 0.5
			if ((version == "0.5" || version == "0.6" || version == "0.7") &&
				(element->get_name() == "blinecalcvertex" || element->get_name() == "blinecalctangent" || element->get_name() == "blinecalcwidth") &&
				value_node->link_name(i) == "homogeneous")
			{
				// old versions aren't homogeneous (new versions are homogeneous by default)
				value_node->set_link("homogeneous", ValueNode_Const::create(bool(false)));
				continue;
			}
			// 'lower_bound' was added while canvas 0.8 was in use and
			// ValueNode_Composite has been modified since canvas version 0.7
			if((version=="0.8" || version =="0.7") &&
				(element->get_name() == "composite") && value_node->link_name(i) =="lower_bound")
			{
				// old versions have lower boundary set to 0.0
				value_node->set_link("lower_bound", ValueNode_Const::create(Real(0.0)));
				continue;
			}
			// 'upper_bound' was added while canvas 0.8 was in use and
			// ValueNode_Composite has been modified since canvas version 0.7
			if((version=="0.8" || version =="0.7") &&
				(element->get_name() == "composite") && value_node->link_name(i) =="upper_bound")
			{
				// old versions have upper boundary set to 1.0
				value_node->set_link("upper_bound", ValueNode_Const::create(Real(1.0)));
				continue;
			}
			// 'split_radius' was added while canvas 0.9 was in use and
			// ValueNode_Composite has been modified since canvas version 0.7
			if((version == "0.1" || version == "0.2" || version == "0.3" || version == "0.4" || version == "0.5" || version == "0.6" || version=="0.7" || version=="0.8" || version =="0.9") &&
			   (element->get_name() == "composite") && value_node->link_name(i) =="split_radius")
			{
				// old versions must have split_radius and split_angle set to the same value node than split
				value_node->set_link("split_radius", value_node->get_link("split")->clone(canvas));
				continue;
			}
			// 'split_angle' was added while canvas 0.9 was in use and
			// ValueNode_Composite has been modified since canvas version 0.7
			if((version == "0.1" || version == "0.2" || version == "0.3" || version == "0.4" || version == "0.5" || version == "0.6" || version=="0.7" || version=="0.8" || version =="0.9") &&
			   (element->get_name() == "composite") && value_node->link_name(i) =="split_angle")
			{
				// old versions must have split_radius and split_angle set to the same value node than split
				value_node->set_link("split_angle", value_node->get_link(value_node->get_link_index_from_name("split"))->clone(canvas));
				continue;
			}

			error(element, strprintf(_("<%s> is missing link %d (%s)"),
									 element->get_name().c_str(),
									 i,
									 value_node->link_name(i).c_str()));
 			return 0;
		}
	}

	// pre 0.4 canvases had *calctangent outputs scaled down by 0.5 for some reason
	if (element->get_name() == "blinecalctangent" || element->get_name() == "segcalctangent")
	{
		if (version == "0.1" || version == "0.2" || version == "0.3")
		{
			LinkableValueNode::Handle scale_value_node=ValueNodeRegistry::create("scale",type);
			scale_value_node->set_link("link", value_node);
			scale_value_node->set_link("scalar", ValueNode_Const::create(Real(0.5)));

			value_node = scale_value_node;
		}
	}

	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_linkable_value_node done\n", __FILE__, __LINE__);
	return value_node;
}

ValueNode_StaticList::Handle
CanvasParser::parse_static_list(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="static_list");

	if(!element->get_attribute("type"))
	{
		error(element,"Missing attribute \"type\" in <list>");
		return ValueNode_StaticList::Handle();
	}

	Type &type=ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(type == type_nil)
	{
		error(element,"Bad type in <list>");
		return ValueNode_StaticList::Handle();
	}

	ValueNode_StaticList::Handle value_node;

	value_node=ValueNode_StaticList::create_on_canvas(type);

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <list>")));
		return ValueNode_StaticList::Handle();
	}

	value_node->set_root_canvas(canvas->get_root());

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="entry")
		{
			ValueNode::Handle list_entry;

			if(child->get_attribute("use"))
			{
				// \todo does this need to be able to read 'use="canvas"', like waypoints can now?  (see 'surefind_canvas' in this file)
				std::string id=child->get_attribute("use")->get_value();
				try
				{
					list_entry=canvas->surefind_value_node(id);
				}
				catch(Exception::IDNotFound&)
				{
					error(child,"\"use\" attribute in <entry> references unknown ID -- "+id);
					continue;
				}
			}
			else
			{
				xmlpp::Element::NodeList list = child->get_children();
				xmlpp::Element::NodeList::iterator iter;

				// Search for the first non-text XML element
				for(iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::Element*>(*iter)) break;

				if(iter==list.end())
				{
					error(child,strprintf(_("<entry> is missing its contents or missing \"use\" element")));
					continue;
				}

				list_entry=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!list_entry)
					error((*iter),"Parse of ValueNode failed");

				// \todo do a search for more elements and warn if they are found

			}

			value_node->add(list_entry);
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}
	return value_node;
}

static bool is_bool_attribute_true(xmlpp::Element* element, const char* name) {
	if (xmlpp::Attribute* attr = element->get_attribute(name)) {
		return is_true(attr->get_value());
	}
	return false;
}

// This will also parse a bline
ValueNode_DynamicList::Handle
CanvasParser::parse_dynamic_list(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="dynamic_list" ||
		element->get_name()=="bline" ||
		element->get_name()=="wplist" ||
		element->get_name()=="dilist" ||
		element->get_name()=="average" ||
		element->get_name()=="weighted_average" );

	const float fps(canvas?canvas->rend_desc().get_frame_rate():0);

	if(!element->get_attribute("type"))
	{
		error(element,"Missing attribute \"type\" in <dynamic_list>");
		return ValueNode_DynamicList::Handle();
	}

	Type &type = ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(type == type_nil)
	{
		error(element,"Bad type in <dynamic_list>");
		return ValueNode_DynamicList::Handle();
	}

	ValueNode_DynamicList::Handle value_node;

	bool must_rotate_point_list = false;

	if(element->get_name()=="bline")
	{
		value_node = ValueNode_BLine::create(type_list, canvas);
		if (is_bool_attribute_true(element, "loop")) {
			std::string version = canvas->get_version();
			if (version == "1.0" || (version[0] == '0' && version[1] == '.'))
				must_rotate_point_list = true;
		}
	}
	else if(element->get_name()=="wplist")
	{
		value_node = ValueNode_WPList::create();
	}
	else if(element->get_name()=="dilist")
	{
		value_node = ValueNode_DIList::create();
	}
	else if(element->get_name()=="weighted_average")
	{
		Type& contained_type = ValueAverage::get_type_from_weighted(type);
		value_node = new ValueNode_WeightedAverage(contained_type, canvas);
	} else if (element->get_name()=="average") {
		value_node = new synfig::ValueNode_Average(type, canvas);
	}
	else
		value_node = ValueNode_DynamicList::create_on_canvas(type);

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <dynamic_list>")));
		return ValueNode_DynamicList::Handle();
	}

	value_node->set_root_canvas(canvas->get_root());

	if (is_bool_attribute_true(element, "loop"))
		value_node->set_loop(true);

	xmlpp::Element::NodeList list = element->get_children();

	if (must_rotate_point_list) {
		if (list.size() > 0) {
			while (dynamic_cast<xmlpp::Element*>(list.back()) == nullptr) {
				list.pop_back();
			}
			if (list.size() > 0) {
				xmlpp::Node * node = list.back();
				list.pop_back();
				list.push_front(node);
			}
		}
	}

	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="entry")
		{
			ValueNode_DynamicList::ListEntry list_entry;

			// Parse begin/end waypoints
			{
				typedef synfig::ValueNode_DynamicList::ListEntry::Activepoint Activepoint;
				typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

				String begin_sequence;
				String end_sequence;

				ActivepointList &timing_info(list_entry.timing_info);

				if(child->get_attribute("begin"))
					begin_sequence=child->get_attribute("begin")->get_value();

				if(child->get_attribute("on"))
					begin_sequence=child->get_attribute("on")->get_value();

				if(child->get_attribute("end"))
					end_sequence=child->get_attribute("end")->get_value();

				if(child->get_attribute("off"))
					end_sequence=child->get_attribute("off")->get_value();

				// clear out any auto-start
				if(!begin_sequence.empty())
					timing_info.clear();

				//! \optimize
				while(!begin_sequence.empty())
				{
					String::iterator iter(find(begin_sequence.begin(),begin_sequence.end(),','));
					String timecode(begin_sequence.begin(),	iter);
					int priority=0;

					// skip whitespace before checking for a priority
					while (isspace(timecode[0]))
						timecode=timecode.substr(1);

					// If there is a priority, then grab it and remove
					// it from the timecode
					if(timecode[0]=='p')
					{
						//priority=timecode[1]-'0';
						//timecode=String(timecode.begin()+3,timecode.end());
						int space=timecode.find_first_of(' ');
						priority=atoi(String(timecode,1,space-1).c_str());
						timecode=String(timecode.begin()+space+1,timecode.end());
						//synfig::info("priority: %d      timecode: %s",priority,timecode.c_str());
					}

					timing_info.push_back(
						Activepoint(
							Time(
								timecode,
								fps
							),
							true,	// Mark as a "on" activepoint
							priority
						)
					);

					if(iter==begin_sequence.end())
						begin_sequence.clear();
					else
						begin_sequence=String(iter+1,begin_sequence.end());
				}

				//! \optimize
				while(!end_sequence.empty())
				{
					String::iterator iter(find(end_sequence.begin(),end_sequence.end(),','));
					String timecode(end_sequence.begin(),	iter);
					int priority=0;

					// skip whitespace before checking for a priority
					while (isspace(timecode[0]))
						timecode=timecode.substr(1);

					// If there is a priority, then grab it and remove
					// it from the timecode
					if(timecode[0]=='p')
					{
						//priority=timecode[1]-'0';
						//timecode=String(timecode.begin()+3,timecode.end());
						int space=timecode.find_first_of(' ');
						priority=atoi(String(timecode,1,space-1).c_str());
						timecode=String(timecode.begin()+space+1,timecode.end());
						//synfig::info("priority: %d      timecode: %s",priority,timecode.c_str());
					}

					timing_info.push_back(
						Activepoint(
							Time(
								timecode,
								fps
							),
							false,	// Mark as a "off" activepoint
							priority
						)
					);
					if(iter==end_sequence.end())
						end_sequence.clear();
					else
						end_sequence=String(iter+1,end_sequence.end());
				}

				timing_info.sort();
			}

			if(child->get_attribute("use"))
			{
				// \todo does this need to be able to read 'use="canvas"', like waypoints can now?  (see 'surefind_canvas' in this file)
				std::string id=child->get_attribute("use")->get_value();
				try
				{
					list_entry.value_node=canvas->surefind_value_node(id);
					if(PlaceholderValueNode::Handle::cast_dynamic(list_entry.value_node))
						throw Exception::IDNotFound("parse_dynamic_list()");
				}
				catch(Exception::IDNotFound&)
				{
					error(child,"\"use\" attribute in <entry> references unknown ID -- "+id);
					continue;
				}
			}
			else
			{
				xmlpp::Element::NodeList list = child->get_children();
				xmlpp::Element::NodeList::iterator iter;

				// Search for the first non-text XML element
				for(iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::Element*>(*iter)) break;

				if(iter==list.end())
				{
					error(child,strprintf(_("<entry> is missing its contents or missing \"use\" element")));
					continue;
				}

				list_entry.value_node=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!list_entry.value_node)
					error((*iter),"Parse of ValueNode failed");

				// \todo do a search for more elements and warn if they are found

			}

			value_node->add(list_entry);
			value_node->set_link(value_node->link_count()-1,list_entry.value_node);
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}
	return value_node;
}

ValueNode::Handle
CanvasParser::parse_value_node(xmlpp::Element *element,Canvas::Handle canvas)
{
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node\n", __FILE__, __LINE__);
	ValueNode::Handle value_node;
	assert(element);

	GUID guid;

	if(element->get_attribute("guid"))
	{
		guid=GUID(element->get_attribute("guid")->get_value())^canvas->get_root()->get_guid();
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d got guid %s\n", __FILE__, __LINE__, guid.get_string().c_str());
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d and element name = '%s'\n", __FILE__, __LINE__, element->get_name().c_str());
		value_node=guid_cast<ValueNode>(guid);
		if(value_node)
		{
			DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node done early\n", __FILE__, __LINE__);
			if(element->get_name()!="canvas" && ValueBase::ident_type(element->get_name()) != type_nil)
			{
				if (element->get_name() == "bone_valuenode")
				{
					ValueNode_Bone::Handle value_node_bone(ValueNode_Bone::Handle::cast_dynamic(value_node));
					if (!value_node_bone)
					{
						DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d bone_valuenode isn't a ValueNode_Bone?  It's a placeholder?\n", __FILE__, __LINE__);
						return value_node;
					}

					return ValueNode_Const::create(ValueBase(value_node_bone));
				}
			}
			return value_node;
		}
	}

	// If ValueBase::ident_type() recognizes the name, then we know it's a ValueBase
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d element name = '%s'\n", __FILE__, __LINE__, element->get_name().c_str());
	if(element->get_name()!="canvas" && ValueBase::ident_type(element->get_name()) != type_nil)
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_value\n", __FILE__, __LINE__);
		ValueBase data=parse_value(element,canvas);

		if(!data.is_valid())
		{
			error(element,strprintf(_("Bad data in <%s>"),element->get_name().c_str()));
			printf("%s:%d parse_value_node done bad data\n", __FILE__, __LINE__);
			return value_node;
		}

		// We want to convert this ValueBase into a
		// ValueNode_Const. That way, we can treat the
		// ID like any other Datanode. Think of this
		// as a shorthand for creating constant ValueNodes.

		value_node=ValueNode_Const::create(data);
	}
	else
	if(element->get_name()=="hermite" || element->get_name()=="animated")
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_animated\n", __FILE__, __LINE__);
		value_node=parse_animated(element,canvas);
	}
	else
	if(element->get_name()=="static_list")
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_static_list\n", __FILE__, __LINE__);
		value_node=parse_static_list(element,canvas);
	}
	else
	if(element->get_name()=="dynamic_list")
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_dynamic_list\n", __FILE__, __LINE__);
		value_node=parse_dynamic_list(element,canvas);
	}
	else
	if(element->get_name()=="bline") // This is not a typo. The dynamic list parser will parse a bline.
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_dynamic_list for bline\n", __FILE__, __LINE__);
		value_node=parse_dynamic_list(element,canvas);
	}
	else
	if(element->get_name()=="wplist") // This is not a typo. The dynamic list parser will parse a wplist.
		value_node=parse_dynamic_list(element,canvas);
	else
	if(element->get_name()=="dilist") // This is not a typo. The dynamic list parser will parse a dilist.
		value_node=parse_dynamic_list(element,canvas);
	else
	if(element->get_name()=="weighted_average" || element->get_name()=="average") // This is not a typo. The dynamic list parser will parse a weighted_average.
		value_node=parse_dynamic_list(element,canvas);
	else
	if(ValueNodeRegistry::book().count(element->get_name()))
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_linkable_value_node\n", __FILE__, __LINE__);
		value_node=parse_linkable_value_node(element,canvas);
		if (!value_node)
		{
			DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_linkable_value_node gave us a null valuenode\n", __FILE__, __LINE__);
			value_node = PlaceholderValueNode::create();
		}
	}
	else
	if(element->get_name()=="canvas")
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node calls parse_canvas\n", __FILE__, __LINE__);
		value_node=ValueNode_Const::create(parse_canvas(element,canvas,true));
	}
	else
	{
		DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node doesn't know what to call\n", __FILE__, __LINE__);
		error_unexpected_element(element,element->get_name());
		error(element, strprintf(_("Expected a ValueNode.  Refer to '%s'"),
								 VALUENODE_COMPATIBILITY_URL));
		value_node=PlaceholderValueNode::create();
	}

	value_node->set_root_canvas(canvas->get_root());

	// If we were successful, and our element has
	// an ID attribute, go ahead and add it to the
	// value_node list
	if(value_node && element->get_attribute("id"))
	{
		std::string id=element->get_attribute("id")->get_value();

		//value_node->set_id(id);

		// If there is already a value_node in the list
		// with the same ID, then that is an error
		try { canvas->add_value_node(value_node,id); }
		catch (Exception::BadLinkName&)
		{
			warning(element,strprintf(_("Bad ID \"%s\""),id.c_str()));
			printf("%s:%d parse_value_node done bad id\n", __FILE__, __LINE__);
			return value_node;
		}
		catch (Exception::IDAlreadyExists&)
		{
			error(element,strprintf(_("Duplicate ID \"%s\""),id.c_str()));
			printf("%s:%d parse_value_node done dup id\n", __FILE__, __LINE__);
			return value_node;
		}
		catch(...)
		{
			error(element,strprintf(_("Unknown Exception thrown when adding ValueNode \"%s\""),id.c_str()));
			throw;
		}
	}
	value_node->set_guid(guid);
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_value_node done\n", __FILE__, __LINE__);
	return value_node;
}

void
CanvasParser::parse_canvas_defs(xmlpp::Element *element,Canvas::Handle canvas)
{
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_canvas_defs\n", __FILE__, __LINE__);
	assert(element->get_name()=="defs");
	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="canvas")
			parse_canvas(child, canvas);
		else
			parse_value_node(child,canvas);
	}
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_canvas_defs done\n", __FILE__, __LINE__);
}

std::list<ValueNode::Handle>
CanvasParser::parse_canvas_bones(xmlpp::Element *element,Canvas::Handle canvas)
{
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_canvas_bones\n", __FILE__, __LINE__);
	assert(element->get_name()=="bones");
	xmlpp::Element::NodeList list = element->get_children();
	std::list<ValueNode::Handle> bone_list;
	in_bones_section = true;
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
			bone_list.push_back(parse_value_node(child,canvas));
	}
	in_bones_section = false;
	DEBUG_LOG("SYNFIG_DEBUG_LOAD_CANVAS", "%s:%d parse_canvas_bones done\n", __FILE__, __LINE__);
	return bone_list;
}

Layer::Handle
CanvasParser::parse_layer(xmlpp::Element *element,Canvas::Handle canvas)
{

	assert(element->get_name()=="layer");
	Layer::Handle layer;

	if(!element->get_attribute("type"))
	{
		error(element,_("Missing \"type\" attribute to \"layer\" element"));
		return Layer::Handle();
	}
	if(element->get_attribute("type")->get_value() == "filled_rectangle")
	layer=Layer::create("rectangle");
	else layer=Layer::create(element->get_attribute("type")->get_value());
	layer->set_canvas(canvas);

	if(element->get_attribute("group"))
	{
		layer->add_to_group(
			element->get_attribute("group")->get_value()
		);
	}

	// Handle the version attribute
	String version;
	if(element->get_attribute("version"))
	{
		version = element->get_attribute("version")->get_value();
		if(version>layer->get_version())
			warning(element,_("Installed layer version is smaller than layer version in file"));
		if(version!=layer->get_version())
			layer->set_version(version);
	}

	// Handle the description
	if(element->get_attribute("desc"))
		layer->set_description(element->get_attribute("desc")->get_value());

	if(element->get_attribute("active"))
		layer->set_active(!is_false(element->get_attribute("active")->get_value()));

	if(element->get_attribute("exclude_from_rendering"))
		layer->set_exclude_from_rendering(!is_false(element->get_attribute("exclude_from_rendering")->get_value()));

	// Load old groups
	Layer_PasteCanvas::Handle layer_pastecanvas = etl::handle<Layer_Group>::cast_dynamic(layer);
	bool old_pastecanvas = layer_pastecanvas && version=="0.1";
	ValueNode::Handle origin_node;
	ValueNode_Composite::Handle transformation_node;
	ValueNode_Add::Handle offset_node;
	ValueNode_Scale::Handle scale_scalar_node;
	ValueNode_Exp::Handle scale_node;
	bool origin_const=true, focus_const=true, zoom_const=true;
	if (old_pastecanvas) {
		transformation_node = ValueNode_Composite::create(ValueBase(Transformation()), canvas);
		layer->connect_dynamic_param("transformation", ValueNode::Handle(transformation_node));

		offset_node = ValueNode_Add::create(ValueBase(Vector(0,0)));
		transformation_node->set_link("offset", offset_node);

		origin_node = offset_node->get_link("rhs");
		layer->connect_dynamic_param("origin", ValueNode::Handle(origin_node));

		scale_scalar_node = ValueNode_Scale::create(ValueBase(Vector(1,1)));
		transformation_node->set_link("scale", scale_scalar_node);

		scale_node = ValueNode_Exp::create(ValueBase(Real(1)));
		scale_scalar_node->set_link("scalar", scale_node);
	}

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="name")
			warning(child,_("<name> entry for <layer> is not yet supported. Ignoring..."));
		else
		if(child->get_name()=="desc")
			warning(child,_("<desc> entry for <layer> is not yet supported. Ignoring..."));
		else
		if(child->get_name()=="param")
		{
			xmlpp::Element::NodeList list = child->get_children();

			if(!child->get_attribute("name"))
			{
				error(child,_("Missing \"name\" attribute for <param>."));
				continue;
			}

			String param_name=child->get_attribute("name")->get_value();

			// SVN r2013 and r2014 renamed all 'pos' and 'offset' parameters to 'origin'
			// 'pos' and 'offset' will appear in old .sif files; handle them correctly
			if (param_name == "pos" || param_name == "offset")
				param_name = "origin";

			if(child->get_attribute("use"))
			{
				// If the "use" attribute is used, then the
				// element should be empty. Warn the user if
				// we find otherwise.
				if(!list.empty())
					warning(child,_("Found \"use\" attribute for <param>, but it wasn't empty. Ignoring contents..."));

				String str=	child->get_attribute("use")->get_value();

				if (str.empty())
					error(child,_("Empty use=\"\" value in <param>"));
				else if(layer->get_param(param_name).get_type()==type_canvas)
				{
					String warnings;
					Canvas::Handle c(canvas->surefind_canvas(str, warnings));
					warnings_text += warnings;
					if(!c) error((*iter),strprintf(_("Failed to load subcanvas '%s'"), str.c_str()));
					if(!layer->set_param(param_name,c))
						error((*iter),_("Layer rejected canvas link"));
					//Parse the static option and sets it to the canvas ValueBase
					ValueBase v=layer->get_param(param_name);
					v.set_static(parse_static(child));
					layer->set_param(param_name, v);
				}
				else
				try
				{
					ValueNode::Handle value_node=canvas->surefind_value_node(str);
					if(PlaceholderValueNode::Handle::cast_dynamic(value_node))
						throw Exception::IDNotFound("parse_layer()");

					// Assign the value_node to the dynamic parameter list
					if (param_name == "segment_list" && (layer->get_name() == "region" || layer->get_name() == "outline"))
					{
						synfig::warning("%s: Updated valuenode connection to use the \"bline\" parameter instead of \"segment_list\".",
										layer->get_name().c_str());
						param_name = "bline";
					}

					// NB: this part of code has copy below
					bool processed = false;
					if (old_pastecanvas)
					{
						processed = true;
						if (param_name == "origin")
						{
							origin_const = false;
							offset_node->set_link("lhs", value_node);
						}
						else
						if (param_name == "focus")
						{
							focus_const = false;
							origin_node = value_node;
							layer->connect_dynamic_param("origin_node", ValueNode::Handle(origin_node));
							offset_node->set_link("rhs", value_node);
						}
						else
						if (param_name == "zoom")
						{
							zoom_const = false;
							scale_node->set_link("exp", value_node);
						}
						else
							processed = false;
					}

					if (!processed) layer->connect_dynamic_param(param_name,value_node);
    			}
				catch(Exception::IDNotFound&)
				{
					error(child,strprintf(_("Unknown ID (%s) referenced in parameter \"%s\""),str.c_str(), param_name.c_str()));
				}

				continue;
			}

			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter))
					break;
				//if(!(!dynamic_cast<xmlpp::Element*>(*iter) && (*iter)->get_name()=="text"||(*iter)->get_name()=="comment"   )) break;

			if(iter==list.end())
			{
				error(child,_("<param> is either missing its contents, or missing a \"use\" attribute."));
				continue;
			}

			ValueBase data;
			ValueNode::Handle value_node;

			// If we recognize the element name as a
			// ValueBase, then treat is at one
			if(/*(*iter)->get_name()!="canvas" && */ValueBase::ident_type((*iter)->get_name()) != type_nil && !dynamic_cast<xmlpp::Element*>(*iter)->get_attribute("guid"))
			{
				data=parse_value(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!data.is_valid())
				{
					error((*iter),_("Bad data for <param>"));
					continue;
				}
			}
			else	// ... otherwise, we assume that it is a ValueNode
			{
				value_node=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!value_node)
				{
					error((*iter),_("Bad data for <param>"));
					continue;
				}
			}

			// NB: this part of code has copy above
			bool processed = false;
			if (old_pastecanvas)
			{
				processed = true;
				bool is_const = !value_node;
				ValueNode::Handle node = value_node ? value_node : ValueNode_Const::create(data,canvas);
				if (param_name == "origin")
				{
					// ice0: check here 
					if (!is_const) origin_const = false;
					offset_node->set_link("lhs", node);
				}
				else
				if (param_name == "focus")
				{
					if (!is_const) focus_const = false;
					origin_node = node;
					layer->connect_dynamic_param("origin_node", ValueNode::Handle(origin_node));
					offset_node->set_link("rhs", node);
				}
				else
				if (param_name == "zoom")
				{
					if (!is_const) zoom_const = false;
					scale_node->set_link("exp", node);
				}
				else
					processed = false;
			}

			if (!processed)
			{
				if (value_node) {
					// Assign the value_node to the dynamic parameter list
					layer->connect_dynamic_param(param_name,value_node);
				} else {
					// Set the layer's parameter, and make sure that
					// the layer linked it
					if(!layer->set_param(param_name,data))
					{
						// TODO(ice0): Add normal version comparison function (check glib)
						// TODO(ice0): Remove stubs after updating image files (.sif)
						if (param_name == "loopyness" && layer->get_name() == "outline" && (layer->get_version() == "0.3")) {
							continue;
						}

						if (param_name == "falloff" && layer->get_name() == "circle" && (layer->get_version() == "0.2")) {
							continue;
						}

						if (param_name == "fast" && layer->get_name() == "advanced_outline" && (layer->get_version() == "0.3")) {
							continue;
						}

						if (param_name == "enable_transformation" && layer->get_name() == "group" && (layer->get_version() == "0.3")) {
							continue;
						}


						warning((*iter),strprintf(_("Layer '%s' rejected value for parameter '%s'"),
												  element->get_attribute("type")->get_value().c_str(),
												  param_name.c_str()));
						continue;
					}
				}
			}

			// Warn if there is trash after the param value
			for (++iter; iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter))
					warning((*iter),strprintf(_("Unexpected element <%s> after <param> data, ignoring..."),(*iter)->get_name().c_str()));
			continue;
		}
		else
		{
			printf("%s:%d\n", __FILE__, __LINE__);
			error_unexpected_element(child,child->get_name());
		}
	}

	// Simplify old pastecanvas conversion
	if (old_pastecanvas) {
		bool focus_zero = focus_const && (*origin_node)(0).get(Vector()) == Vector(0,0);
		bool zoom_zero = zoom_const && (*scale_node->get_link("exp"))(0).get(Real()) == 0;
		if (origin_const && focus_const && zoom_const)
		{
			ValueBase origin = (*origin_node)(0);
			transformation_node->set_link("offset", ValueNode_Const::create((*offset_node)(0), canvas));
			transformation_node->set_link("scale", ValueNode_Const::create((*scale_scalar_node)(0), canvas));
			layer->disconnect_dynamic_param("origin");
			layer->set_param("origin", origin);
		} else {
			if (origin_const && focus_const)
			{
				ValueBase origin = (*origin_node)(0);
				layer->disconnect_dynamic_param("origin");
				layer->set_param("origin", origin);
				transformation_node->set_link("offset", ValueNode_Const::create((*offset_node)(0), canvas));
			} else
			if (focus_zero)
			{
				layer->disconnect_dynamic_param("origin");
				transformation_node->set_link("offset", offset_node->get_link("lhs"));
			}
			else
			if (focus_const)
			{
				ValueBase origin = (*origin_node)(0);
				layer->disconnect_dynamic_param("origin");
				layer->set_param("origin", origin);
			}

			if (zoom_zero)
				transformation_node->set_link("scale", ValueNode_Const::create(ValueBase(Vector(1,1)), canvas));
			else
			if (zoom_const)
				transformation_node->set_link("scale", ValueNode_Const::create((*scale_scalar_node)(0), canvas));
		}
	}

	// add amplifiers for blur
	if (layer->get_name() == "blur" && (version == "0.0" || version == "0.1" || version == "0.2"))
	{
		if (layer->dynamic_param_list().count("type"))
		{
			warning(element, "Cannot apply amplifiers to layer blur with animated type");
		}
		if (layer->get_param("type").get(int()) == ::Blur::GAUSSIAN)
		{
			warning(element, "Cannot apply amplifiers to layer blur with type GAUSSIAN");
		}
		else
		{
			int type = layer->get_param("type").get(int());
			Real amplifier = 1.0 / ::Blur::get_size_amplifier(type);
			if (layer->dynamic_param_list().count("size") && layer->dynamic_param_list().find("size")->second)
			{
				ValueNode_Scale::Handle scale = ValueNode_Scale::create(layer->get_param("size"));
				scale->set_link("link", ValueNode::Handle(layer->dynamic_param_list().find("size")->second));
				scale->set_link("scalar", ValueNode_Const::create(amplifier));
				layer->connect_dynamic_param("size", ValueNode::LooseHandle(scale));
			}
			else
			{
				layer->set_param("size", layer->get_param("size").get(Vector()) * amplifier);
			}
		}
	}

	// init blending for skeleton_deformation
	if (layer->get_name() == "skeleton_deformation" && (version == "0.0" || version == "0.1"))
	{
		layer->disconnect_dynamic_param("amount");
		layer->disconnect_dynamic_param("blend_method");
		layer->set_param("amount", 1.0);
		layer->set_param("blend_method", Color::BLEND_STRAIGHT);
	}

	layer->reset_version();
	return layer;
}

Canvas::Handle
CanvasParser::parse_canvas(xmlpp::Element *element,Canvas::Handle parent,bool inline_,const FileSystem::Identifier &identifier,String filename)
{

	if(element->get_name()!="canvas")
	{
		error_unexpected_element(element,element->get_name(),"canvas");
		return Canvas::Handle();
	}
	Canvas::Handle canvas;



	if(parent && (element->get_attribute("id") || inline_))
	{
		if(inline_)
		{
			canvas=Canvas::create_inline(parent);
		}
		else
		{
			try
			{
				String warnings;
				canvas=parent->find_canvas(element->get_attribute("id")->get_value(), warnings);
				warnings_text += warnings;
			}
			catch(...)
			{
				canvas=parent->new_child_canvas(element->get_attribute("id")->get_value());
			}
		}
		canvas->rend_desc().clear_flags();
	}
	else
	{
		canvas=Canvas::create();
		canvas->set_identifier(identifier);
		if(filename=="/dev/stdin")
			canvas->set_file_name("./stdin.sif");
		else
			canvas->set_file_name(filename);
		canvas->rend_desc().clear_flags();
	}

	if(element->get_attribute("guid"))
	{
		GUID guid(element->get_attribute("guid")->get_value());
		if(guid_cast<Canvas>(guid))
			return guid_cast<Canvas>(guid);
		else
			canvas->set_guid(guid);
	}

	if(element->get_attribute("version"))
		canvas->set_version(element->get_attribute("version")->get_value());
	else if(parent)
		canvas->set_version(parent->get_version());

	if(element->get_attribute("width"))
	{
		int width = atoi(element->get_attribute("width")->get_value().c_str());
		if (width < 1)
			fatal_error(element, _("Canvas with width or height less than one is not allowed"));
		canvas->rend_desc().set_w(width);
	}

	if(element->get_attribute("height"))
	{
		int height = atoi(element->get_attribute("height")->get_value().c_str());
		if (height < 1)
			fatal_error(element, _("Canvas with width or height less than one is not allowed"));
		canvas->rend_desc().set_h(height);
	}

	if(element->get_attribute("xres"))
		canvas->rend_desc().set_x_res(atof(element->get_attribute("xres")->get_value().c_str()));

	if(element->get_attribute("yres"))
		canvas->rend_desc().set_y_res(atof(element->get_attribute("yres")->get_value().c_str()));

	Gamma gamma = canvas->rend_desc().get_gamma();
	String version = canvas->get_version();
	if ( version == "1.0" || (version[0] == '0' && version[1] == '.') )
	{
		gamma.set(2.2);
		// Synfig 1.4.0 works differently with looped outlines.
		// So we give user a warning when he opens old files.
		// See https://github.com/synfig/synfig/issues/1307
		if ( canvas->is_root()) {
			warnings_text += _("You're opening a file created in an older version of Synfig.\n"
					"If you save this file with the current version, "
					"it might not open correctly in an older version of Synfig anymore.");
		}
	}
	if(element->get_attribute("gamma-r"))
		gamma.set_r(atof(element->get_attribute("gamma-r")->get_value().c_str()));
	if(element->get_attribute("gamma-g"))
		gamma.set_g(atof(element->get_attribute("gamma-g")->get_value().c_str()));
	if(element->get_attribute("gamma-b"))
		gamma.set_b(atof(element->get_attribute("gamma-b")->get_value().c_str()));
	canvas->rend_desc().set_gamma(gamma);

	if(element->get_attribute("fps"))
		canvas->rend_desc().set_frame_rate(atof(element->get_attribute("fps")->get_value().c_str()));

	if(element->get_attribute("start-time"))
		canvas->rend_desc().set_time_start(Time(element->get_attribute("start-time")->get_value(),canvas->rend_desc().get_frame_rate()));

	if(element->get_attribute("begin-time"))
		canvas->rend_desc().set_time_start(Time(element->get_attribute("begin-time")->get_value(),canvas->rend_desc().get_frame_rate()));

	if(element->get_attribute("end-time"))
		canvas->rend_desc().set_time_end(Time(element->get_attribute("end-time")->get_value(),canvas->rend_desc().get_frame_rate()));

	if(element->get_attribute("antialias"))
		canvas->rend_desc().set_antialias(atoi(element->get_attribute("antialias")->get_value().c_str()));

	if(element->get_attribute("view-box"))
	{
		std::string values=element->get_attribute("view-box")->get_value();
		Vector
			tl,
			br;
		tl[0]=atof(std::string(values.data(),values.find(' ')).c_str());
		values=std::string(values.begin()+values.find(' ')+1,values.end());
		tl[1]=atof(std::string(values.data(),values.find(' ')).c_str());
		values=std::string(values.begin()+values.find(' ')+1,values.end());
		br[0]=atof(std::string(values.data(),values.find(' ')).c_str());
		values=std::string(values.begin()+values.find(' ')+1,values.end());
		br[1]=atof(values.c_str());

		canvas->rend_desc().set_tl(tl);
		canvas->rend_desc().set_br(br);
	}

	if(element->get_attribute("bgcolor"))
	{
		std::string values=element->get_attribute("bgcolor")->get_value();
		Color bg;

		bg.set_r(atof(std::string(values.data(),values.find(' ')).c_str()));
		values=std::string(values.begin()+values.find(' ')+1,values.end());

		bg.set_g(atof(std::string(values.data(),values.find(' ')).c_str()));
		values=std::string(values.begin()+values.find(' ')+1,values.end());

		bg.set_b(atof(std::string(values.data(),values.find(' ')).c_str()));
		values=std::string(values.begin()+values.find(' ')+1,values.end());

		bg.set_a(atof(values.c_str()));

		canvas->rend_desc().set_bg_color(bg);
	}

	if(element->get_attribute("focus"))
	{
		std::string values = trim(element->get_attribute("focus")->get_value());
		const auto separator_pos = values.find(' ');
		Vector focus;
		if (separator_pos != std::string::npos) {
			focus[0] = atof(values.substr(0, separator_pos).c_str());
			focus[1] = atof(values.substr(separator_pos + 1).c_str());
		}

		canvas->rend_desc().set_focus(focus);
	}

	canvas->rend_desc().set_flags(RendDesc::PX_ASPECT|RendDesc::IM_SPAN);

	std::list<ValueNode::Handle> bone_list;
	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(child)
		{
			if(child->get_name()=="defs")
			{
				if(canvas->is_inline())
					error(child,_("Group canvases cannot have a <defs> section"));
				parse_canvas_defs(child, canvas);
			}
			else
			if(child->get_name()=="bones")
			{
				if(canvas->is_inline())
					error(child,_("Inline canvas cannot have a <bones> section"));
				bone_list = parse_canvas_bones(child, canvas);
			}
			else
			if(child->get_name()=="keyframe")
			{
				if(canvas->is_inline())
				{
					warning(child,_("Group canvases cannot have keyframes"));
					continue;
				}

				canvas->keyframe_list().add(parse_keyframe(child,canvas));
				canvas->keyframe_list().sync();
			}
			else
			if(child->get_name()=="meta")
			{
				if(canvas->is_inline())
				{
					warning(child,_("Group canvases cannot have metadata"));
					continue;
				}

				if(!child->get_attribute("name"))
				{
					warning(child,_("<meta> must have a name"));
					continue;
				}

				if(!child->get_attribute("content"))
				{
					warning(child,_("<meta> must have content"));
					continue;
				}
				
				// In Synfig prior to version 1.0 we have messed decimal separator:
				// some files use ".", but other ones use ","/
				// Let's try to put a workaround for that.
				std::vector<String> replacelist;
				replacelist.push_back("background_first_color");
				replacelist.push_back("background_second_color");
				replacelist.push_back("background_size");
				replacelist.push_back("grid_color");
				replacelist.push_back("grid_size");
				replacelist.push_back("jack_offset");
				String content;
				content=child->get_attribute("content")->get_value();
				if(std::find(replacelist.begin(), replacelist.end(), child->get_attribute("name")->get_value()) != replacelist.end()) 
				{
					size_t index = 0;
					while (true) {
					     /* Locate the substring to replace. */
					     index = content.find(',', index);
					     if (index == std::string::npos) break;

					     /* Make the replacement. */
					     content.replace(index, 1, ".");

					     /* Advance index forward so the next iteration doesn't pick it up as well. */
					     index += 1;
					}
					
				}
				canvas->set_meta_data(child->get_attribute("name")->get_value(),content);
			}
			else if(child->get_name()=="name")
			{
				xmlpp::Element::NodeList list = child->get_children();

				// If we don't have any name, warn
				if(list.empty())
					warning(child,_("blank \"name\" entity"));

				std::string tmp;
				for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::TextNode*>(*iter))tmp+=dynamic_cast<xmlpp::TextNode*>(*iter)->get_content();
				canvas->set_name(tmp);
			}
			else
			if(child->get_name()=="desc")
			{

				xmlpp::Element::NodeList list = child->get_children();

				// If we don't have any description, warn
				if(list.empty())
					warning(child,_("blank \"desc\" entity"));

				std::string tmp;
				for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::TextNode*>(*iter))tmp+=dynamic_cast<xmlpp::TextNode*>(*iter)->get_content();
				canvas->set_description(tmp);
			}
			else
			if(child->get_name()=="author")
			{

				xmlpp::Element::NodeList list = child->get_children();

				// If we don't have any description, warn
				if(list.empty())
					warning(child,_("blank \"author\" entity"));

				std::string tmp;
				for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::TextNode*>(*iter))tmp+=dynamic_cast<xmlpp::TextNode*>(*iter)->get_content();
				canvas->set_author(tmp);
			}
			else
			if(child->get_name()=="layer")
			{
				//if(canvas->is_inline())
				//	canvas->push_front(parse_layer(child,canvas->parent()));
				//else
					canvas->push_front(parse_layer(child,canvas));
			}
			else
			{
				printf("%s:%d\n", __FILE__, __LINE__);
				error_unexpected_element(child,child->get_name());
			}
		}
//		else
//		if((child->get_name()=="text"||child->get_name()=="comment") && child->has_child_text())
//			continue;
	}

	if(canvas->value_node_list().placeholder_count())
	{
		String nodes;
		for (ValueNodeList::const_iterator iter = canvas->value_node_list().begin(); iter != canvas->value_node_list().end(); ++iter) {
			if(PlaceholderValueNode::Handle::cast_dynamic(*iter))
			{
				if (nodes != "") nodes += ", ";
				nodes += "'" + (*iter)->get_id() + "'";
			}
		}
		error(element,strprintf(_("Canvas '%s' has undefined %s: %s"),
								canvas->get_id().c_str(),
								canvas->value_node_list().placeholder_count() == 1 ? _("ValueNode") : _("ValueNodes"),
								nodes.c_str()));
	}

	canvas->set_version(CURRENT_CANVAS_VERSION);
	return canvas;
}

void
CanvasParser::register_canvas_in_map(Canvas::Handle canvas, String as)
{
	get_open_canvas_map()[canvas.get()]=filesystem::Path::absolute_path(as);
	canvas->signal_deleted().connect(sigc::bind(sigc::ptr_fun(_remove_from_open_canvas_map),canvas.get()));
	canvas->signal_file_name_changed().connect(sigc::bind(sigc::ptr_fun(_canvas_file_name_changed),canvas.get()));
}

#ifdef _DEBUG
void
CanvasParser::show_canvas_map(String file, int line, String text)
{
	return;
	printf("  .-----\n  |  %s:%d %s\n", file.c_str(), line, text.c_str());
	const auto& canvas_map = synfig::get_open_canvas_map();
	for (const auto& iter : canvas_map)
	{
		printf("  |    %40s : %lx (%d)\n", iter.second.c_str(), uintptr_t(iter.first.get()), iter.first->count());
	}
	printf("  `-----\n\n");
}
#endif	// _DEBUG

Canvas::Handle
CanvasParser::parse_from_file_as(const FileSystem::Identifier &identifier,const String &as,String &errors)
{
	ChangeLocale change_locale(LC_NUMERIC, "C");

	try
	{
		const std::string absolute_path = filesystem::Path::absolute_path(as);
		for (const auto& it : get_open_canvas_map()) {
			if (it.second == absolute_path) {
				return it.first;
			}
		}

		filename=as;
		total_warnings_=0;
		
		synfig::info(String("Loading file: ") + filename);
		FileSystem::ReadStream::Handle stream = identifier.get_read_stream();
		if (stream)
		{
			if (identifier.filename.extension().u8string() == ".sifz")
				stream = FileSystem::ReadStream::Handle(new ZReadStream(stream, zstreambuf::compression::gzip));

			xmlpp::DomParser parser;
			parser.parse_stream(*stream);
			stream.reset();
			if(parser)
			{
				Canvas::Handle canvas(parse_canvas(parser.get_document()->get_root_node(),0,false,identifier,as));
				if (!canvas) return canvas;
				register_canvas_in_map(canvas, as);

				const ValueNodeList& value_node_list(canvas->value_node_list());

				again:
				ValueNodeList::const_iterator iter;
				for(iter=value_node_list.begin();iter!=value_node_list.end();++iter)
				{
					ValueNode::Handle value_node(*iter);
					if(value_node->is_exported() && value_node->get_id().find("Unnamed")==0)
					{
						canvas->remove_value_node(value_node, true);
						goto again;
					}
				}

				return canvas;
			}
		} else {
			throw std::runtime_error(String("  * ") + _("Can't find linked file") + " \"" + identifier.filename.u8string() + "\"");
		}
	}
	catch(Exception::BadLinkName&) { synfig::error("BadLinkName Thrown"); }
	catch(Exception::BadType&) { synfig::error("BadType Thrown"); }
	catch(Exception::FileNotFound&) { synfig::error("FileNotFound Thrown"); }
	catch(Exception::IDNotFound&) { synfig::error("IDNotFound Thrown"); }
	catch(Exception::IDAlreadyExists&) { synfig::error("IDAlreadyExists Thrown"); }
	catch(xmlpp::internal_error &x)
	{
		if (!strcmp(x.what(), "Couldn't create parsing context"))
			throw std::runtime_error(String("  * ") + _("Can't open file") + " \"" + identifier.filename.u8string() + "\"");
		throw;
	}
	catch(const std::exception& ex)
	{
		synfig::error("Standard Exception: "+String(ex.what()));
		errors = ex.what();
		return Canvas::Handle();
	}
	catch(const String& str)
	{
		std::cerr<<str.c_str()<<std::endl;
		//	synfig::error(str);
		errors = str;
		return Canvas::Handle();
	}
	return Canvas::Handle();
}

Canvas::Handle
CanvasParser::parse_as(xmlpp::Element* node,String &errors)
{
	ChangeLocale change_locale(LC_NUMERIC, "C");
	try
	{
		total_warnings_=0;
		if(node)
		{
			Canvas::Handle canvas(parse_canvas(node,0,false,FileSystemNative::instance()->get_identifier(std::string()),""));
			if (!canvas) return canvas;

			const ValueNodeList& value_node_list(canvas->value_node_list());

			again:
			ValueNodeList::const_iterator iter;
			for(iter=value_node_list.begin();iter!=value_node_list.end();++iter)
			{
				ValueNode::Handle value_node(*iter);
				if(value_node->is_exported() && value_node->get_id().find("Unnamed")==0)
				{
					canvas->remove_value_node(value_node, false); // \todo verify false here
					goto again;
				}
			}

			return canvas;
		}
	}
	catch(Exception::BadLinkName&) { synfig::error("BadLinkName Thrown"); }
	catch(Exception::BadType&) { synfig::error("BadType Thrown"); }
	catch(Exception::FileNotFound&) { synfig::error("FileNotFound Thrown"); }
	catch(Exception::IDNotFound&) { synfig::error("IDNotFound Thrown"); }
	catch(Exception::IDAlreadyExists&) { synfig::error("IDAlreadyExists Thrown"); }
	catch(xmlpp::internal_error &x)
	{
		if (!strcmp(x.what(), "Couldn't create parsing context"))
			throw std::runtime_error(String("  * ") + _("Can't open file") + " \"" + "\"");
		throw;
	}
	catch(const std::exception& ex)
	{
		synfig::error("Standard Exception: "+String(ex.what()));
		errors = ex.what();
		return Canvas::Handle();
	}
	catch(const String& str)
	{
		std::cerr<<str.c_str()<<std::endl;
		errors = str;
		return Canvas::Handle();
	}
	return Canvas::Handle();
}
//extern
Canvas::Handle
synfig::open_canvas(xmlpp::Element* node,String &errors,String &warnings){
	Canvas::Handle canvas;
	CanvasParser parser;
	parser.set_allow_errors(true);
	try
	{
		canvas=parser.parse_as(node,errors);
	}
	catch (...)
	{
		synfig::error("open canvas of a node");
		throw;
	}

	warnings = parser.get_warnings_text();

	if(parser.error_count())
	{
		errors = parser.get_errors_text();
		return Canvas::Handle();
	}
	return canvas;
}


