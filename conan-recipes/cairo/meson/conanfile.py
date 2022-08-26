import contextlib
import glob
import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools import files, microsoft
from conans import tools, Meson, VisualStudioBuildEnvironment

required_conan_version = ">=1.50.0"


class CairoConan(ConanFile):
    name = "cairo"
    description = "Cairo is a 2D graphics library with support for multiple output devices"
    topics = ("cairo", "graphics")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://cairographics.org/"
    license = ("LGPL-2.1-only", "MPL-1.1")
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_freetype": [True, False],
        "with_fontconfig": [True, False],
        "with_xlib": [True, False],
        "with_xlib_xrender": [True, False],
        "with_xcb": [True, False],
        "with_glib": [True, False],
        "with_lzo": [True, False],
        "with_zlib": [True, False],
        "with_png": [True, False],
        "with_opengl": [False, "desktop", "gles2", "gles3"],
        "with_symbol_lookup": [True, False],
        "tee": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_freetype": True,
        "with_fontconfig": True,
        "with_xlib": True,
        "with_xlib_xrender": True,
        "with_xcb": True,
        "with_glib": True,
        "with_lzo": True,
        "with_zlib": True,
        "with_png": True,
        "with_opengl": "desktop",
        "with_symbol_lookup": False,
        "tee": True,
    }

    generators = "pkg_config"

    _meson = None

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    @property
    def _settings_build(self):
        return getattr(self, "settings_build", self.settings)

    def export_sources(self):
        for patch in self.conan_data.get("patches", {}).get(self.version, []):
            self.copy(patch["patch_file"])

    def config_options(self):
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd
        if self.settings.os == "Windows":
            del self.options.fPIC
        if self.settings.os != "Linux":
            del self.options.with_xlib
            del self.options.with_xlib_xrender
            del self.options.with_xcb
            del self.options.with_symbol_lookup
        if self.settings.os in ["Macos", "Windows"]:
            del self.options.with_opengl

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        del self.settings.compiler.cppstd
        del self.settings.compiler.libcxx
        if self.options.with_glib and self.options.shared:
            self.options["glib"].shared = True

    def requirements(self):
        self.requires("pixman/0.40.0")
        if self.options.with_zlib and self.options.with_png:
            self.requires("expat/2.4.8")
        if self.options.with_lzo:
            self.requires("lzo/2.10")
        if self.options.with_zlib:
            self.requires("zlib/1.2.12")
        if self.options.with_freetype:
            self.requires("freetype/2.12.1")
        if self.options.with_fontconfig:
            self.requires("fontconfig/2.13.93")
        if self.options.with_png:
            self.requires("libpng/1.6.37")
        if self.options.with_glib:
            self.requires("glib/2.73.3")
        if self.settings.os == "Linux":
            if self.options.with_xlib or self.options.with_xlib_xrender or self.options.with_xcb:
                self.requires("xorg/system")
        if self.options.get_safe("with_opengl") == "desktop":
            self.requires("opengl/system")
            if self.settings.os == "Windows":
                self.requires("glext/cci.20210420")
                self.requires("wglext/cci.20200813")
                self.requires("khrplatform/cci.20200529")
        if self.options.get_safe("with_opengl") and self.settings.os in ["Linux", "FreeBSD"]:
            self.requires("egl/system")

    def build_requirements(self):
        self.build_requires("meson/0.63.1")
        self.build_requires("pkgconf/1.7.4")

    def validate(self):
        if self.options.get_safe("with_xlib_xrender") and not self.options.get_safe("with_xlib"):
            raise ConanInvalidConfiguration("'with_xlib_xrender' option requires 'with_xlib' option to be enabled as well!")
        if self.options.with_glib:
            if self.options["glib"].shared:
                if microsoft.is_msvc_static_runtime(self):
                    raise ConanInvalidConfiguration(
                        "Linking shared glib with the MSVC static runtime is not supported"
                    )
            elif self.options.shared:
                raise ConanInvalidConfiguration(
                    "Linking a shared library against static glib can cause unexpected behaviour."
                )

    @contextlib.contextmanager
    def _build_context(self):
        if microsoft.is_msvc(self):
            env_build = VisualStudioBuildEnvironment(self)
            if not self.options.shared:
                env_build.flags.append("-DCAIRO_WIN32_STATIC_BUILD")
                env_build.cxx_flags.append("-DCAIRO_WIN32_STATIC_BUILD")
            with tools.environment_append(env_build.vars):
                yield
        else:
            yield

    def source(self):
        files.get(self, **self.conan_data["sources"][self.version], destination=self._source_subfolder, strip_root=True)

    def _configure_meson(self):
        def boolean(value):
            return "enabled" if value else "disabled"

        meson = Meson(self)

        defs = dict()
        defs["tests"] = "disabled"
        defs["zlib"] = boolean(self.options.with_zlib)
        defs["png"] = boolean(self.options.with_png)
        defs["freetype"] = boolean(self.options.with_freetype)
        defs["fontconfig"] = boolean(self.options.with_fontconfig)
        if self.settings.os == "Linux":
            defs["xcb"] = boolean(self.options.get_safe("with_xcb"))
            defs["xlib"] = boolean(self.options.get_safe("with_xlib"))
            defs["xlib-xrender"] = boolean(self.options.get_safe("with_xlib_xrender"))
        else:
            defs["xcb"] = "disabled"
            defs["xlib"] = "disabled"
        if self.options.get_safe("with_opengl") == "desktop":
            defs["gl-backend"] = "gl"
        elif self.options.get_safe("with_opengl") == "gles2":
            defs["gl-backend"] = "glesv2"
        elif self.options.get_safe("with_opengl") == "gles3":
            defs["gl-backend"] = "glesv3"
        else:
            defs["gl-backend"] = "disabled"
        defs["glesv2"] = boolean(self.options.get_safe("with_opengl") == "gles2")
        defs["glesv3"] = boolean(self.options.get_safe("with_opengl") == "gles3")
        defs["tee"] = boolean(self.options.tee)
        defs["symbol-lookup"] = boolean(self.options.get_safe("with_symbol_lookup"))

        # future options to add, see meson_options.txt.
        # for now, disabling explicitly, to avoid non-reproducible auto-detection of system libs
        defs["cogl"] = "disabled"  # https://gitlab.gnome.org/GNOME/cogl
        defs["directfb"] = "disabled"
        defs["drm"] = "disabled" # not yet compilable in cairo 1.17.4
        defs["openvg"] = "disabled"  # https://www.khronos.org/openvg/
        defs["qt"] = "disabled" # not yet compilable in cairo 1.17.4
        defs["gtk2-utils"] = "disabled"
        defs["spectre"] = "disabled"  # https://www.freedesktop.org/wiki/Software/libspectre/

        meson.configure(
            source_folder=self._source_subfolder,
            args=["--wrap-mode=nofallback"],
            build_folder=self._build_subfolder,
            defs=defs,
        )
        return meson

    def build(self):
        files.apply_conandata_patches(self)

        # Dependency freetype2 found: NO found 2.11.0 but need: '>= 9.7.3'
        if self.options.with_freetype:
            files.replace_in_file(self, "freetype2.pc",
                                  f"Version: {self.deps_cpp_info['freetype'].version}",
                                  "Version: 9.7.3")
        with self._build_context():
            meson = self._configure_meson()
            meson.build()

    def _fix_library_names(self):
        if microsoft.is_msvc(self):
            with tools.chdir(os.path.join(self.package_folder, "lib")):
                for filename_old in glob.glob("*.a"):
                    filename_new = filename_old[3:-2] + ".lib"
                    self.output.info("rename %s into %s" % (filename_old, filename_new))
                    files.rename(self, filename_old, filename_new)

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses", src=self._source_subfolder)
        self.copy("COPYING*", src=self._source_subfolder, dst="licenses", keep_path=False)
        with self._build_context():
            meson = self._configure_meson()
            meson.install()
        self._fix_library_names()
        files.rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        files.rm(self, "*.pdb", os.path.join(self.package_folder, "bin"))

    def package_info(self):
        base_requirements = {"pixman::pixman"}
        base_system_libs = {}

        def add_component_and_base_requirements(component, requirements, system_libs=None):
            self.cpp_info.components[component].set_property("pkg_config_name", component)
            self.cpp_info.components[component].names["pkg_config"] = component
            self.cpp_info.components[component].requires += ["cairo_"] + requirements
            base_requirements.update(set(requirements))
            if system_libs is not None:
                self.cpp_info.components[component].system_libs += system_libs
                base_system_libs.update(set(system_libs))

        self.cpp_info.set_property("pkg_config_name", "cairo-all-do-no-use")
        self.cpp_info.names["pkg_config"] = "cairo-all-do-no-use"
        self.cpp_info.components["cairo_"].libs = ["cairo"]
        self.cpp_info.components["cairo_"].includedirs.insert(0, os.path.join("include", "cairo"))
        self.cpp_info.components["cairo_"].set_property("pkg_config_name", "cairo")

        if self.settings.os == "Linux":
            self.cpp_info.components["cairo_"].system_libs.extend(["m", "dl", "pthread"])
            if self.options.get_safe("with_symbol_lookup"):
                self.cpp_info.components["cairo_"].system_libs.append("bfd")
            self.cpp_info.components["cairo_"].cflags = ["-pthread"]
            self.cpp_info.components["cairo_"].cxxflags = ["-pthread"]

        if self.options.with_lzo:
            self.cpp_info.components["cairo_"].requires.append("lzo::lzo")

        if self.options.with_zlib:
            self.cpp_info.components["cairo_"].requires.append("zlib::zlib")

        if self.options.with_png:
            add_component_and_base_requirements("cairo-png", ["libpng::libpng"])
            add_component_and_base_requirements("cairo-svg", ["libpng::libpng"])

        if self.options.with_fontconfig:
            add_component_and_base_requirements("cairo-fc", ["fontconfig::fontconfig"])

        if self.options.with_freetype:
            add_component_and_base_requirements("cairo-ft", ["freetype::freetype"])

        if self.options.get_safe("with_xlib"):
            add_component_and_base_requirements("cairo-xlib", ["xorg::x11", "xorg::xext"])

        if self.options.get_safe("with_xlib_xrender"):
            add_component_and_base_requirements("cairo-xlib-xrender", ["xorg::xrender"])

        if self.options.get_safe("with_xcb"):
            add_component_and_base_requirements("cairo-xcb", ["xorg::xcb", "xorg::xcb-render"])
            add_component_and_base_requirements("cairo-xcb-shm", ["xorg::xcb", "xorg::xcb-shm"])

            if self.options.get_safe("with_xlib"):
                add_component_and_base_requirements("cairo-xlib-xcb", ["xorg::x11-xcb"])

        if tools.is_apple_os(self.settings.os):
            self.cpp_info.components["cairo-quartz"].set_property("pkg_config_name", "cairo-quartz")
            self.cpp_info.components["cairo-quartz"].names["pkg_config"] = "cairo-quartz"
            self.cpp_info.components["cairo-quartz"].requires = ["cairo_"]

            self.cpp_info.components["cairo-quartz-image"].set_property("pkg_config_name", "cairo-quartz-image")
            self.cpp_info.components["cairo-quartz-image"].names["pkg_config"] = "cairo-quartz-image"
            self.cpp_info.components["cairo-quartz-image"].requires = ["cairo_"]

            self.cpp_info.components["cairo-quartz-font"].set_property("pkg_config_name", "cairo-quartz-font")
            self.cpp_info.components["cairo-quartz-font"].names["pkg_config"] = "cairo-quartz-font"
            self.cpp_info.components["cairo-quartz-font"].requires = ["cairo_"]

            self.cpp_info.components["cairo_"].frameworks.append("CoreGraphics")

        if self.settings.os == "Windows":
            self.cpp_info.components["cairo-win32"].set_property("pkg_config_name", "cairo-win32")
            self.cpp_info.components["cairo-win32"].names["pkg_config"] = "cairo-win32"
            self.cpp_info.components["cairo-win32"].requires = ["cairo_"]

            self.cpp_info.components["cairo-win32-font"].set_property("pkg_config_name", "cairo-win32-font")
            self.cpp_info.components["cairo-win32-font"].names["pkg_config"] = "cairo-win32-font"
            self.cpp_info.components["cairo-win32-font"].requires = ["cairo_"]

            self.cpp_info.components["cairo_"].system_libs.extend(["gdi32", "msimg32", "user32"])

            if not self.options.shared:
                self.cpp_info.components["cairo_"].defines.append("CAIRO_WIN32_STATIC_BUILD=1")

        if self.options.get_safe("with_opengl"):
            if self.options.with_opengl == "desktop":
                add_component_and_base_requirements("cairo-gl", ["opengl::opengl"])

                if self.settings.os == "Linux":
                    add_component_and_base_requirements("cairo-glx", ["opengl::opengl"])

                if self.settings.os == "Windows":
                    add_component_and_base_requirements("cairo-wgl", ["glext::glext", "wglext::wglext", "khrplatform::khrplatform"])

            elif self.options.with_opengl == "gles3":
                add_component_and_base_requirements("cairo-glesv3", [], ["GLESv2"])
            elif self.options.with_opengl == "gles2":
                add_component_and_base_requirements("cairo-glesv2", [], ["GLESv2"])
            if self.settings.os in ["Linux", "FreeBSD"]:
                add_component_and_base_requirements("cairo-egl", ["egl::egl"])

        if self.options.with_zlib:
            add_component_and_base_requirements("cairo-script", ["zlib::zlib"])
            add_component_and_base_requirements("cairo-ps", ["zlib::zlib"])
            add_component_and_base_requirements("cairo-pdf", ["zlib::zlib"])
            self.cpp_info.components["cairo-script-interpreter"].set_property("pkg_config_name", "cairo-script-interpreter")
            self.cpp_info.components["cairo-script-interpreter"].names["pkg_config"] = "cairo-script-interpreter"
            self.cpp_info.components["cairo-script-interpreter"].libs = ["cairo-script-interpreter"]
            self.cpp_info.components["cairo-script-interpreter"].requires = ["cairo_"]

            if self.options.with_png:
                add_component_and_base_requirements("cairo-xml", ["zlib::zlib"])
                add_component_and_base_requirements("cairo-util_", ["expat::expat"])

        if self.options.tee:
            self.cpp_info.components["cairo-tee"].set_property("pkg_config_name", "cairo-tee")
            self.cpp_info.components["cairo-tee"].names["pkg_config"] = "cairo-tee"
            self.cpp_info.components["cairo-tee"].requires = ["cairo_"]

        # util directory
        if self.options.with_glib:
            self.cpp_info.components["cairo-gobject"].set_property("pkg_config_name", "cairo-gobject")
            self.cpp_info.components["cairo-gobject"].names["pkg_config"] = "cairo-gobject"
            self.cpp_info.components["cairo-gobject"].libs = ["cairo-gobject"]
            self.cpp_info.components["cairo-gobject"].requires = ["cairo_", "glib::gobject-2.0", "glib::glib-2.0"]

        self.cpp_info.components["cairo_"].requires += list(base_requirements)
        self.cpp_info.components["cairo_"].system_libs += list(base_system_libs)

    def package_id(self):
        if self.options.get_safe("with_glib") and not self.options["glib"].shared:
            self.info.requires["glib"].full_package_mode()
