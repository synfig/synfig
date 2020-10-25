/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/surfacefile.h
**	\brief SurfaceFile Header
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SURFACEFILE_H
#define __SYNFIG_RENDERING_SURFACEFILE_H

/* === H E A D E R S ======================================================= */

#include <synfig/filesystem.h>

#include "../surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class SurfaceFile: public Surface
{
public:
	typedef etl::handle<SurfaceFile> Handle;
	static Token token;
	virtual Token::Handle get_token() const
		{ return token.handle(); }

private:
	FileSystem::Identifier identifier;

protected:
	virtual bool get_pixels_vfunc(Color *buffer) const;

public:
	SurfaceFile() { }
	SurfaceFile(const FileSystem::Identifier &identifier)
		{ set_identifier(identifier); }

	virtual bool is_read_only() const
		{ return true; }

	const FileSystem::Identifier& get_identifier() const
		{ return identifier; }

	void set_identifier(const FileSystem::Identifier &identifier) {
		if (this->identifier != identifier)
			{ this->identifier = identifier; reload(); }
	}

	bool reload();
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
