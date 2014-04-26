/* === S Y N F I G ========================================================= */
/*!	\file synfig/renderer.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERER_H
#define __SYNFIG_RENDERER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <map>
#include <limits>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

class Renderer
{
public:
	typedef int RendererId;

	enum PrimitiveType
	{
		PrimitiveTypeSurface,
		PrimitiveTypePolygon,
		PrimitiveTypeColoredPolygon,
		PrimitiveTypeMesh
	};

	enum { PrimitiveTypeCount = 4 };

	//! Base class of PrimitiveData (PrimitiveData base), NOT database
	class PrimitiveDataBase: public etl::shared_object {
	public:
		typedef etl::handle<PrimitiveDataBase> Handle;
		typedef etl::handle<const PrimitiveDataBase> ConstHandle;
	};

	template<typename T>
	class PrimitiveData: public PrimitiveDataBase {
	public:
		typedef T Data;
		typedef etl::handle<PrimitiveData> Handle;
		typedef etl::handle<const PrimitiveData> ConstHandle;
		Data data;
	};

	typedef PrimitiveDataBase::Handle (*FuncCreate)();
	typedef PrimitiveDataBase::Handle (*FuncCopy)(PrimitiveDataBase::Handle primitive);
	typedef PrimitiveDataBase::Handle (*FuncConvert)(PrimitiveDataBase::Handle primitive);

	struct KeyCreate {
		PrimitiveType primitive_type;
		RendererId renderer_id;

		inline KeyCreate():
			primitive_type(PrimitiveTypeSurface), renderer_id(0) { }
		inline KeyCreate(PrimitiveType primitive_type, RendererId renderer_id):
			primitive_type(primitive_type), renderer_id(renderer_id) { }
		inline bool operator < (const KeyCreate &other) const
		{
			return primitive_type < other.primitive_type ? true
				 : other.primitive_type < primitive_type ? false
				 : renderer_id < other.renderer_id;
		}
	};

	typedef KeyCreate KeyCopy;

	struct KeyConvert {
		PrimitiveType primitive_type;
		RendererId renderer_id_from;
		RendererId renderer_id_to;

		inline KeyConvert():
			primitive_type(PrimitiveTypeSurface), renderer_id_from(0), renderer_id_to(0) { }
		inline KeyConvert(PrimitiveType primitive_type, RendererId renderer_id_from, RendererId renderer_id_to):
			primitive_type(primitive_type), renderer_id_from(renderer_id_from), renderer_id_to(renderer_id_to) { }
		inline bool operator < (const KeyConvert &other) const
		{
			return primitive_type < other.primitive_type ? true
				 : other.primitive_type < primitive_type ? false
				 : renderer_id_from < other.renderer_id_from ? true
				 : other.renderer_id_from < renderer_id_from ? false
				 : renderer_id_to < other.renderer_id_to;
		}
	};

	struct ConvertChainEntry {
	public:
		int count;
		RendererId renderer_id_to;
		ConvertChainEntry *next;
		FuncConvert func;
		inline ConvertChainEntry():
			count(0), renderer_id_to(0), next(NULL), func(NULL) { }
	};

	typedef std::map<KeyCreate, FuncCreate> BookCreate;
	typedef std::map<KeyCopy, FuncCopy> BookCopy;
	typedef std::map<KeyConvert, FuncConvert> BookConvert;
	typedef std::map<KeyConvert, ConvertChainEntry> BookConvertChain;

private:
	static RendererId last_registered_id;
	static BookCreate book_create;
	static BookCopy book_copy;
	static BookConvert book_convert;
	static BookConvertChain book_convert_chain;

	static void build_convert_chain();

protected:
	static void register_renderer(int &id);
	static void register_func_create(const KeyCreate &key, FuncCreate func);
	static void register_func_copy(const KeyCopy &key, FuncCopy func);
	static void register_func_convert(const KeyConvert &key, FuncConvert func);
	static void unregister_renderer(int &id);

	template<typename T>
	static PrimitiveDataBase::Handle func_default_create()
		{ return new T(); }

	static PrimitiveDataBase::Handle func_default_convert(PrimitiveDataBase::Handle primitive)
		{ return primitive; }

public:
	class PrimitiveBase
	{
	private:
		typedef std::map<RendererId, PrimitiveDataBase::Handle> Map;
		typedef Map::value_type Pair;
		mutable Map primitives;
		bool editing;

	public:
		const PrimitiveType type;

	private:
		Map::iterator get_entry(RendererId renderer_id) const;

	protected:
		inline PrimitiveBase(PrimitiveType type): editing(false), type(type) { }

	public:
		virtual ~PrimitiveBase();
		inline bool is_editing() const { return editing; }
		PrimitiveDataBase::ConstHandle get_primitive(RendererId renderer_id) const;
		PrimitiveDataBase::Handle begin_edit_primitive(RendererId renderer_id);
		void end_edit_primitive();
	};

	template<typename RendererType, Renderer::PrimitiveType primitive_type>
	class TypesTemplate { };

	template<PrimitiveType primitive_type>
	class Primitive: public PrimitiveBase
	{
	public:
		Primitive(): PrimitiveBase(primitive_type) { }

		template<typename RendererType>
		const typename TypesTemplate<RendererType, primitive_type>::Data* get()
		{
			typedef PrimitiveData<typename TypesTemplate<RendererType, primitive_type>::Data> PrimitiveData;
			typedef typename PrimitiveData::ConstHandle ConstHandle;
			ConstHandle handle =
				ConstHandle::cast_dynamic(
					PrimitiveBase::get_primitive(
						TypesTemplate<RendererType, primitive_type>::get_id()));
			return handle ? &handle->data : NULL;
		}

		template<typename RendererType>
		typename TypesTemplate<RendererType, primitive_type>::Data* begin_edit()
		{
			PrimitiveDataBase::Handle base_handle =
				PrimitiveBase::begin_edit_primitive(
					TypesTemplate<RendererType, primitive_type>::get_id());
			if (base_handle)
			{
				typedef typename TypesTemplate<RendererType, primitive_type>::Data Data;
				typedef PrimitiveData<Data> PrimitiveData;
				typedef typename PrimitiveData::Handle Handle;
				Handle handle = Handle::cast_dynamic(base_handle);
				if (handle) return &handle->data;
				Renderer::PrimitiveBase::end_edit_primitive();
			}
			return NULL;
		}

		inline void end_edit() { Renderer::PrimitiveBase::end_edit_primitive(); }
	};

	template<typename T>
	class TypesTemplateBase {
		typedef T Data;
		typedef PrimitiveData<Data> Primitive;
	};

	template<typename T>
	class TypesBase
	{
	public:
		typedef T RendererType;
		typedef TypesTemplate<RendererType, PrimitiveTypeSurface> Surface;
		typedef TypesTemplate<RendererType, PrimitiveTypePolygon> Polygon;
		typedef TypesTemplate<RendererType, PrimitiveTypeColoredPolygon> ColoredPolygon;
		typedef TypesTemplate<RendererType, PrimitiveTypeMesh> Mesh;
	};

	typedef Primitive<PrimitiveTypeSurface> PrimitiveSurface;
	typedef Primitive<PrimitiveTypePolygon> PrimitivePolygon;
	typedef Primitive<PrimitiveTypeColoredPolygon> PrimitiveColoredPolygon;
	typedef Primitive<PrimitiveTypeMesh> PrimitiveMesh;

	class Params {
	public:
		// TODO: blend method, color
		PrimitiveBase *out_surface;
		const PrimitiveBase *back_surface;
		const PrimitiveBase *mesh_texture_surface;
		inline Params(): out_surface(NULL), back_surface(NULL), mesh_texture_surface(NULL) { }
	};

	enum Result {
		ResultSuccess,
		ResultNotSupported,
		ResultFail
	};

protected:
	bool supported_primitives[PrimitiveTypeCount];

public:
	inline bool is_primitive_supported(PrimitiveType primitive_type)
		{ return supported_primitives[primitive_type]; }

	Renderer();
	virtual ~Renderer();
	virtual Result render(const Params &params, const PrimitiveBase &primitive);
	virtual Result render_surface(const Params &params, const Primitive<PrimitiveTypeSurface> &primitive);
	virtual Result render_polygon(const Params &params, const Primitive<PrimitiveTypePolygon> &primitive);
	virtual Result render_colored_polygon(const Params &params, const Primitive<PrimitiveTypeColoredPolygon> &primitive);
	virtual Result render_mesh(const Params &params, const Primitive<PrimitiveTypeMesh> &primitive);
};

}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
