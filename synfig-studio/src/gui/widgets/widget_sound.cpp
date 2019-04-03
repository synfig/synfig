
#include <thread>
#include <iostream>

#include <sys/wait.h>

#include <fmt/format.h>
#include <core/audio.h>
#include <core/node/abstract_node.h>
#include <core/os/fork_pipe.h>
//#include <generic/node_editor.h>
//#include <generic/timeline_editor.h>
#include <widgets/timeline_area.h>
#include <util/strings.h>
#include <canvasview.h>
#include<synfig-core/src/synfig/soundprocessor.h>
#include <widget_sound.h>
#include <QGraphicsPixmapItem>
#include <cstring>

//using namespace fmt::literals;
using namespace studio;

#ifdef _WIN32
#	ifdef SOUND_DIR
#		undef SOUND_DIR
#		define SOUND_DIR "share\\synfig\\sounds"
#	endif
#endif
#ifndef SOUND_DIR
#	define SOUND_DIR "/usr/local/share/synfig/sounds"
#endif
#define ETL_DIRECTORY_SEPARATOR	'/'

Widget_Sound:: Widget_Sound(): range_adjustment(Gtk::Adjustment::create(-1.0, -2.0, 2.0, 0.1, 0.1, 2))
{

	
	set_size_request(64, 64);

	range_adjustment->signal_changed().connect(
		sigc::mem_fun(*this, &Widget_Sound::queue_draw) );
	range_adjustment->signal_value_changed().connect(
		sigc::mem_fun(*this, &Widget_Sound::queue_draw) );

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK);
}
Widget_Sound::virtual ~Widget_Sound() {
        if (read_thread_1.joinable())
            read_thread_1.join();
        if (read_thread_2.joinable())
            read_thread_2.join();

set_time_model(etl::handle<TimeModel>());
    }

 void
Widget_Sound::    setup_canvas() override {
        node_update();
    }

 void 
Widget_Sound::   node_update() override {


        if (auto canvas = get_canvas()) {
            if (auto file_path = get_path() ) {
                
                    generate_waveform(file_path) ;
                
            } else {
                canvas->set_background_image({});
            }
        }
    }

 string 
Widget_Sound::   get_path() {
      /*  if (auto audio_value = get_node_as<core::Audio>()) {
            auto node = dynamic_cast<core::AbstractNode*>(audio_value.get());
            if (node == nullptr)
                return "";
            try {
                auto path_node = node->get_property_as<string>("file_path");
                if (path_node == nullptr)
                    return "";
                return path_node->value(get_core_context());
            } catch (core::NodeAccessError) {
                return "";
            }
        } else {
            return "";
        }*/

#ifdef _WIN32
	String path_to_sounds = FileSystem::fix_slashes(etl::dirname(basepath)) // how to get base path?not clear yet
		+ ETL_DIRECTORY_SEPARATOR + SOUND_DIR;
#else
	String path_to_sounds = SOUND_DIR;
	return path_to_sounds;
    }

void
Widget_Sound::  generate_waveform(string const& file_path) {
        FILE* pipe_input; // unused
        FILE* pipe_output;
#warning "it isn't very safe to pass unchecked string to external program"
        auto pid = fork_pipe(pipe_input, pipe_output, {"/usr/bin/env", "ffprobe", "-i", file_path, "-show_entries", "format=duration", "-v", "quiet", "-of", "csv=p=0"});
        cached_path = file_path;
        read_thread_1 = std::thread([this, pipe_output, pid]() {
            int status;
            waitpid(pid, &status, 0);
            if (status != 0)
                return;
            double duration;
            if (fscanf(pipe_output, "%lf", &duration) < 1)
                return;
            if (duration <= 0)
                return;
            generate_waveform_with_duration(duration);
        });
    }
Widget_Sound::    void generate_waveform_with_duration(double duration) {
        FILE* pipe_input; // unused
        FILE* pipe_output; // unused
#warning "it isn't very safe to pass unchecked string to external program"
        auto pid = fork_pipe(pipe_input, pipe_output, {"/usr/bin/env", "ffmpeg", "-i", cached_path, "-filter_complex", "showwavespic=s={}x{}:colors=black|gray"_format(int(duration*pixels_per_second), 80), "-frames:v", "1", "-y", cached_path+".png"});
        if (read_thread_2.joinable())
            read_thread_2.detach();
        read_thread_2 = std::thread([this, pid]() {
            int status;
            waitpid(pid, &status, 0);
            if (status == 0)
                load_waveform();
        });
    }

 void
Widget_Sound::  load_waveform() {
        QPixmap pixmap;
        pixmap.load(util::str(cached_path+".png"));
//couln't find function code for below 2 lines in synfig
        get_canvas()->set_background_image(pixmap);
        get_canvas()->set_bg_transform(QTransform().scale(1.0/pixels_per_second, 1));
    }


//REGISTER_CANVAS_EDITOR(TimelineArea, Widget_Sound, core::Audio);

