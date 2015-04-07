/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderer.h
**	\brief Renderer Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_RENDERER_H
#define __SYNFIG_RENDERING_RENDERER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class RendererDependentObject;
class Surface;
class Transformation;
class Blending;
class Primitive;

class Renderer: public etl::shared_object
{
public:
	typedef etl::handle<Renderer> Handle;

	class DependentObject: public etl::shared_object
	{
	public:
		typedef etl::handle<DependentObject> Handle;
		typedef std::vector<Renderer::Handle> RendererList;
		typedef std::vector<Handle> AlternativeList;

	private:
		RendererList renderers;
		AlternativeList alternatives;

	public:
		bool check_renderer_ptr(const Renderer *x) const
		{
			for(RendererList::const_iterator i = renderers.begin(); i != renderers.end(); ++i)
				if (i == x) return true;
			return false;
		}

		bool check_renderer(const Renderer::Handle &x) const
			{ return check_renderer_ptr(x.get()); }
		void add_renderer(const Renderer::Handle &x)
			{ if (x && !check_renderer(x)) renderers.push_back(x); }

		const AlternativeList& get_alternatives() const
			{ return alternatives; }
		bool check_alternative_ptr(const DependentObject *x) const
		{
			for(AlternativeList::const_iterator i = get_alternatives().begin(); i != get_alternatives().end(); ++i)
				if (i == x) return true;
			return false;
		}
		bool check_alternative(const Handle &x) const
			{ return check_alternative_ptr(x.get()); }
		void add_alternative(const Handle &x)
			{ if (x && !check_alternative(x)) alternatives.push_back(x); }

		void changed()
			{ renderers.clear(); alternatives.clear(); }
	};

private:
	Handle alternative;

	DependentObject::Handle convert_obj(const DependentObject::Handle &obj);

protected:
	virtual bool is_supported_vfunc(const etl::handle<DependentObject> &obj) const;
	virtual etl::handle<DependentObject> convert_vfunc(const etl::handle<DependentObject> &obj);
	virtual bool draw_vfunc(
			const etl::handle<Surface> &target_surface,
			const etl::handle<Transformation> &transformation,
			const etl::handle<Blending> &blending,
			const etl::handle<Primitive> &primitive );

public:
	Handle get_alternative() const { return alternative; }
	void set_alternative(const Handle &x) { alternative = x; }

	etl::handle<Surface> convert_surface(const etl::handle<Surface> &surface);
	etl::handle<Transformation> convert_transformation(const etl::handle<Transformation> &transformation);
	etl::handle<Blending> convert_blending(const etl::handle<Blending> &blending);
	etl::handle<Primitive> convert_primtive(const etl::handle<Primitive> &primitive);

	etl::handle<Surface> convert(const etl::handle<Surface> &surface)
		{ return convert_surface(surface); }
	etl::handle<Transformation> convert(const etl::handle<Transformation> &transformation)
		{ return convert_transformation(transformation); }
	etl::handle<Blending> convert(const etl::handle<Blending> &blending)
		{ return convert_blending(blending); }
	etl::handle<Primitive> convert(const etl::handle<Primitive> &primitive)
		{ return convert_primtive(primitive); }

	bool draw(
			const etl::handle<Surface> &target_surface,
			const etl::handle<Transformation> &transformation,
			const etl::handle<Blending> &blending,
			const etl::handle<Primitive> &primitive );
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
