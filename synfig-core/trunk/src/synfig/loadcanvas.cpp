/* === S Y N F I G ========================================================= */
/*!	\file loadcanvas.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ETL/stringf>
#include <libxml++/libxml++.h>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "loadcanvas.h"
#include "valuenode.h"
#include "valuenode_subtract.h"
#include "valuenode_animated.h"
#include "valuenode_composite.h"
#include "valuenode_const.h"
#include "valuenode_linear.h"
#include "valuenode_dynamiclist.h"
#include "valuenode_reference.h"
#include "valuenode_scale.h"
#include "valuenode_timedswap.h"
#include "valuenode_twotone.h"
#include "valuenode_stripes.h"
#include "valuenode_segcalctangent.h"
#include "valuenode_segcalcvertex.h"
#include "valuenode_bline.h"

#include "layer.h"
#include "string.h"

#include "exception.h"

#include "gradient.h"

#include <map>
#include <sigc++/bind.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;
using namespace etl;

/*
class test_class {
static int bleh;
public:
	test_class() { assert(!bleh); bleh++; synfig::info("test_class: initi: %d",bleh); }
	~test_class() { assert(bleh); synfig::info("test_class: uninit: %d",bleh); bleh--; }
};
int test_class::bleh(0);

test_class test_class_instance;
*/

/* === M A C R O S ========================================================= */

inline bool is_whitespace(char x) { return ((x)=='\n' || (x)=='\t' || (x)==' '); }

/* === P R O C E D U R E S ================================================= */

static std::map<String, Canvas::LooseHandle>* open_canvas_map_(0);

std::map<synfig::String, etl::loose_handle<Canvas> >& synfig::get_open_canvas_map()
{
	if(!open_canvas_map_)
		open_canvas_map_=new std::map<String, Canvas::LooseHandle>;
	return *open_canvas_map_;
}

static void _remove_from_open_canvas_map(Canvas *x) { get_open_canvas_map().erase(etl::absolute_path(x->get_file_name())); }

static void _canvas_file_name_changed(Canvas *x)
{
	std::map<synfig::String, etl::loose_handle<Canvas> >::iterator iter;

	for(iter=get_open_canvas_map().begin();iter!=get_open_canvas_map().end();++iter)
		if(iter->second==x)
			break;
	assert(iter!=get_open_canvas_map().end());
	if(iter==get_open_canvas_map().end())
		return;
	get_open_canvas_map().erase(iter->first);
	get_open_canvas_map()[etl::absolute_path(x->get_file_name())]=x;

}

Canvas::Handle
synfig::open_canvas(const String &filename)
{
	CanvasParser parser;

	parser.set_allow_errors(true);

	Canvas::Handle canvas=parser.parse_from_file(filename);

	if(parser.error_count())
		return Canvas::Handle();

	return canvas;
}

Canvas::Handle
synfig::open_canvas_as(const String &filename,const String &as)
{
	CanvasParser parser;

	parser.set_allow_errors(true);

	Canvas::Handle canvas=parser.parse_from_file_as(filename,as);

	if(parser.error_count())
		return Canvas::Handle();

	return canvas;
}

Canvas::Handle
synfig::string_to_canvas(const String &data)
{
	CanvasParser parser;

	parser.set_allow_errors(true);

	Canvas::Handle canvas=parser.parse_from_string(data);

	if(parser.error_count())
		return Canvas::Handle();

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
	string str=strprintf("%s:<%s>:%d: warning: ",filename.c_str(),element->get_name().c_str(),element->get_line())+text;
	//synfig::warning(str);
	cerr<<str<<endl;
	total_warnings_++;
	if(total_warnings_>=max_warnings_)
		fatal_error(element, _("Too many warnings"));
}

void
CanvasParser::error(xmlpp::Node *element, const String &text)
{
	string str=strprintf("%s:<%s>:%d: error: ",filename.c_str(),element->get_name().c_str(),element->get_line())+text;
	total_errors_++;
	if(!allow_errors_)
		throw runtime_error(str);
	cerr<<str<<endl;
	//	synfig::error(str);
}

void
CanvasParser::fatal_error(xmlpp::Node *element, const String &text)
{
	string str=strprintf("%s:<%s>:%d:",filename.c_str(),element->get_name().c_str(),element->get_line())+text;
	throw runtime_error(str);
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


	if(element->get_children().empty())
		return ret;

	if(element->get_child_text()->get_content().empty())
		return ret;

	ret.set_description(element->get_child_text()->get_content());

	return ret;
}


Real
CanvasParser::parse_real(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="real");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"real"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"real"));
		return false;
	}

	string val=element->get_attribute("value")->get_value();

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

	string val=element->get_attribute("value")->get_value();

	return Time(val,canvas->rend_desc().get_frame_rate());
}

int
CanvasParser::parse_integer(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="integer");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"integer"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"integer"));
		return false;
	}

	string val=element->get_attribute("value")->get_value();

	return atoi(val.c_str());
}

Vector
CanvasParser::parse_vector(xmlpp::Element *element,Canvas::Handle canvas)
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
			error_unexpected_element(child,child->get_name());
	}
	return vect;
}

Color
CanvasParser::parse_color(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="color");

	if(element->get_children().empty())
	{
		error(element, "Undefined value in <color>");
		return Color();
	}

	Color color;

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
			error_unexpected_element(child,child->get_name());
	}

	return color;
}

synfig::String
CanvasParser::parse_string(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="string");

	if(element->get_children().empty())
	{
		warning(element, "Undefined value in <string>");
		return synfig::String();
	}

	if(element->get_child_text()->get_content().empty())
	{
		warning(element, "Content element of <string> appears to be empty");
		return synfig::String();
	}

	return element->get_child_text()->get_content();
}

bool
CanvasParser::parse_bool(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="bool");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"bool"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"bool"));
		return false;
	}

	string val=element->get_attribute("value")->get_value();

	if(val=="true" || val=="1")
		return true;
	if(val=="false" || val=="0")
		return false;

	error(element,strprintf(_("Bad value \"%s\" in <%s>"),val.c_str(),"bool"));

	return false;
}

Gradient
CanvasParser::parse_gradient(xmlpp::Element *node,Canvas::Handle canvas)
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
			cpoint.color=parse_color(child,canvas);

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
	vector<ValueBase> value_list;

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
CanvasParser::parse_segment(xmlpp::Element *element,Canvas::Handle canvas)
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

			seg.p1=parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas);
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

			seg.t1=parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas);
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

			seg.p2=parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas);
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

			seg.t2=parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas);
		}
		else
			error_unexpected_element(child,child->get_name());
	}
	return seg;
}

BLinePoint
CanvasParser::parse_bline_point(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="bline_point");
	if(element->get_children().empty())
	{
		error(element, "Undefined value in <bline_point>");
		return BLinePoint();
	}

	BLinePoint ret;
	ret.set_split_tangent_flag(false);

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

			ret.set_vertex(parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas));
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

			ret.set_tangent1(parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas));
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

			ret.set_tangent2(parse_vector(dynamic_cast<xmlpp::Element*>(*iter),canvas));
			ret.set_split_tangent_flag(true);
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

			ret.set_width(parse_real(dynamic_cast<xmlpp::Element*>(*iter),canvas));
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

			ret.set_origin(parse_real(dynamic_cast<xmlpp::Element*>(*iter),canvas));
		}
		else
			error_unexpected_element(child,child->get_name());
	}
	return ret;
}

Angle
CanvasParser::parse_angle(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="angle");

	if(!element->get_children().empty())
		warning(element, strprintf(_("<%s> should not contain anything"),"angle"));

	if(!element->get_attribute("value"))
	{
		error(element,strprintf(_("<%s> is missing \"value\" attribute"),"angle"));
		return Angle();
	}

	string val=element->get_attribute("value")->get_value();

	return Angle::deg(atof(val.c_str()));
}

ValueBase
CanvasParser::parse_value(xmlpp::Element *element,Canvas::Handle canvas)
{
	if(element->get_name()=="real")
		return parse_real(element,canvas);
	else
	if(element->get_name()=="time")
		return parse_time(element,canvas);
	else
	if(element->get_name()=="integer")
		return parse_integer(element,canvas);
	else
	if(element->get_name()=="string")
		return parse_string(element,canvas);
	else
	if(element->get_name()=="vector")
	{
		return parse_vector(element,canvas);
	}
	else
	if(element->get_name()=="color")
		return parse_color(element,canvas);
	else
	if(element->get_name()=="segment")
		return parse_segment(element,canvas);
	else
	if(element->get_name()=="list")
		return parse_list(element,canvas);
	else
	if(element->get_name()=="gradient")
		return parse_gradient(element,canvas);
	else
	if(element->get_name()=="bool")
		return parse_bool(element,canvas);
	else
	//if(element->get_name()=="canvas")
	//	return parse_canvas(element,canvas,true);	// inline canvas
	//else
	if(element->get_name()=="angle" || element->get_name()=="degrees" || element->get_name()=="radians" || element->get_name()=="rotations")
		return parse_angle(element,canvas);
	else
	if(element->get_name()=="bline_point")
		return parse_bline_point(element,canvas);
	else
	if(element->get_name()=="canvas")
		return ValueBase(parse_canvas(element,canvas,true));
	else
	{
		DEBUGPOINT();
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

	ValueBase::Type type=ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(!type)
	{
		error(element,"Bad type in <animated>");
		return ValueNode_Animated::Handle();
	}

	ValueNode_Animated::Handle value_node=ValueNode_Animated::create(type);

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <animated> with type \"%s\""),ValueBase::type_name(type).c_str()));
		return ValueNode_Animated::Handle();
	}

	value_node->set_root_canvas(canvas->get_root());

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
				if (type==ValueBase::TYPE_CANVAS)
					waypoint_value_node=ValueNode_Const::create(canvas->surefind_canvas(child->get_attribute("use")->get_value()));
				else
					waypoint_value_node=canvas->surefind_value_node(child->get_attribute("use")->get_value());
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
				waypoint_value_node=waypoint_value_node->clone();

				// Warn if there is trash after the param value
				for(iter++; iter != list.end(); ++iter)
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
				waypoint->set_time_tension(atof(str.c_str()));
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
				string val=child->get_attribute("before")->get_value();
				if(val=="halt")
					waypoint->set_before(INTERPOLATION_HALT);
				else if(val=="constant")
					waypoint->set_before(INTERPOLATION_CONSTANT);
				else if(val=="linear")
					waypoint->set_before(INTERPOLATION_LINEAR);
				else if(val=="manual")
					waypoint->set_before(INTERPOLATION_MANUAL);
				else if(val=="auto")
					waypoint->set_before(INTERPOLATION_TCB);
				else
					error(child,strprintf(_("\"%s\" not a valid value for attribute \"%s\" in <%s>"),val.c_str(),"before","waypoint"));
			}

			if(child->get_attribute("after"))
			{
				string val=child->get_attribute("after")->get_value();
				if(val=="halt")
					waypoint->set_after(INTERPOLATION_HALT);
				else if(val=="constant")
					waypoint->set_after(INTERPOLATION_CONSTANT);
				else if(val=="linear")
					waypoint->set_after(INTERPOLATION_LINEAR);
				else if(val=="manual")
					waypoint->set_after(INTERPOLATION_MANUAL);
				else if(val=="auto")
					waypoint->set_after(INTERPOLATION_TCB);
				else
					error(child,strprintf(_("\"%s\" not a valid value for attribute \"%s\" in <%s>"),val.c_str(),"before","waypoint"));
			}
			}
			catch(Exception::BadTime x)
			{
				warning(child,x.what());
			}
			continue;

		}
		else
			error_unexpected_element(child,child->get_name());
	}
	value_node->changed();
	return value_node;
}

// This function is a phase-out hack for the timed swap value node
etl::handle<ValueNode_Animated>
CanvasParser::parse_timedswap(xmlpp::Element *node,Canvas::Handle canvas)
{
	ValueNode_TimedSwap::Handle timed_swap(parse_linkable_value_node(node,canvas));

	assert(timed_swap);

	ValueNode_Animated::Handle animated(ValueNode_Animated::create(timed_swap->get_type()));

	animated->set_root_canvas(canvas->get_root());

	assert(animated);

	Time swap_time, swap_length;
	(*timed_swap->get_swap_time())(0).put(&swap_time);
	(*timed_swap->get_swap_length())(0).put(&swap_length);

	animated->new_waypoint(swap_time-swap_length,timed_swap->get_before());
	animated->new_waypoint(swap_time,timed_swap->get_after());

	return animated;
}


handle<ValueNode_Subtract>
CanvasParser::parse_subtract(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="subtract");

	handle<ValueNode_Subtract> value_node;
	handle<ValueNode> lhs,rhs,scalar;

	if(element->get_attribute("type"))
	{
		ValueBase::Type type=ValueBase::ident_type(element->get_attribute("type")->get_value());

		if(!type)
		{
			error(element,"Bad type in <subtract>");
			return ValueNode_Subtract::Handle();
		}
		value_node=ValueNode_Subtract::create(type);
	}
	else
		value_node=ValueNode_Subtract::create();

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <subtract>")));
		return handle<ValueNode_Subtract>();
	}

	//if(element->get_attribute("scalar"))
	//{
	//	value_node->set_scalar(atof(element->get_attribute("scalar")->get_value().c_str()));
	//}

	try
	{
		if(element->get_attribute("scalar"))
		{
			// This is for compatibility with older versions of the file format
			String value(element->get_attribute("scalar")->get_value());
			if((value[0]<='9' && value[0]>='0')	 || value[0]=='-')
			{
				warning(element, _("Use of a real value where the ID should be is deprecated"));
				value_node->set_scalar(atof(value.c_str()));
			}
			else
				value_node->set_scalar(canvas->surefind_value_node(value));
			scalar=value_node->get_scalar();
		}

		if(element->get_attribute("lhs"))
		{
			lhs=canvas->surefind_value_node(element->get_attribute("lhs")->get_value());
			value_node->set_lhs(lhs);
		}

		if(element->get_attribute("rhs"))
		{
			rhs=canvas->surefind_value_node(element->get_attribute("rhs")->get_value());
			value_node->set_rhs(rhs);
		}
	}
	catch (Exception::IDNotFound)
	{
		error(element,"attribute in <subtract> references unknown ID");
	}
	catch (Exception::FileNotFound)
	{
		error(element,"Unable to open external file referenced in ID");
	}

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		if(child->get_name()=="lhs")
		{
			if(lhs)
			{
				error(child,"LHS component of <subtract> already defined!");
				continue;
			}

			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(child,_("<lhs> is missing its contents"));
				continue;
			}

			lhs=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

			if(lhs)
				value_node->set_lhs(lhs);
			else
			{
				error((*iter),"Parse of LHS ValueNode failed");
			}

			// \todo do a search for more elements and warn if they are found
		}
		else
		if(child->get_name()=="rhs")
		{
			if(rhs)
			{
				error(child,"RHS component of <subtract> already defined!");
				continue;
			}

			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(child,_("<rhs> is missing its contents"));
				continue;
			}

			rhs=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

			if(rhs)
				value_node->set_rhs(rhs);
			else
			{
				error((*iter),"Parse of RHS ValueNode failed");
			}

			// \todo do a search for more elements and warn if they are found
		}
		else
		if(child->get_name()=="scalar")
		{
			if(scalar)
			{
				error(child,"scalar component of <subtract> already defined!");
				continue;
			}

			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(child,_("<scalar> is missing its contents"));
				continue;
			}

			scalar=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

			if(scalar)
				value_node->set_scalar(scalar);
			else
			{
				error((*iter),"Parse of scalar ValueNode failed");
			}

			// \todo do a search for more elements and warn if they are found
		}
		else
			error_unexpected_element(child,child->get_name());
	}

	if(!value_node->get_rhs() || !value_node->get_lhs() || !value_node->get_scalar())
		error(element,"<subtract> is missing LHS, RHS, or SCALAR");

	if(value_node->get_rhs() == value_node->get_lhs())
		warning(element,"LHS is equal to RHS in <subtract>, so this value_node will always be zero!");

	return value_node;
}

etl::handle<LinkableValueNode>
CanvasParser::parse_linkable_value_node(xmlpp::Element *element,Canvas::Handle canvas)
{
	handle<LinkableValueNode> value_node;
	ValueBase::Type type;

	// Determine the type
	if(element->get_attribute("type"))
	{
		type=ValueBase::ident_type(element->get_attribute("type")->get_value());

		if(!type)
		{
			error(element,"Bad type in ValueNode");
			return 0;
		}
	}
	else
	{
		error(element,"Missing type in ValueNode");
		return 0;
	}

	value_node=LinkableValueNode::create(element->get_name(),type);

	if(!value_node)
	{
		error(element,"Unknown ValueNode type "+element->get_name());
		return 0;
	}

	if(value_node->get_type()!=type)
	{
		error(element,"ValueNode did not accept type");
		return 0;
	}

	value_node->set_root_canvas(canvas->get_root());

	int i;
	for(i=0;i<value_node->link_count();i++)
	{
		if(element->get_attribute(value_node->link_name(i)))
		try {
			String id(element->get_attribute(value_node->link_name(i))->get_value());

			if(!value_node->set_link(i,
					canvas->surefind_value_node(
						id
					)
				)
			) error(element,strprintf(_("Unable to set link \"%s\" to ValueNode \"%s\" (link #%d in \"%s\")"),value_node->link_name(i).c_str(),id.c_str(),i,value_node->get_name().c_str()));
		}
		catch(Exception::IDNotFound)
		{
			error(element,"Unable to resolve "+element->get_attribute(value_node->link_name(i))->get_value());
		}
		catch(Exception::FileNotFound)
		{
			error(element,"Unable to open file referenced in "+element->get_attribute(value_node->link_name(i))->get_value());
		}
		catch(...)
		{
			error(element,strprintf(_("Unknown Exception thrown when referencing ValueNode \"%s\""),
				element->get_attribute(value_node->link_name(i))->get_value().c_str()));
			throw;
		}
	}



	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		try
		{
			if(!child)
				continue;
			int index=value_node->get_link_index_from_name(child->get_name());

			xmlpp::Element::NodeList list = child->get_children();
			xmlpp::Element::NodeList::iterator iter;

			// Search for the first non-text XML element
			for(iter = list.begin(); iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter)) break;

			if(iter==list.end())
			{
				error(child,_("element is missing its contents"));
				continue;
			}

			ValueNode::Handle link=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

			if(!link)
			{
				error((*iter),"Parse of ValueNode failed");
			}
			else
			if(!value_node->set_link(index,link))
			{
				//error(dynamic_cast<xmlpp::Element*>(*iter),strprintf("Unable to connect value node ('%s' of type '%s') to link %d",link->get_name().c_str(),ValueBase::type_name(link->get_type()).c_str(),index));
				error(element,strprintf("Unable to connect value node ('%s' of type '%s') to link %d",link->get_name().c_str(),ValueBase::type_name(link->get_type()).c_str(),index));
			}

			// \todo do a search for more elements and warn if they are found

		}
		catch(Exception::BadLinkName)
		{
			error_unexpected_element(child,child->get_name());
		}
		catch(...)
		{
			error(child,strprintf(_("Unknown Exception thrown when working on element \"%s\""),child->get_name().c_str()));
			throw;
		}
	}

	return value_node;
}

handle<ValueNode_Composite>
CanvasParser::parse_composite(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="composite");

	if(!element->get_attribute("type"))
	{
		error(element,"Missing attribute \"type\" in <composite>");
		return handle<ValueNode_Composite>();
	}

	ValueBase::Type type=ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(!type)
	{
		error(element,"Bad type in <composite>");
		return handle<ValueNode_Composite>();
	}

	handle<ValueNode_Composite> value_node=ValueNode_Composite::create(type);
	handle<ValueNode> c[6];

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <composite>")));
		return handle<ValueNode_Composite>();
	}

	int i;

	for(i=0;i<value_node->link_count();i++)
	{
		string name=strprintf("c%d",i+1);
		if(c[i])
		{
			error(element,name+" was already defined in <composite>");
			continue;
		}
		if(element->get_attribute(name))
		{
			c[i]=canvas->surefind_value_node(element->get_attribute(name)->get_value());
			if(c[i])
			{
				if(!value_node->set_link(i,c[i]))
				{
					error(element,'"'+name+"\" attribute in <composite> has bad type");
				}
			}
			else
				error(element,'"'+name+"\" attribute in <composite> references unknown ID");
		}
	}

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(!child)
			continue;
		else
		for(i=0;i<value_node->link_count();i++)
		{
			string name=strprintf("c%d",i+1);
			if(child->get_name()==name)
			{
				if(c[i])
				{
					error(child,name+" was already defined in <composite>");
					break;
				}

				xmlpp::Element::NodeList list = child->get_children();
				xmlpp::Element::NodeList::iterator iter;

				// Search for the first non-text XML element
				for(iter = list.begin(); iter != list.end(); ++iter)
					if(dynamic_cast<xmlpp::Element*>(*iter)) break;

				if(iter==list.end())
				{
					error(child,strprintf(_("<%s> is missing its contents"),name.c_str()));
					break;
				}

				c[i]=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!c[i])
				{
					error((*iter),"Parse of "+name+" ValueNode failed");
					break;
				}

				if(!value_node->set_link(i,c[i]))
				{
					error(child,strprintf(_("<%s> has a bad value"),name.c_str()));
					break;
				}

				// \todo do a search for more elements and warn if they are found
				break;
			}
		}
		// somewhat of a hack, but it works
		if(i==value_node->link_count()) error_unexpected_element(child,child->get_name());
	}

	switch(value_node->link_count())
	{
	case 1:
		if(!value_node->get_link(0))
		{
			error(element,"<composite> is missing parameters");
			return handle<ValueNode_Composite>();
		}
		break;
	case 2:
		if(!value_node->get_link(0) ||!value_node->get_link(1))
		{
			error(element,"<composite> is missing parameters");
			return handle<ValueNode_Composite>();
		}
		break;
	case 3:
		if(!value_node->get_link(0) ||!value_node->get_link(1) ||!value_node->get_link(2))
		{
			error(element,"<composite> is missing parameters");
			return handle<ValueNode_Composite>();
		}
		break;
	case 4:
		if(!value_node->get_link(0) ||!value_node->get_link(1) ||!value_node->get_link(2) ||!value_node->get_link(3))
		{
			error(element,"<composite> is missing parameters");
			return handle<ValueNode_Composite>();
		}
		break;
    }
	return value_node;
}

// This will also parse a bline
handle<ValueNode_DynamicList>
CanvasParser::parse_dynamic_list(xmlpp::Element *element,Canvas::Handle canvas)
{
	assert(element->get_name()=="dynamic_list" || element->get_name()=="bline");

	const float fps(canvas?canvas->rend_desc().get_frame_rate():0);

	if(!element->get_attribute("type"))
	{
		error(element,"Missing attribute \"type\" in <dynamic_list>");
		return handle<ValueNode_DynamicList>();
	}

	ValueBase::Type type=ValueBase::ident_type(element->get_attribute("type")->get_value());

	if(!type)
	{
		error(element,"Bad type in <dynamic_list>");
		return handle<ValueNode_DynamicList>();
	}

	handle<ValueNode_DynamicList> value_node;
	handle<ValueNode_BLine> bline_value_node;

	if(element->get_name()=="bline")
	{
		value_node=bline_value_node=ValueNode_BLine::create();
		if(element->get_attribute("loop"))
		{
			String loop=element->get_attribute("loop")->get_value();
			if(loop=="true" || loop=="1" || loop=="TRUE" || loop=="True")
				bline_value_node->set_loop(true);
			else
				bline_value_node->set_loop(false);
		}

	}
	else
		value_node=ValueNode_DynamicList::create(type);

	if(!value_node)
	{
		error(element,strprintf(_("Unable to create <dynamic_list>")));
		return handle<ValueNode_DynamicList>();
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
				string id=child->get_attribute("use")->get_value();
				try
				{
					list_entry.value_node=canvas->surefind_value_node(id);
				}
				catch(Exception::IDNotFound)
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
			error_unexpected_element(child,child->get_name());
	}
	return value_node;
}

handle<ValueNode>
CanvasParser::parse_value_node(xmlpp::Element *element,Canvas::Handle canvas)
{
	handle<ValueNode> value_node;
	assert(element);

	GUID guid;

	if(element->get_attribute("guid"))
	{
		guid=GUID(element->get_attribute("guid")->get_value())^canvas->get_root()->get_guid();
		value_node=guid_cast<ValueNode>(guid);
		if(value_node)
			return value_node;
	}

	// If ValueBase::ident_type() recognises the name, then we know it's a ValueBase
	if(element->get_name()!="canvas" && ValueBase::ident_type(element->get_name()))
	{
		ValueBase data=parse_value(element,canvas);

		if(!data.is_valid())
		{
			error(element,strprintf(_("Bad data in <%s>"),element->get_name().c_str()));
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
		value_node=parse_animated(element,canvas);
	else
	if(element->get_name()=="subtract")
		value_node=parse_subtract(element,canvas);
	else
	if(element->get_name()=="composite")
		value_node=parse_composite(element,canvas);
	else
	if(element->get_name()=="dynamic_list")
		value_node=parse_dynamic_list(element,canvas);
	else
	if(element->get_name()=="bline") // This is not a typo. The dynamic list parser will parse a bline.
		value_node=parse_dynamic_list(element,canvas);
	else
	if(element->get_name()=="timed_swap")
		value_node=parse_timedswap(element,canvas);
	else
	if(LinkableValueNode::book().count(element->get_name()))
		value_node=parse_linkable_value_node(element,canvas);
	else
	if(element->get_name()=="canvas")
		value_node=ValueNode_Const::create(parse_canvas(element,canvas,true));
	else
	{
		error_unexpected_element(element,element->get_name());
		error(element, "Expected a ValueNode");
	}


	value_node->set_root_canvas(canvas->get_root());


	// If we were successful, and our element has
	// an ID attribute, go ahead and add it to the
	// value_node list
	if(value_node && element->get_attribute("id"))
	{
		string id=element->get_attribute("id")->get_value();

		//value_node->set_id(id);

		// If there is already a value_node in the list
		// with the same ID, then that is an error
		try { canvas->add_value_node(value_node,id); }
		catch(Exception::BadLinkName)
		{
			warning(element,strprintf(_("Bad ID \"%s\""),id.c_str()));
			return value_node;
		}
		catch(Exception::IDAlreadyExists)
		{
			error(element,strprintf(_("Duplicate ID \"%s\""),id.c_str()));
			return value_node;
		}
		catch(...)
		{
			error(element,strprintf(_("Unknown Exception thrown when adding ValueNode \"%s\""),id.c_str()));
			throw;
		}
	}
	value_node->set_guid(guid);
	return value_node;
}

void
CanvasParser::parse_canvas_defs(xmlpp::Element *element,Canvas::Handle canvas)
{
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

	layer=Layer::create(element->get_attribute("type")->get_value());
	layer->set_canvas(canvas);

	if(element->get_attribute("group"))
	{
		layer->add_to_group(
			element->get_attribute("group")->get_value()
		);
	}

	// Handle the version attribute
	if(element->get_attribute("version"))
	{
		String version(element->get_attribute("version")->get_value());
		if(version>layer->get_version())
			warning(element,_("Installed layer version is larger than layer version in file"));
		if(version!=layer->get_version())
			layer->set_version(version);
	}

	// Handle the description
	if(element->get_attribute("desc"))
		layer->set_description(element->get_attribute("desc")->get_value());

	if(element->get_attribute("active"))
		layer->set_active(element->get_attribute("active")->get_value()=="false"?false:true);

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

			if(child->get_attribute("use"))
			{
				// If the "use" attribute is used, then the
				// element should be empty. Warn the user if
				// we find otherwise.
				if(!list.empty())
					warning(child,_("Found \"use\" attribute for <param>, but it wasn't empty. Ignoring contents..."));

				String str=	child->get_attribute("use")->get_value();

				if(layer->get_param(param_name).get_type()==ValueBase::TYPE_CANVAS)
				{
					if(!layer->set_param(param_name,canvas->surefind_canvas(str)))
						error((*iter),_("Layer rejected canvas link"));
				}
				else
				try
				{
					handle<ValueNode> value_node=canvas->surefind_value_node(str);

					// Assign the value_node to the dynamic parameter list
					layer->connect_dynamic_param(param_name,value_node);
    			}
				catch(Exception::IDNotFound)
				{
					error(child,strprintf(_("Unknown ID (%s) referenced in <param>"),str.c_str()));
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

			// If we recognise the element name as a
			// ValueBase, then treat is at one
			if(/*(*iter)->get_name()!="canvas" && */ValueBase::ident_type((*iter)->get_name()) && !dynamic_cast<xmlpp::Element*>(*iter)->get_attribute("guid"))
			{
				ValueBase data=parse_value(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!data.is_valid())
				{
					error((*iter),_("Bad data for <param>"));
					continue;
				}

				// Set the layer's parameter, and make sure that
				// the layer liked it
				if(!layer->set_param(param_name,data))
				{
					warning((*iter),_("Layer rejected value for <param>"));
					continue;
				}
			}
			else	// ... otherwise, we assume that it is a ValueNode
			{
				handle<ValueNode> value_node=parse_value_node(dynamic_cast<xmlpp::Element*>(*iter),canvas);

				if(!value_node)
				{
					error((*iter),_("Bad data for <param>"));
					continue;
				}

				// Assign the value_node to the dynamic parameter list
				layer->connect_dynamic_param(param_name,value_node);
			}

			// Warn if there is trash after the param value
			for(iter++; iter != list.end(); ++iter)
				if(dynamic_cast<xmlpp::Element*>(*iter))
					warning((*iter),strprintf(_("Unexpected element <%s> after <param> data, ignoring..."),(*iter)->get_name().c_str()));
			continue;
		}
		else
			error_unexpected_element(child,child->get_name());
	}

	layer->reset_version();
	return layer;
}

Canvas::Handle
CanvasParser::parse_canvas(xmlpp::Element *element,Canvas::Handle parent,bool inline_, String filename)
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
				canvas=parent->find_canvas(element->get_attribute("id")->get_value());
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

	if(element->get_attribute("width"))
		canvas->rend_desc().set_w(atoi(element->get_attribute("width")->get_value().c_str()));

	if(element->get_attribute("height"))
		canvas->rend_desc().set_h(atoi(element->get_attribute("height")->get_value().c_str()));

	if(element->get_attribute("xres"))
		canvas->rend_desc().set_x_res(atof(element->get_attribute("xres")->get_value().c_str()));

	if(element->get_attribute("yres"))
		canvas->rend_desc().set_y_res(atof(element->get_attribute("yres")->get_value().c_str()));


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
		string values=element->get_attribute("view-box")->get_value();
		Vector
			tl,
			br;
		tl[0]=atof(string(values.data(),values.find(' ')).c_str());
		values=string(values.begin()+values.find(' ')+1,values.end());
		tl[1]=atof(string(values.data(),values.find(' ')).c_str());
		values=string(values.begin()+values.find(' ')+1,values.end());
		br[0]=atof(string(values.data(),values.find(' ')).c_str());
		values=string(values.begin()+values.find(' ')+1,values.end());
		br[1]=atof(values.c_str());

		canvas->rend_desc().set_tl(tl);
		canvas->rend_desc().set_br(br);
	}

	if(element->get_attribute("bgcolor"))
	{
		string values=element->get_attribute("bgcolor")->get_value();
		Color bg;

		bg.set_r(atof(string(values.data(),values.find(' ')).c_str()));
		values=string(values.begin()+values.find(' ')+1,values.end());

		bg.set_g(atof(string(values.data(),values.find(' ')).c_str()));
		values=string(values.begin()+values.find(' ')+1,values.end());

		bg.set_b(atof(string(values.data(),values.find(' ')).c_str()));
		values=string(values.begin()+values.find(' ')+1,values.end());

		bg.set_a(atof(values.c_str()));

		canvas->rend_desc().set_bg_color(bg);
	}

	if(element->get_attribute("focus"))
	{
		string values=element->get_attribute("focus")->get_value();
		Vector focus;

		focus[0]=atof(string(values.data(),values.find(' ')).c_str());
		values=string(values.begin()+values.find(' ')+1,values.end());
		focus[1]=atof(values.c_str());

		canvas->rend_desc().set_focus(focus);
	}

	canvas->rend_desc().set_flags(RendDesc::PX_ASPECT|RendDesc::IM_SPAN);

	xmlpp::Element::NodeList list = element->get_children();
	for(xmlpp::Element::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		xmlpp::Element *child(dynamic_cast<xmlpp::Element*>(*iter));
		if(child)
		{
			if(child->get_name()=="defs")
			{
				if(canvas->is_inline())
					error(child,_("Inline canvas cannot have a <defs> section"));
				parse_canvas_defs(child, canvas);
			}
			else
			if(child->get_name()=="keyframe")
			{
				if(canvas->is_inline())
				{
					warning(child,_("Inline canvas cannot have keyframes"));
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
					warning(child,_("Inline canvases cannot have metadata"));
					continue;
				}

				String name,content;

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

				canvas->set_meta_data(child->get_attribute("name")->get_value(),child->get_attribute("content")->get_value());
			}
			else if(child->get_name()=="name")
			{
				xmlpp::Element::NodeList list = child->get_children();

				// If we don't have any name, warn
				if(list.empty())
					warning(child,_("blank \"name\" entitity"));

				string tmp;
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
					warning(child,_("blank \"desc\" entitity"));

				string tmp;
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
					warning(child,_("blank \"author\" entitity"));

				string tmp;
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
				error_unexpected_element(child,child->get_name());
		}
//		else
//		if((child->get_name()=="text"||child->get_name()=="comment") && child->has_child_text())
//			continue;
	}

	if(canvas->value_node_list().placeholder_count())
	{
		error(element,strprintf(_("Canvas %s has undefined ValueNodes"),canvas->get_id().c_str()));
	}

	return canvas;
}

Canvas::Handle
CanvasParser::parse_from_file(const String &file)
{
	return parse_from_file_as(file,file);
}

Canvas::Handle
CanvasParser::parse_from_file_as(const String &file_,const String &as_)
{
	CHECK_EXPIRE_TIME();
	try
	{
        ChangeLocale change_locale(LC_NUMERIC, "C");
		String file(unix_to_local_path(file_));
		String as(unix_to_local_path(as_));

		if(get_open_canvas_map().count(etl::absolute_path(as)))
			return get_open_canvas_map()[etl::absolute_path(as)];

		filename=as;
		total_warnings_=0;
		xmlpp::DomParser parser(file);
		if(parser)
		{
			Canvas::Handle canvas(parse_canvas(parser.get_document()->get_root_node(),0,false,as));
			get_open_canvas_map()[etl::absolute_path(as)]=canvas;
			canvas->signal_deleted().connect(sigc::bind(sigc::ptr_fun(_remove_from_open_canvas_map),canvas.get()));
			canvas->signal_file_name_changed().connect(sigc::bind(sigc::ptr_fun(_canvas_file_name_changed),canvas.get()));



			const ValueNodeList& value_node_list(canvas->value_node_list());

			again:
			ValueNodeList::const_iterator iter;
			for(iter=value_node_list.begin();iter!=value_node_list.end();++iter)
			{
				ValueNode::Handle value_node(*iter);
				if(value_node->is_exported() && value_node->get_id().find("Unnamed")==0)
				{
					canvas->remove_value_node(value_node);
					goto again;
				}
			}

			return canvas;
		}
	}
	catch(Exception::BadLinkName) { synfig::error("BadLinkName Thrown"); }
	catch(Exception::BadType) { synfig::error("BadType Thrown"); }
	catch(Exception::FileNotFound) { synfig::error("FileNotFound Thrown"); }
	catch(Exception::IDNotFound) { synfig::error("IDNotFound Thrown"); }
	catch(Exception::IDAlreadyExists) { synfig::error("IDAlreadyExists Thrown"); }
	catch(const std::exception& ex)
	{
		synfig::error("Standard Exception: "+String(ex.what()));
		return Canvas::Handle();
	}
	catch(const String& str)
	{
		cerr<<str<<endl;
		//	synfig::error(str);
		return Canvas::Handle();
	}
	return Canvas::Handle();
}

Canvas::Handle
CanvasParser::parse_from_string(const String &data)
{
	CHECK_EXPIRE_TIME();

	try
	{
        ChangeLocale change_locale(LC_NUMERIC, "C");
		filename=_("<INTERNAL>");
		total_warnings_=0;
		xmlpp::DomParser parser;
		parser.parse_memory(data);
		xmlpp::Element *root=parser.get_document()->get_root_node();
		if(parser)
		{
			Canvas::Handle canvas(parse_canvas(root));
			canvas->signal_deleted().connect(sigc::bind(sigc::ptr_fun(_remove_from_open_canvas_map),canvas.get()));
			canvas->signal_file_name_changed().connect(sigc::bind(sigc::ptr_fun(_canvas_file_name_changed),canvas.get()));

			const ValueNodeList& value_node_list(canvas->value_node_list());
			again:
			ValueNodeList::const_iterator iter;
			for(iter=value_node_list.begin();iter!=value_node_list.end();++iter)
			{
				ValueNode::Handle value_node(*iter);
				if(value_node->is_exported() && value_node->get_id().find("Unnamed")==0)
				{
					canvas->remove_value_node(value_node);
					goto again;
				}
			}

			return canvas;
		}
	}
	catch(Exception::BadLinkName) { synfig::error("BadLinkName Thrown"); }
	catch(Exception::BadType) { synfig::error("BadType Thrown"); }
	catch(Exception::FileNotFound) { synfig::error("FileNotFound Thrown"); }
	catch(Exception::IDNotFound) { synfig::error("IDNotFound Thrown"); }
	catch(Exception::IDAlreadyExists) { synfig::error("IDAlreadyExists Thrown"); }
	catch(const std::exception& ex)
	{
		synfig::error("Standard Exception: "+String(ex.what()));
		return Canvas::Handle();
	}
	catch(const String& str)
	{
		cerr<<str<<endl;
		//	synfig::error(str);
		return Canvas::Handle();
	}
	return Canvas::Handle();
}
