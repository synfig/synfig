/* === S I N F G =========================================================== */
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

#ifndef __SINFG_IMPORT_H
#define __SINFG_IMPORT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_bitmap.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/importer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Import : public sinfg::Layer_Bitmap
{
	SINFG_LAYER_MODULE_EXT

private:
	sinfg::String filename;
	sinfg::String abs_filename;
	sinfg::Importer::Handle importer;
	sinfg::Time time_offset;

protected:
	Import();

public:
	~Import();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;	

	virtual Vocab get_param_vocab()const;

	virtual void on_canvas_set();

	virtual void set_time(sinfg::Context context, sinfg::Time time)const;

	virtual void set_time(sinfg::Context context, sinfg::Time time, const sinfg::Point &pos)const;
};

/* === E N D =============================================================== */

#endif
