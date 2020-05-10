
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include <glibmm.h>

#include <gtkmm/application.h>

#include <synfig/general.h>
#include <synfig/main.h>
#include <synfig/loadcanvas.h>
#include <synfig/filesystemnative.h>

#include "visualizationwindow.h"


using namespace synfig;
typedef std::map<String, rendering::Renderer::Handle> RendererMap;


const char commandname[] = "synfigplayer";


class TestCallback: public ProgressCallback {
public:
	virtual bool task(const String &task)
		{ synfig::info("%s", task.c_str()); return true; }
	virtual bool error(const String &task)
		{ synfig::error("%s", task.c_str()); return true; }
	virtual bool warning(const String &task)
		{ synfig::warning("%s", task.c_str()); return true; }
};


void print_renderers(const RendererMap& renderers) {
	std::cout << "available renderers: " << std::endl;
	for(RendererMap::const_iterator i = renderers.begin(); i != renderers.end(); ++i)
		if (i->second)
			std::cout << "  " << i->first << " - " << i->second->get_name() << std::endl;
}


int main(int argc, char **argv)
{

	//// init


#ifdef _WIN32
	//  Enable standard input/output on Windows
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CON", "r", stdin);
		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
	}
#endif

	Glib::init();
	bool r_time;

	String binary_path = get_binary_path(argv[0]);
	String base_dir = etl::dirname(binary_path);
	TestCallback callback;

	Main main(base_dir, &callback);

	info("Visualization test");

	// copy args
	std::vector<char*> args(argv, argv + argc);
	argv = &args.front();


	//// parse command line


	const RendererMap& renderers = rendering::Renderer::get_renderers();
	
	if (argc < 3) {
		std::cout << std::endl;
		std::cout << "usage: " << std::endl;
		std::cout << "  " << commandname << " <file.sif|file.sifz> <renderer>" << std::endl;
		std::cout << "Options:"<<std::endl;
		std::cout << "  --benchmark - Ignore real-time synchronization and render every frame (used for benchmarks)."<<std::endl;
		std::cout << std::endl;
		print_renderers(renderers);
		return 0;
	}

	const String filename = argv[1];
	info("filename: %s", filename.c_str());

	const String renderer_name = argv[2];
	info("renderer_name: %s", renderer_name.c_str());

	// remove processed args from argv
	args.erase(args.begin() + 1);
	args.erase(args.begin() + 1);
	argc = (int)args.size();

	//// get renderer


	RendererMap::const_iterator ri = renderers.find(renderer_name);
	if (ri == renderers.end() || !ri->second) {
		error("unknown renderer: %s", renderer_name.c_str());
		print_renderers(renderers);
		return 1;
	}
	rendering::Renderer::Handle renderer = ri->second;


	//// get canvas


	String errors, warnings;
	Canvas::Handle canvas = open_canvas_as(
		FileSystemNative::instance()->get_identifier(filename),
		filename,
		errors,
		warnings );
	if (!canvas)
		return 1;


	//// run Gtk::Application


	info("create Gtk::Application");
	std::cout<<args[1]<<" "<<(std::string(args[1])=="--benchmark")<<" "<<argc<<std::endl;

	r_time = !(argc==2 && std::string(args[1])=="--benchmark");

	if(argc==2)
		argc--;
	std::cout<<r_time<<std::endl;


	Glib::RefPtr<Gtk::Application> application = Gtk::Application::create(argc, argv);
	
	info("create window");
	VisualizationWindow window(canvas, renderer,r_time);
	
	info("run");
	int result = application->run(window);
	
	if (result) error("Gtk::Application finished with error code: %d", result);
	info("end");
	return result;
}

