/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_soundwave.h
**	\brief Widget for display a sound wave time-graph
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_WIDGET_SOUNDWAVE_H
#define SYNFIG_STUDIO_WIDGET_SOUNDWAVE_H

#include <gui/widgets/widget_timegraphbase.h>
#include <gui/selectdraghelper.h>

namespace studio {

class Widget_SoundWave : public Widget_TimeGraphBase
{
public:
	Widget_SoundWave();
	virtual ~Widget_SoundWave() override;

	bool load(const std::string &filename);
	void clear();

	std::string get_filename() const { return filename; }

	// what sound channel to display
	void set_channel_idx(int new_channel_idx);
	// what sound channel is being displayed
	int get_channel_idx() const;
	// how many sound channels are available
	int get_channel_number() const;

	void set_delay(synfig::Time delay);
	const synfig::Time& get_delay() const;

	virtual void set_time_model(const etl::handle<TimeModel> &x) override;

	sigc::signal<void, const std::string&> & signal_file_loaded() { return signal_file_loaded_; }

protected:
	bool on_event(GdkEvent *event) override;
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

	void on_time_model_changed() override;

private:
	std::mutex mutex;
	std::string filename;

	// sound data
	unsigned char * buffer;
	unsigned long buffer_length;

	// sound format
	int frequency;
	int n_channels;
	int n_samples;

	// user settings
	synfig::Time sound_delay;
	int channel_idx;

	// status
	bool loading_error;
	synfig::Time previous_lower_time;
	synfig::Time previous_upper_time;

	sigc::signal<void, const std::string&> signal_file_loaded_;

	void setup_mouse_handler();

	bool do_load(const std::string& filename);

	// I'm too lazy to code/copy again mouse actions for panning/zooming/scrolling
	struct MouseHandler : SelectDragHelper<int>
	{
		// SelectDragHelper interface
	public:
		MouseHandler() : SelectDragHelper<int>("sound-no-drag") {}
		virtual ~MouseHandler() override;
		virtual void get_item_position(const int& , Gdk::Point& ) override {}
		virtual bool find_item_at_position(int, int, int&) override { return false; }
		virtual bool find_items_in_rect(Gdk::Rectangle, std::vector<int>&) override { return false; }
		virtual void get_all_items(std::vector<int>&) override {}
		virtual void delta_drag(int, int, bool) override {}
	} mouse_handler;

};

}

#endif // SYNFIG_STUDIO_WIDGET_SOUNDWAVE_H
