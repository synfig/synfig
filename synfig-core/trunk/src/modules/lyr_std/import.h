/* === S Y N F I G ========================================================= */
/*!	\file import.h
**	\brief Template Header
**
**	$Id: import.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SYNFIG_IMPORT_H
#define __SYNFIG_IMPORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_bitmap.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/importer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Import : public synfig::Layer_Bitmap
{
	SYNFIG_LAYER_MODULE_EXT

private:
	synfig::String filename;
	synfig::String abs_filename;
	synfig::Importer::Handle importer;
	synfig::Time time_offset;

protected:
	Import();

public:
	~Import();
	
	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;	

	virtual Vocab get_param_vocab()const;

	virtual void on_canvas_set();

	virtual void set_time(synfig::Context context, synfig::Time time)const;

	virtual void set_time(synfig::Context context, synfig::Time time, const synfig::Point &pos)const;
};

/* === E N D =============================================================== */

#endif
