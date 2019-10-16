/* === S Y N F I G ========================================================= */
/*!	\file layer_switch.h
**	\brief Header file for implementation of the "Switch" layer
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

#ifndef __SYNFIG_LAYER_SWITCH_H
#define __SYNFIG_LAYER_SWITCH_H

/* === H E A D E R S ======================================================= */

#include "layer_pastecanvas.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
/*!	\class Layer_Switch
**	\brief Class of the Switch layer.
*/
class Layer_Switch : public Layer_PasteCanvas
{
	//! Layer module: defines the needed members to belong to a layer's factory.
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (String) Active Layer Name
	ValueBase param_layer_name;
	//! Parameter: (int) Active Layer Depth
	ValueBase param_layer_depth;

	mutable std::set<String> last_possible_layers;
	mutable std::set<String> last_existant_layers;

	sigc::signal<void> signal_possible_layers_changed_;
	void possible_layers_changed();

public:
	//! Default constructor
	Layer_Switch();
	//! Destructor
	virtual ~Layer_Switch();
	//! Returns a string with the localized name of this layer
	virtual String get_local_name()const;

	//!	Sets the parameter described by \a param to \a value. \see Layer::set_param
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	//! Get the value of the specified parameter. \see Layer::get_param
	virtual ValueBase get_param(const String & param)const;
	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;

	Layer::Handle get_current_layer()const;

	sigc::signal<void>& signal_possible_layers_changed() { return signal_possible_layers_changed_; }

	virtual void on_childs_changed();
	virtual void on_static_param_changed(const String &param);
	virtual void on_dynamic_param_changed(const String &param);
	virtual void on_possible_layers_changed() { }

	void get_existant_layers(std::set<String> &x) const;
	void get_possible_layers(std::set<String> &x) const;
	void get_possible_new_layers(std::set<String> &x) const;
	void get_impossible_existant_layers(std::set<String> &x) const;

	//! Sets z_range* fields of specified ContextParams \a cp
	virtual void apply_z_range_to_params(ContextParams &cp)const;
}; // END of class Layer_Switch

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
