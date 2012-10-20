/* === S Y N F I G ========================================================= */
/*!	\file tool/optionsprocessor.h
**	\brief Synfig Tool Options Processor Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2012 Diego Barrios Romero
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

#ifndef __SYNFIG_OPTIONSPROCESSOR_H
#define __SYNFIG_OPTIONSPROCESSOR_H

class OptionsProcessor
{
public:
	OptionsProcessor(boost::program_options::variables_map& vm,
					   const boost::program_options::options_description& po_visible)
		: _vm(vm), _po_visible(po_visible) { }

#ifdef _DEBUG
	void process_debug_options() throw (SynfigToolException&);
#endif

	void process_settings_options();

	void process_info_options() throw (SynfigToolException&);

	Job extract_job() throw (SynfigToolException&);

	synfig::RendDesc extract_renddesc(synfig::RendDesc& renddesc);

	synfig::TargetParam extract_targetparam() throw (SynfigToolException&);
private:
	void extract_canvas_info(Job& job);

	boost::program_options::variables_map _vm;
	boost::program_options::options_description _po_visible;
};

#endif // __SYNFIG_OPTIONSPROCESSOR_H
