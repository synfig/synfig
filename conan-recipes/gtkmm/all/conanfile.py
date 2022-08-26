from conans import ConanFile, Meson, tools
from conan.tools.files import rename
from conan.tools.microsoft import is_msvc
import os
import shutil


class GtkmmConan(ConanFile):
    name = "gtkmm"
    description = "gtkmm is a GUI toolkit and nothing more, and it strives to be the best C++ GUI toolkit."
    topics = "gui", "gtk", "widgets", "wrapper"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://www.gtkmm.org/"
    license = "LGPL-2.1"
    generators = "pkg_config"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_atkmm": [True, False],
        "with_x11": [True, False],
        "build_demos": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_atkmm": False,
        "build_demos": False,
        "with_x11": False
    }

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

        if self.settings.os != "Linux":
            del self.options.with_x11

    def build_requirements(self):
        self.build_requires("meson/0.62.1")
        self.build_requires("pkgconf/1.7.4")

    def requirements(self):
        self.requires("gtk/3.24.34")
        self.requires("glibmm/2.66.4")
        self.requires("cairomm/1.14.3")
        self.requires("pangomm/2.46.2")
        self.requires("gdk-pixbuf/2.42.6")
        if self.options.with_atkmm:
           self.requires("atkmm/2.28.2")
        if self.options.build_demos:
           self.requires("epoxy/1.5.10")

    def source(self):
        tools.get(**self.conan_data["sources"][self.version], strip_root=True, destination=self._source_subfolder)

    def _configure_meson(self):
        meson = Meson(self)
        defs = {
            "build-atkmm-api": "true" if self.options.with_atkmm else "false",
            "build-x11-api": "true" if "with_x11" in self.options and self.options.with_x11 else "false",
            "build-demos": "true" if self.options.build_demos else "false",
            "build-tests": "false",
            "build-documentation": "false",
            "msvc14x-parallel-installable": "false",
        }

        meson.configure(
            defs=defs,
            build_folder=self._build_subfolder,
            source_folder=self._source_subfolder,
            pkg_config_paths=[self.install_folder],
        )

        return meson

    def _patch_sources(self):
        # glibmm_generate_extra_defs library does not provide any standard way
        # for discovery, which is why pangomm uses "find_library" method instead
        # of "dependency". this patch adds a hint to where this library is
        glibmm_generate_extra_defs_dir = [
            os.path.join(self.deps_cpp_info["glibmm"].rootpath, libdir) for
            libdir in self.deps_cpp_info["glibmm"].libdirs]

        tools.replace_in_file(
            os.path.join(self._source_subfolder, "tools",
                         "extra_defs_gen", "meson.build"),
            "required: glibmm_dep.type_name() != 'internal',",
            f"required: glibmm_dep.type_name() != 'internal', dirs: {glibmm_generate_extra_defs_dir}")


    def build(self):
        self._patch_sources()

        with tools.environment_append(tools.RunEnvironment(self).vars):
            meson = self._configure_meson()
            meson.build()

    def package(self):
        self.copy("COPYING", dst="licenses", src=self._source_subfolder)
        meson = self._configure_meson()
        meson.install()

        shutil.move(
            os.path.join(self.package_folder, "lib",
                         f"gtkmm-3.0", "include",
                         "gtkmmconfig.h"),
            os.path.join(self.package_folder, "include",
                         f"gtkmm-3.0", "gtkmmconfig.h"))
        shutil.move(
            os.path.join(self.package_folder, "lib",
                         f"gdkmm-3.0", "include",
                         "gdkmmconfig.h"),
            os.path.join(self.package_folder, "include",
                         f"gdkmm-3.0", "gdkmmconfig.h"))

        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))
        tools.rmdir(
            os.path.join(self.package_folder, "lib",
                         "gtkmm-3.0", "include"))
        tools.rmdir(
            os.path.join(self.package_folder, "lib",
                         "gdkmm-3.0", "include"))

        if is_msvc(self):
            tools.remove_files_by_mask(
                os.path.join(self.package_folder, "bin"), "*.pdb")
            if not self.options.shared:
                rename(
                    self,
                    os.path.join(self.package_folder, "lib",
                                 f"libgtkmm-3.0.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"gtkmm-3.0.lib"),
                )
                rename(
                    self,
                    os.path.join(self.package_folder, "lib",
                                 f"libgdkmm-3.0.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"gdkmm-3.0.lib"),
                )

    def package_info(self):
        self.cpp_info.components["gdkmm-3.0"].names["pkg-config"] = "gdkmm-3.0"
        self.cpp_info.components["gdkmm-3.0"].set_property(
            "pkg_config_name", "gdkmm-3.0")
        self.cpp_info.components["gdkmm-3.0"].libs = ["gdkmm-3.0"]
        self.cpp_info.components["gdkmm-3.0"].includedirs = [
                os.path.join("include", "gdkmm-3.0")
        ]
        self.cpp_info.components["gdkmm-3.0"].requires = [
            "glibmm::giomm-2.4", "gtk::gtk+-3.0", "cairomm::cairomm-1.0",
            "pangomm::pangomm-1.4", "gdk-pixbuf::gdk-pixbuf"
        ]

        self.cpp_info.components["gtkmm-3.0"].names["pkg-config"] = "gtkmm-3.0"
        self.cpp_info.components["gtkmm-3.0"].set_property(
            "pkg_config_name", "gtkmm-3.0")
        self.cpp_info.components["gtkmm-3.0"].libs = ["gtkmm-3.0"]
        self.cpp_info.components["gtkmm-3.0"].includedirs = [
                os.path.join("include", "gtkmm-3.0")
        ]
        self.cpp_info.components["gtkmm-3.0"].requires = [
            "glibmm::giomm-2.4", "gtk::gtk+-3.0", "cairomm::cairomm-1.0",
            "pangomm::pangomm-1.4", "gdk-pixbuf::gdk-pixbuf", "gdkmm-3.0"
        ]
