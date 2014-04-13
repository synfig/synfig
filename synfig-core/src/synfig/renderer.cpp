/* === S Y N F I G ========================================================= */
/*!	\file synfig/renderer.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 IvanMahonin
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "renderer.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer::RendererId Renderer::last_registered_id = 0;
Renderer::BookCreate Renderer::book_create;
Renderer::BookCopy Renderer::book_copy;
Renderer::BookConvert Renderer::book_convert;
Renderer::BookConvertChain Renderer::book_convert_chain;


// PrimitiveBase

Renderer::PrimitiveBase::Map::iterator
Renderer::PrimitiveBase::get_entry(RendererId renderer_id) const
{
	Map::iterator i = primitives.find(renderer_id);
	if (i != primitives.end()) return i;

	// try convert
	const ConvertChainEntry *entry = NULL;
	KeyConvert key(type, renderer_id, renderer_id);
	for(Map::iterator j = primitives.begin(); j != primitives.end(); ++j)
	{
		key.renderer_id_from = j->first;
		BookConvertChain::const_iterator k = book_convert_chain.find(key);
		if (k != book_convert_chain.end())
		{
			if (entry == NULL || k->second.count < entry->count) {
				i = j;
				entry = &k->second;
				if (entry->count == 0) break;
			}
		}
	}

	while (entry != NULL)
	{
		key.renderer_id_from = i->first;
		key.renderer_id_to = entry->renderer_id_to;
		PrimitiveDataBase::Handle primitive = entry->func(key, i->second);
		if (!primitive) break;
		i = primitives.insert(Pair(key.renderer_id_to, primitive));
		if (i->first == renderer_id) return i;
		entry = entry->next;
	}

	return primitives.end();
}

Renderer::PrimitiveDataBase::ConstHandle
Renderer::PrimitiveBase::get_primitive(RendererId renderer_id) const
{
	if (editing) return PrimitiveDataBase::ConstHandle();
	Map::iterator i = get_entry(renderer_id);
	return i == primitives.end() ? PrimitiveDataBase::ConstHandle() : i->second;
}

Renderer::PrimitiveDataBase::Handle
Renderer::PrimitiveBase::begin_edit_primitive(RendererId renderer_id)
{
	if (editing) return PrimitiveDataBase::Handle();

	// try create
	if (primitives.empty())
	{
		BookCreate::const_iterator j = book_create.find(KeyCreate(type, renderer_id));
		if (j == book_create.end()) return PrimitiveDataBase::Handle();
		PrimitiveDataBase::Handle primitive = j->second();
		return primitive ? (primitives[renderer_id] = primitive) : PrimitiveDataBase::Handle();
	}

	// try convert
	Map::iterator i = get_entry(renderer_id);
	if (i == primitives.end()) return PrimitiveDataBase::Handle();

	// try to make copy if need
	if (i->second.count() > 1)
	{
		BookCopy::const_iterator j = book_copy.find(KeyCreate(type, renderer_id));
		if (j == book_copy.end()) return PrimitiveDataBase::Handle();
		PrimitiveDataBase::Handle primitive = j->second(i->second);
		if (!primitive) return PrimitiveDataBase::Handle();
		i->second = primitive;
	}

	editing = true;

	// remove all but i
	for(Map::iterator j = primitives.begin(); j != primitives.end();)
		if (j == i) ++j; else primitives.erase(j++);

	return i->second;
}

void
Renderer::PrimitiveBase::end_edit_primitive()
{
	editing = false;
}


// Renderer

Renderer::Renderer()
{
	for(int i = 0; i < PrimitiveTypeCount; ++i)
		supported_primitives[i] = false;
}

Renderer::~Renderer() { }

void
Renderer::build_convert_chain()
{
	// TODO:
}

void Renderer::register_renderer(int &id)
{
	if (id != 0) return;
	id = ++last_registered_id;
}

void Renderer::register_func_create(const KeyCreate &key, FuncCreate func)
{
	if (book_create.count(key) == 0) book_create[key] = func;
}

void Renderer::register_func_copy(const KeyCopy &key,FuncCreate func)
{
	if (book_copy.count(key) == 0) book_copy[key] = func;
}

void Renderer::register_func_convert(const KeyConvert &key,FuncCreate func)
{
	if (book_convert.count(key) == 0) book_convert[key] = func;
	build_convert_chain();
}

void Renderer::unregister_renderer(int &id)
{
	if (id == 0) return;
	for(BookCreate::iterator i = book_create.begin(); i != book_create.end();)
		if (i->first.renderer_id == id)
			book_create.erase(i++); else ++i;
	for(BookCopy::iterator i = book_copy.begin(); i != book_copy.end();)
		if (i->first.renderer_id == id)
			book_copy.erase(i++); else ++i;
	for(BookConvert::iterator i = book_convert.begin(); i != book_convert.end();)
		if (i->first.renderer_id_from == id || i->first.renderer_id_to == id)
			book_convert.erase(i++); else ++i;
	id = 0;
	build_convert_chain();
}

Renderer::Result Renderer::render(const Params &params, const PrimitiveBase &primitive)
{
	switch(primitive.type)
	{
	case PrimitiveTypeSurface:
	{
		const Primitive<PrimitiveTypeSurface>* p =
			*dynamic_cast<const Primitive<PrimitiveTypeSurface>*>(&primitive);
		return p == NULL ? ResultFail : Renderer::render_surface(params, *p);
	}
	case PrimitiveTypePolygon:
	{
		const Primitive<PrimitiveTypePolygon>* p =
			*dynamic_cast<const Primitive<PrimitiveTypePolygon>*>(&primitive);
		return p == NULL ? ResultFail : Renderer::render_polygon(params, *p);
	}
	case PrimitiveTypeColoredPolygon:
	{
		const Primitive<PrimitiveTypeColoredPolygon>* p =
			*dynamic_cast<const Primitive<PrimitiveTypeColoredPolygon>*>(&primitive);
		return p == NULL ? ResultFail : Renderer::render_colored_polygon(params, *p);
	}
	case PrimitiveTypeMesh:
	{
		const Primitive<PrimitiveTypeMesh>* p =
			*dynamic_cast<const Primitive<PrimitiveTypeMesh>*>(&primitive);
		return p == NULL ? ResultFail : Renderer::render_mesh(params, *p);
	}
	default:
		break;
	}
	return ResultFail;
}

Renderer::Result Renderer::render_surface(const Params &params, const Primitive<PrimitiveTypeSurface> &primitive)
	{ return Renderer::ResultNotSupported; }
Renderer::Result Renderer::render_polygon(const Params &params, const Primitive<PrimitiveTypePolygon> &primitive)
	{ return Renderer::ResultNotSupported; }
Renderer::Result Renderer::render_colored_polygon(const Params &params, const Primitive<PrimitiveTypeColoredPolygon> &primitive)
	{ return Renderer::ResultNotSupported; }
Renderer::Result Renderer::render_mesh(const Params &params, const Primitive<PrimitiveTypeMesh> &primitive)
	{ return Renderer::ResultNotSupported; }


/* === E N T R Y P O I N T ================================================= */
