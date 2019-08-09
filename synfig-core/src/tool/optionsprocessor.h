/* === S Y N F I G ========================================================= */
/*!	\file tool/optionsprocessor.h
**	\brief Synfig Tool Options Processor Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2014 Diego Barrios Romero
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

#include <string>
#include <vector>
#include <synfig/canvas.h>
//#include <boost/program_options.hpp>

// TODO rename to CommandLineHandler and move the options creation inside.
/// Class to process all the command line options
/*class OptionsProcessor
{
public:
	OptionsProcessor(boost::program_options::variables_map& vm,
					 const boost::program_options::options_description& po_visible);

#ifdef _DEBUG
	void process_debug_options() throw (SynfigToolException&);
#endif

	/// Settings options
	/// verbose, quiet, threads, benchmarks
	void process_settings_options();

	/// Information options
	/// Options that will only display information
	void process_info_options();

	/// Extract the necessary options to create a job
	/// After this, it is necessary to overwrite the necessary RendDesc options
	/// and set the target parameters, if provided. Then can be processed
	Job extract_job();

	/// Overwrite the input RendDesc object with the options given in the command line
	synfig::RendDesc extract_renddesc(const synfig::RendDesc& renddesc);

	/// Extract the target parameters from the options given in the command line
	/// video-codec, bitrate, sequence-separator
	synfig::TargetParam extract_targetparam();

	void print_target_video_codecs_help() const;

private:
	/// Determine which parameters to show in the canvas info
	/// canvas-info
	void extract_canvas_info(Job& job);

	boost::program_options::variables_map _vm;
	boost::program_options::options_description _po_visible;

	struct VideoCodec
	{
		VideoCodec(const std::string& name_, const std::string& description_)
			: name(name_), description(description_)
		{ }

		std::string name, description;
	};
	//! \warning These codecs are linked to the filename extensions for
	//!  mod_ffmpeg. If you change this you must change the others accordingly.
	//!
	std::vector<VideoCodec> _allowed_video_codecs;
};*/

class SynfigCommandLineParser
{
public:
	SynfigCommandLineParser();
	bool parse(int argc, char* argv[]);

	/// Settings options
	/// verbose, quiet, threads, benchmarks
	void process_settings_options();

	/// Trivial information options
	/// Options that will only display information
	/// and don't need to load modules
	void process_trivial_info_options();

	/// Information options
	/// Options that will only display information
	void process_info_options();

	/// Extract the necessary options to create a job
	/// After this, it is necessary to overwrite the necessary RendDesc options
	/// and set the target parameters, if provided. Then can be processed
	Job extract_job();

	/// Overwrite the input RendDesc object with the options given in the command line
	synfig::RendDesc extract_renddesc(const synfig::RendDesc& renddesc);

	/// Extract the target parameters from the options given in the command line
	/// video-codec, bitrate, sequence-separator
	synfig::TargetParam extract_targetparam();

	/// Determine which parameters to show in the canvas info
	/// canvas-info
	void extract_canvas_info(Job& job);

	void print_target_video_codecs_help() const;

#ifdef _DEBUG
	void process_debug_options() throw (SynfigToolException&);
#endif

private:

	template<typename T>
	void add_option(Glib::OptionGroup& og, const std::string& name, const gchar& short_name, T& entry, const std::string& description,
		const Glib::ustring& arg_description);
	// we need explicit method in case of different string/filename encodings
	void add_option_filename(Glib::OptionGroup& og, const std::string& name, const gchar& short_name, std::string& entry, const std::string& description, const Glib::ustring& arg_description);
	
	Glib::OptionContext context;
	Glib::OptionGroup og_set;
	Glib::OptionGroup og_switch;
	Glib::OptionGroup og_misc;
	Glib::OptionGroup og_ffmpeg;
	Glib::OptionGroup og_info;
#ifdef _DEBUG	
	Glib::OptionGroup og_debug;
#endif


	// Settings group
	Glib::ustring	set_target;
	int				set_width;
	int				set_height;
	int				set_span;
	int				set_antialias;
	int				set_quality;
//			(",Q", quality_arg_desc->default_value(DEFAULT_QUALITY), )
	double			set_gamma;
	int				set_num_threads;
	Glib::ustring	set_input_file;
	Glib::ustring	set_output_file;
	Glib::ustring	set_sequence_separator;
	Glib::ustring	set_canvas_id;
	double			set_fps;
	Glib::ustring	set_time;
	Glib::ustring	set_begin_time;
	Glib::ustring	set_start_time;
	Glib::ustring	set_end_time;
	double			set_dpi;
	double			set_dpi_x;
	double			set_dpi_y;

	// Switch group
	int				sw_verbosity;
	bool			sw_quiet;
	bool			sw_print_benchmarks;
	bool			sw_extract_alpha;

	// Misc group
	std::string		misc_append_filename;
	Glib::ustring	misc_canvas_info;
	bool			misc_canvases;

	//FFMPEG group
	Glib::ustring	video_codec;
	int				video_bitrate;

	// Synfig info group
	bool			show_help;
	bool			show_importers;
	bool			show_build_info;
	bool			show_layers_list;
	Glib::ustring	show_layer_info;
	bool			show_license;
	bool			show_modules;
	bool			show_targets;
	bool			show_codecs;
	bool			show_value_nodes;
	bool			show_version;

	// Debug group
#ifdef _DEBUG
	bool			debug_guid;
	bool			debug_signal;
#endif
	
	Glib::OptionGroup::vecustrings remaining_options_list;

	struct VideoCodec
	{
		VideoCodec(const std::string& name_, const std::string& description_)
			: name(name_), description(description_)
		{ }

		std::string name, description;
	};
	/*! \warning These codecs are linked to the filename extensions for
	 *  mod_ffmpeg. If you change this you must change the others accordingly.
	 */
	std::vector<VideoCodec> _allowed_video_codecs;


};


#endif // __SYNFIG_OPTIONSPROCESSOR_H
