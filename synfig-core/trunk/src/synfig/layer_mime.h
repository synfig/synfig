/* === S I N F G =========================================================== */
/*!	\file layer_mime.h
**	\brief Template Header
**
**	$Id: layer_mime.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_LAYER_MIME_H
#define __SINFG_LAYER_MIME_H

/* === H E A D E R S ======================================================= */

#include "layer.h"
#include "string.h"
#include <map>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

/*!	\class Layer_Mime
**	The mime layer is a layer that is used when an unknown
**	layer type is requested. This allows people without
**	all of the correct layers installed to still work with
**	that composition.
*/
class Layer_Mime : public Layer
{
	std::map<String,ValueBase> param_list;
	String name;
public:
	Layer_Mime(String name);

	virtual String get_version()const;

	virtual bool set_version(const String &ver);

	virtual bool set_param(const String &param, const ValueBase &value);

	virtual ValueBase get_param(const String &param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;
	virtual String get_local_name()const;

};

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
