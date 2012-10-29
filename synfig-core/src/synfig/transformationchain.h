/* === S Y N F I G ========================================================= */
/*!	\file transformationchain.h
**	\brief Vector transformation matrixes chain
**
**	$Id$
**
**	\legal
**	Copyright (c) 2012 Diego Barrios Romero
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

#ifndef __SYNFIG_TRANSFORMATIONCHAIN_H
#define __SYNFIG_TRANSFORMATIONCHAIN_H

#include <list>
#include "matrix.h"

namespace synfig {

/*!	\class TransformationChain
**	This class contains a list or chain of transformation matrixes.
**	When advancing through the layers hierarchy new transformation matrixes
**	will be added or the last matrix will be modified.
**	i.e., when performing a Zoom, 3 transformation matrixes will be added
**	for the general case when a translation of the object according to the
**	zoom center, the scaling, and a back translation of the object will be
**	performed.
*/
class TransformationChain
{
private:
	std::list<Matrix> _list;

public:
	/// Push a transformation matrix into the list
	TransformationChain& push(const Matrix& matrix)
	{
		_list.push_front(matrix);

		return *this;
	}

	/// Most recently pushed matrix, for cases when it's enough to modify the
	/// values and a new matrix is not necessary
	Matrix& front()
	{
		return _list.front();
	}

	/// Given a vector, applies the transformation matrixes and returns the result
	Vector get_transformed(const Vector& input) const
	{
		Vector result(input);
		for(std::list<Matrix>::const_iterator i = _list.begin(); i != _list.end(); i++)
			result = i->get_transformed(result);

		return result;
	}
};

}; // END of namespace synfig

#endif /* __SYNFIG_TRANSFORMATIONCHAIN_H */
