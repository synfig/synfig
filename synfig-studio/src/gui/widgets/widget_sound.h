
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


//using namespace fmt::literals;

namespace studio {

class Widget_Sound
{

public:
    Widget_Sound() ;
    virtual ~Widget_Sound() ;

    void setup_canvas() override ;

    void node_update() override ;

private:
    string get_path() ;

    void generate_waveform(string const& file_path) ;
    void generate_waveform_with_duration(double duration) ;
    void load_waveform() ;

private:
    string cached_path;
    const double pixels_per_second = 48;
    std::thread read_thread_1;
    std::thread read_thread_2;
};

//REGISTER_CANVAS_EDITOR(TimelineArea, Widget_Sound, core::Audio);

}
