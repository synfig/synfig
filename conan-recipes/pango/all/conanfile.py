import os
import shutil
import glob
import functools

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.meson import Meson, MesonToolchain
from conan.tools.gnu import PkgConfigDeps
from conan.tools import (
    files,
    microsoft,
    scm
)
from conans import tools

required_conan_version = ">=1.50.2"


class PangoConan(ConanFile):
    name = "pango"
    license = "LGPL-2.0-and-later"
    url = "https://github.com/conan-io/conan-center-index"
    description = "Internationalized text layout and rendering library"
    homepage = "https://www.pango.org/"
    topics = ("conan", "fontconfig", "fonts", "freedesktop")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_libthai": [True, False],
        "with_cairo": [True, False],
        "with_xft": [True, False, "auto"],
        "with_freetype": [True, False, "auto"],
        "with_fontconfig": [True, False, "auto"]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_libthai": False,
        "with_cairo": True,
        "with_xft": "auto",
        "with_freetype": "auto",
        "with_fontconfig": "auto"
    }
    _autotools = None

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def layout(self):
        self.folders.source = self._source_subfolder
        self.folders.build = self._build_subfolder

    def validate(self):
        if self.settings.compiler == "gcc" and scm.Version(self.settings.compiler.version) < "5":
            raise ConanInvalidConfiguration(
                "this recipe does not support GCC before version 5. contributions are welcome")
        if self.options.with_xft and not self.settings.os in ["Linux", "FreeBSD"]:
            raise ConanInvalidConfiguration("Xft can only be used on Linux and FreeBSD")

        if self.options.with_xft and (not self.options.with_freetype or not self.options.with_fontconfig):
            raise ConanInvalidConfiguration("Xft requires freetype and fontconfig")

        if self.options["glib"].shared and microsoft.is_msvc_static_runtime(self):
            raise ConanInvalidConfiguration("Linking shared glib against static MSVC runtime is not supported")

        if self.options.shared and (not self.options["glib"].shared
                                    or not self.options["harfbuzz"].shared or
                                    (self.options.with_cairo
                                     and not self.options["cairo"].shared)):
            raise ConanInvalidConfiguration(
                "Linking a shared library against static glib can cause unexpected behaviour."
            )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

        if self.options.with_xft == "auto":
            self.options.with_xft = self.settings.os in ["Linux", "FreeBSD"]
        if self.options.with_freetype == "auto":
            self.options.with_freetype = not self.settings.os in ["Windows", "Macos"]
        if self.options.with_fontconfig == "auto":
            self.options.with_fontconfig = not self.settings.os in ["Windows", "Macos"]
        if self.options.shared:
            self.options["glib"].shared = True
            self.options["harfbuzz"].shared = True
            if self.options.with_cairo:
                self.options["cairo"].shared = True

    def build_requirements(self):
        self.build_requires("pkgconf/1.7.4")
        self.build_requires("meson/0.63.1")

    def requirements(self):
        if self.options.with_freetype:
            self.requires("freetype/2.12.1")

        if self.options.with_fontconfig:
            self.requires("fontconfig/2.13.93")
        if self.options.with_xft:
            self.requires("libxft/2.3.4")
        if self.options.with_xft and self.options.with_fontconfig and self.options.with_freetype:
            self.requires("xorg/system")    # for xorg::xrender
        if self.options.with_cairo:
            self.requires("cairo/1.17.4")
        self.requires("harfbuzz/4.4.1")
        self.requires("glib/2.73.3")
        self.requires("fribidi/1.0.12")

    def source(self):
        files.get(self,
                  **self.conan_data["sources"][self.version],
                  strip_root=True)

    def generate(self):
        tc = MesonToolchain(self)

        def enabled_disabled(v):
            return "enabled" if v else "disabled"

        tc.project_options["introspection"] = "disabled"
        tc.project_options["libthai"] = enabled_disabled(self.options.with_libthai)
        tc.project_options["cairo"] = enabled_disabled(self.options.with_cairo)
        tc.project_options["xft"] = enabled_disabled(self.options.with_xft)
        tc.project_options["fontconfig"] = enabled_disabled(self.options.with_fontconfig)
        tc.project_options["freetype"] = enabled_disabled(self.options.with_fontconfig)

        tc.generate()

        td = PkgConfigDeps(self)
        td.generate()

    @functools.lru_cache(1)
    def _configure_meson(self):
        meson = Meson(self)
        meson.configure()
        return meson

    def build(self):
        meson_build = os.path.join(self.source_folder, "meson.build")
        files.replace_in_file(self, meson_build, "subdir('tests')", "")
        files.replace_in_file(self, meson_build, "subdir('tools')", "")
        files.replace_in_file(self, meson_build, "subdir('utils')", "")
        files.replace_in_file(self, meson_build, "subdir('examples')", "")

        with tools.environment_append({"PATH": self.deps_cpp_info["glib"].bin_paths}):
            meson = self._configure_meson()
            meson.build()

    def package(self):
        self.copy(pattern="COPYING", dst="licenses", src=self.source_folder)
        with tools.environment_append({"PATH": self.deps_cpp_info["glib"].bin_paths}):
            meson = self._configure_meson()
            meson.install()
        if microsoft.is_msvc(self):
            with tools.chdir(os.path.join(self.package_folder, "lib")):
                for filename_old in glob.glob("*.a"):
                    filename_new = filename_old[3:-2] + ".lib"
                    self.output.info("rename %s into %s" % (filename_old, filename_new))
                    shutil.move(filename_old, filename_new)
        files.rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        files.rm(self, "*.pdb", self.package_folder)

    def package_info(self):
        self.cpp_info.set_property("pkg_config_name", "pango-all-do-no-use")
        self.cpp_info.names["pkg_config"] = "pango-all-do-no-use"

        self.cpp_info.components["pango_"].set_property("pkg_config_name", "pango")
        self.cpp_info.components["pango_"].names["pkg_config"] = "pango"
        self.cpp_info.components["pango_"].libs = ["pango-1.0"]

        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["pango_"].system_libs.append("m")
        self.cpp_info.components["pango_"].requires.append("glib::glib-2.0")
        self.cpp_info.components["pango_"].requires.append("glib::gobject-2.0")
        self.cpp_info.components["pango_"].requires.append("glib::gio-2.0")
        self.cpp_info.components["pango_"].requires.append("fribidi::fribidi")
        self.cpp_info.components["pango_"].requires.append("harfbuzz::harfbuzz")

        if self.options.with_fontconfig:
            self.cpp_info.components["pango_"].requires.append("fontconfig::fontconfig")

        if self.options.with_xft:
            self.cpp_info.components["pango_"].requires.append("libxft::libxft")
            # Pango only uses xrender when Xft, fontconfig and freetype are enabled
            if self.options.with_fontconfig and self.options.with_freetype:
                self.cpp_info.components["pango_"].requires.append("xorg::xrender")
        if self.options.with_cairo:
            self.cpp_info.components["pango_"].requires.append("cairo::cairo_")
        self.cpp_info.components["pango_"].includedirs = [os.path.join(self.package_folder, "include", "pango-1.0")]

        if self.options.with_freetype:
            self.cpp_info.components["pangoft2"].set_property("pkg_config_name", "pangoft2")
            self.cpp_info.components["pangoft2"].names["pkg_config"] = "pangoft2"
            self.cpp_info.components["pangoft2"].libs = ["pangoft2-1.0"]
            self.cpp_info.components["pangoft2"].requires = ["pango_", "freetype::freetype"]
            self.cpp_info.components["pangoft2"].includedirs = [os.path.join(self.package_folder, "include", "pango-1.0")]

        if self.options.with_fontconfig:
            self.cpp_info.components["pangofc"].set_property("pkg_config_name", "pangofc")
            self.cpp_info.components["pangofc"].names["pkg_config"] = "pangofc"
            if self.options.with_freetype:
                self.cpp_info.components["pangofc"].requires = ["pangoft2"]

        if self.settings.os != "Windows":
            self.cpp_info.components["pangoroot"].set_property("pkg_config_name", "pangoroot")
            self.cpp_info.components["pangoroot"].names["pkg_config"] = "pangoroot"
            if self.options.with_freetype:
                self.cpp_info.components["pangoroot"].requires = ["pangoft2"]

        if self.options.with_xft:
            self.cpp_info.components["pangoxft"].set_property("pkg_config_name", "pangoxft")
            self.cpp_info.components["pangoxft"].names["pkg_config"] = "pangoxft"
            self.cpp_info.components["pangoxft"].libs = ["pangoxft-1.0"]
            self.cpp_info.components["pangoxft"].requires = ["pango_", "pangoft2"]
            self.cpp_info.components["pangoxft"].includedirs = [os.path.join(self.package_folder, "include", "pango-1.0")]

        if self.settings.os == "Windows":
            self.cpp_info.components["pangowin32"].set_property("pkg_config_name", "pangowin32")
            self.cpp_info.components["pangowin32"].names["pkg_config"] = "pangowin32"
            self.cpp_info.components["pangowin32"].libs = ["pangowin32-1.0"]
            self.cpp_info.components["pangowin32"].requires = ["pango_"]
            self.cpp_info.components["pangowin32"].system_libs.append("gdi32")

        if self.options.with_cairo:
            self.cpp_info.components["pangocairo"].set_property("pkg_config_name", "pangocairo")
            self.cpp_info.components["pangocairo"].names["pkg_config"] = "pangocairo"
            self.cpp_info.components["pangocairo"].libs = ["pangocairo-1.0"]
            self.cpp_info.components["pangocairo"].requires = ["pango_"]
            if self.options.with_freetype:
                self.cpp_info.components["pangocairo"].requires.append("pangoft2")
            if self.settings.os == "Windows":
                self.cpp_info.components["pangocairo"].requires.append("pangowin32")
                self.cpp_info.components["pangocairo"].system_libs.append("gdi32")
            self.cpp_info.components["pangocairo"].includedirs = [os.path.join(self.package_folder, "include", "pango-1.0")]

        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))

    def package_id(self):
        if not self.options["glib"].shared:
            self.info.requires["glib"].full_package_mode()
