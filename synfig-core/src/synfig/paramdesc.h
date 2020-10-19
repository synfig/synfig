/* === S Y N F I G ========================================================= */
/*!	\file paramdesc.h
**	\brief ParamDesc Class Implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_PARAMDESC_H
#define __SYNFIG_PARAMDESC_H

/* === H E A D E R S ======================================================= */

#include "string.h"
#include "real.h"
#include "color.h"
#include "interpolation.h"
#include <list>
#include <cassert>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueBase;

/*!	\class ParamDesc
**	\brief Parameter Description Class
**	\todo writeme
*/
class ParamDesc
{
public:

	//! \writeme
	struct EnumData
	{
		int value;
		String name;
		String local_name;
		EnumData(int value, const String &name, const String &local_name):
			value(value),
			name(name),
			local_name(local_name)
		{
		}
	};

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:
   	String name_;			//! The actual parameter name
   	String local_name_; 	//! Localized name
   	String desc_;			//! Short description of parameter (Think tooltops)
   	String group_;			//! Which group this parameter is a member of (optional)
   	String hint_;			//! Parameter hint
   	String origin_;			//! Parameter origin
   	String connect_;
   	String box_;
	Real scalar_;			//! Scalar value for visual editing
	bool exponential_;		//! Allow range from -inf to inf for visual editing
   	bool critical_;
	bool hidden_;
	bool invisible_duck_;
	bool is_distance_;
	bool animation_only_;
	bool static_;
	Interpolation interpolation_;

	std::list<EnumData> enum_list_;

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:

   	ParamDesc(const String &a="IM_A_BUG_SO_REPORT_ME"):
		name_           (a),
		local_name_     (a),
		scalar_         (1.0),
		exponential_    (false),
		critical_       (true),
		hidden_         (false),
		invisible_duck_ (false),
		is_distance_    (false),
		animation_only_ (false),
		static_         (false),
		interpolation_  (INTERPOLATION_UNDEFINED)
	{ }

   	ParamDesc(const ValueBase&, const String &a);

   	ParamDesc(const String &name, const ParamDesc &blank)
   		{ *this = blank; name_ = name; }

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! \writeme
	const std::list<EnumData> &get_enum_list()const { return enum_list_; }

   	//! Sets the localized name of the parameter.
   	ParamDesc &set_local_name(const String &n) { local_name_=n; return *this; }

   	//! Sets the localized description of the parameter.
   	ParamDesc &set_description(const String &d) { desc_=d; return *this; }

   	//! Sets the group that this parameter is a member of
   	ParamDesc &set_group(const String &n) { group_=n; return *this; }

   	//! Sets a "hint" for the parameter.
   	ParamDesc &set_hint(const String &h) { hint_=h; return *this; }

   	//! \writeme
   	ParamDesc &set_connect(const String &h) { connect_=h; return *this; }

   	//! \writeme
   	ParamDesc &set_box(const String &h) { box_=h; return *this; }

   	//! Sets a flag regarding the duck visibility
   	ParamDesc &set_invisible_duck(bool x=true) { invisible_duck_=x; return *this; }

   	//! Returns the flag regarding duck visibility
   	bool get_invisible_duck() { return invisible_duck_; }


   	//! \writeme
   	ParamDesc &set_animation_only(bool x=true) { animation_only_=x; return *this; }

   	//! \writeme
   	bool get_animation_only() { return animation_only_; }


   	//! Sets which parameter is to be used as the origin when the user edits visually.
   	ParamDesc &set_origin(const String &h) { origin_=h; return *this; }

   	//! Sets the scalar value for the parameter
   	/*! This value determines how the value is to be presented
   	**	to the user when editing visually. */
   	ParamDesc &set_scalar(const Real &n) { scalar_=n; return *this; }
	
	//! Sets if the parameter value should be exposed for visual editing using the exponential function
   	/*!	Such representation allows to set the Real values in the range from \c -inf to \c inf . */
	ParamDesc &set_exponential(bool x=true) { exponential_=x; return *this; }

   	//!	Marks the parameter as not necessary for saving or copying
   	ParamDesc &not_critical() { critical_=false; return *this; }

   	//!	\writeme
   	ParamDesc &hidden() { hidden_=true; return *this; }

   	//!	Marks the parameter as only readable. Implies not_critical()
   	/*!	\todo This function needs to be written, as it is only a stub */
   	ParamDesc &read_only() { return *this; }

   	//!	Marks the parameter as only writable. Implies not_critical()
   	/*!	\todo This function needs to be written, as it is only a stub */
   	ParamDesc &write_only() { return *this; }

   	//! Adds a description of a possible enumeration value
   	/*!	Only relevant if the parameter is of an integer type and hint set to \c "enum" . */
   	ParamDesc &add_enum_value(int val, const String &enum_name,const String &enum_local_name)
		{ enum_list_.push_back(EnumData(val,enum_name,enum_local_name)); return *this; }

   	//! Returns the localized name of the parameter
   	const String &get_local_name()const { return local_name_; }

   	//! Returns the name of the parameter
   	const String &get_name()const { return name_; }

   	//! Returns the localized description of the parameter
   	const String &get_description()const { return desc_; }

   	//! Returns the parameter's group
   	const String &get_group()const { return group_; }

   	//! Returns a "hint" about the parameter, regarding how it is to be displayed to the user
   	const String &get_hint()const { return hint_; }

   	//! Returns the name of the parameter that is defined as the "origin". Used for visual editing.
   	const String &get_origin()const { return origin_; }

   	//! \writeme
   	const String &get_connect()const { return connect_; }

	   	//! \writeme
   	const String &get_box()const { return box_; }

   	//! Returns the scalar value for the parameter. Used for visual editing.
   	const Real &get_scalar()const { return scalar_; }
	
	//! Tells if the value should be exposed for editing using the exponential function
   	/*!	Such representation allows to set the Real values in the range from \c -inf to \c inf . */
	bool get_exponential()const {return exponential_; }

   	//! Returns \c true if the layer is critical, \c false otherwise.
   	bool get_critical()const { return critical_; }

   	//! Returns \c true if the layer is hidden, \c false otherwise.
   	bool get_hidden()const { return hidden_; }
	
	bool get_static()const { return static_; }
	ParamDesc &set_static(bool s) { static_=s; return *this; }

	Interpolation get_interpolation()const { return interpolation_; }
	ParamDesc &set_interpolation(Interpolation i) { interpolation_=i; return *this; }

   	ParamDesc& set_is_distance(bool x=true) { is_distance_=x; return *this;}
   	bool get_is_distance()const { return is_distance_; }
}; // END of class ParamDesc

class ParamVocab : public std::list< ParamDesc >
{
public:
	const ParamDesc& operator[] (const String &name) const
	{
		static const ParamDesc blank;

		for(const_iterator i = begin(); i != end(); ++i)
			if (i->get_name() == name) return *i;
		assert(false);
		return blank;
	}
};

} // END of namespace synfig

/* === E N D =============================================================== */

#endif
