from conans import ConanFile, Meson, tools
from conan.tools.files import rename
from conans.errors import ConanInvalidConfiguration
from conan.tools.microsoft import is_msvc
import os
import fileinput
import shlex

required_conan_version = ">=1.50.0"


class GtkConan(ConanFile):
    name = "gtk"
    description = "libraries used for creating graphical user interfaces for applications."
    topics = ("gtk", "widgets")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://www.gtk.org"
    license = "LGPL-2.1-or-later"
    generators = "pkg_config"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_wayland": [True, False],
        "with_x11": [True, False],
        "with_cups": [True, False],
        "with_cloudprint": [True, False]
        }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_wayland": False,
        "with_x11": True,
        "with_cups": False,
        "with_cloudprint": False
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
            del self.options.with_wayland
            del self.options.with_x11

    def validate(self):
        if self.settings.compiler == "gcc" and tools.Version(self.settings.compiler.version) < "5":
            raise ConanInvalidConfiguration("this recipes does not support GCC before version 5. contributions are welcome")
        if tools.Version(self.version) >= "4.1.0":
            if not self.options.shared:
                raise ConanInvalidConfiguration("gtk supports only shared since 4.1.0")
        if self.settings.os == "Linux" and (self.options.with_wayland or self.options.with_x11):
            if not self.options["pango"].with_xft:
                raise ConanInvalidConfiguration("gtk requires pango with freetype when built with wayland/x11 support")

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

    def build_requirements(self):
        self.build_requires("meson/0.63.1")
        self.build_requires("pkgconf/1.7.4")

    def requirements(self):
        # FIXME: remove this once dependency mismatch is resolved
        self.requires("zlib/1.2.12")
        self.requires("expat/2.4.8")

        self.requires("gdk-pixbuf/2.42.6")
        self.requires("glib/2.73.0")
        self.requires("cairo/1.17.4")
        if self.settings.os == "Linux":
            self.requires("at-spi2-atk/2.38.0")
            if self.options.with_wayland:
                self.requires("wayland/1.20.0")
                self.requires("xkbcommon/1.4.0")
            if self.options.with_x11:
                self.requires("xorg/system")
        self.requires("libepoxy/1.5.10")
        self.requires("pango/1.50.7")
        self.requires("atk/2.38.0")

    def source(self):
        tools.get(**self.conan_data["sources"][self.version], strip_root=True, destination=self._source_subfolder)

    def _configure_meson(self):
        meson = Meson(self)
        defs = {}
        if self.settings.os == "Linux":
            defs["wayland_backend"] = "true" if self.options.with_wayland else "false"
            defs["x11_backend"] = "true" if self.options.with_x11 else "false"
        defs["introspection"] = "false"
        defs["gtk_doc"] = "false"
        defs["man"] = "false"
        defs["tests"] = "false"
        defs["examples"] = "false"
        defs["demos"] = "false"
        defs["datadir"] = os.path.join(self.package_folder, "res", "share")
        defs["localedir"] = os.path.join(self.package_folder, "res", "share", "locale")
        defs["sysconfdir"] = os.path.join(self.package_folder, "res", "etc")

        args=[]
        args.append("--wrap-mode=nofallback")
        meson.configure(defs=defs, build_folder=self._build_subfolder, source_folder=self._source_subfolder, pkg_config_paths=[self.install_folder], args=args)
        return meson

    def build(self):
        tools.replace_in_file(os.path.join(
            self._source_subfolder, "build-aux", "meson", "post-install.py"), "pkg-config", "pkgconf")
        tools.replace_in_file(os.path.join(self._source_subfolder, "meson.build"), "\ntest(\n", "\nfalse and test(\n")
        if "4.2.0" <= tools.Version(self.version) < "4.6.1":
            tools.replace_in_file(os.path.join(self._source_subfolder, "meson.build"),
                                  "gtk_update_icon_cache: true",
                                  "gtk_update_icon_cache: false")
        if "4.6.2" <= tools.Version(self.version):
            tools.replace_in_file(os.path.join(self._source_subfolder, "meson.build"),
                                  "dependency(is_msvc_like ? ",
                                  "dependency(false ? ")
        with tools.environment_append(tools.RunEnvironment(self).vars):
            meson = self._configure_meson()

            build_folder = os.path.join(os.getcwd(), self._build_subfolder).replace("\\", "/")
            for line in fileinput.input(os.path.join(build_folder, "build.ninja"), inplace=True):
                # the command response file for linking gtk exceeds msvc linker limit
                # this hack fixes this by removing duplicates in the link arguments
                # inside the build.ninja file
                # see https://docs.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk1170?view=msvc-170
                idx = line.find("LINK_ARGS =")
                if idx >= 0:
                    parts = shlex.split(line[idx + len("LINK_ARGS ="):])
                    print("{} {}".format(
                        line[: idx + len("LINK_ARGS =")],
                        " ".join((f'"{v}"' for v in dict.fromkeys(parts)))), end='')
                    continue

                # in windows, ninja uses CreateProcess which has a limit of 32767 chars
                # gnome.mkenums uses absolute paths, which increases the command length
                # see: https://github.com/mesonbuild/meson/issues/6710
                # this hack replaces all absolute paths of the build subfolder
                # with relative paths to decrease the length of the command
                idx = line.find("COMMAND = ")
                if idx >= 0:
                    parts = shlex.split(line[idx + len("COMMAND ="):])
                    print("{} {}".format(
                        line[: idx + len("COMMAND =")],
                        " ".join((f'"{v.replace(build_folder, ".")}"' for v in parts))))
                    continue

                print(line, end='')

            meson.build()

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses", src=self._source_subfolder)
        meson = self._configure_meson()
        with tools.environment_append({**tools.RunEnvironment(self).vars,
                                       "PKG_CONFIG_PATH": self.install_folder}):
            meson.install()


        if is_msvc(self):
            tools.remove_files_by_mask(
                os.path.join(self.package_folder, "bin"), "*.pdb")
            if not self.options.shared:
                ver = '3'
                rename(
                    self,
                    os.path.join(self.package_folder, "lib",
                                 f"libgtk-{ver}.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"gtk-{ver}.lib"),
                )
                rename(
                    self,
                    os.path.join(self.package_folder, "lib",
                                 f"libgdk-{ver}.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"gdk-{ver}.lib"),
                )
                rename(
                    self,
                    os.path.join(self.package_folder, "lib",
                                 f"libgailutil-{ver}.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"gailutil-{ver}.lib"),
                )

        self.copy(pattern="COPYING", src=self._source_subfolder, dst="licenses")
        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))
        tools.remove_files_by_mask(os.path.join(self.package_folder, "bin"), "*.pdb")
        tools.remove_files_by_mask(os.path.join(self.package_folder, "lib"), "*.pdb")

    def package_info(self):
        self.cpp_info.resdirs = ["res"]

        self.cpp_info.components["gdk-3.0"].libs = ["gdk-3"]
        self.cpp_info.components["gdk-3.0"].includedirs = [os.path.join("include", "gtk-3.0")]
        self.cpp_info.components["gdk-3.0"].requires = []
        self.cpp_info.components["gdk-3.0"].requires.extend(["pango::pango_", "pango::pangocairo"])
        self.cpp_info.components["gdk-3.0"].requires.append("gdk-pixbuf::gdk-pixbuf")
        self.cpp_info.components["gdk-3.0"].requires.extend(["cairo::cairo", "cairo::cairo-gobject"])
        if self.settings.os == "Linux":
            self.cpp_info.components["gdk-3.0"].requires.extend(["glib::gio-unix-2.0", "cairo::cairo-xlib"])
            if self.options.with_x11:
                self.cpp_info.components["gdk-3.0"].requires.append("xorg::xorg")
        self.cpp_info.components["gdk-3.0"].requires.append("libepoxy::libepoxy")
        self.cpp_info.components["gdk-3.0"].names["pkg_config"] = "gdk-3.0"

        if self.settings.os == "Windows":
            self.cpp_info.components["gdk-3.0"].system_libs = [
                    "imm32", "gdi32", "shell32", "ole32", "winmm", "dwmapi",
                    "setupapi", "cfgmgr32", "hid", "winspool", "comctl32",
                    "comdlg32"
            ]

        self.cpp_info.components["gtk+-3.0"].libs = ["gtk-3"]
        self.cpp_info.components["gtk+-3.0"].requires = ["gdk-3.0", "atk::atk"]
        self.cpp_info.components["gtk+-3.0"].requires.extend(["cairo::cairo", "cairo::cairo-gobject"])
        self.cpp_info.components["gtk+-3.0"].requires.extend(["gdk-pixbuf::gdk-pixbuf", "glib::gio-2.0"])
        if self.settings.os == "Linux":
            self.cpp_info.components["gtk+-3.0"].requires.append("at-spi2-atk::at-spi2-atk")
            self.cpp_info.components["gtk+-3.0"].requires.append("glib::gio-unix-2.0")
            if self.options.with_wayland:
                self.cpp_info.components["gtk+-3.0"].requires.append("xkbcommon::libxkbcommon")
        if (self.settings.os == "Linux" and
            (self.options.with_wayland
             or self.options.with_x11)) or self.options["pango"].with_xft:
            self.cpp_info.components["gtk+-3.0"].requires.append("pango::pangoft2")
        self.cpp_info.components["gtk+-3.0"].requires.append("libepoxy::libepoxy")
        self.cpp_info.components["gtk+-3.0"].includedirs = [os.path.join("include", "gtk-3.0")]
        self.cpp_info.components["gtk+-3.0"].names["pkg_config"] = "gtk+-3.0"

        self.cpp_info.components["gail-3.0"].libs = ["gailutil-3"]
        self.cpp_info.components["gail-3.0"].requires = ["gtk+-3.0", "atk::atk"]
        self.cpp_info.components["gail-3.0"].includedirs = [os.path.join("include", "gail-3.0")]
        self.cpp_info.components["gail-3.0"].names["pkg_config"] = "gail-3.0"

        if (self.settings.os == "Linux"
                and (self.options.with_wayland or self.options.with_x11)):
            self.cpp_info.components["gtk+-unix-print-3.0"].includedirs = [
                os.path.join("include", "gtk-3.0", "unix-print")
            ]
            self.cpp_info.components["gtk+-unix-print-3.0"].requires = [
                "gtk+-3.0"
            ]
            self.cpp_info.components["gtk+-unix-print-3.0"].set_property(
                "pkg_config_custom_content",
                "targets=broadway wayland x11\n"
                "gtk_binary_version=3.0.0\n"
                "gtk_host=x86_64-linux")

        # FIXME: remove once dependency mismatch is solved
        self.cpp_info.components["gtk+-3.0"].requires.extend(["zlib::zlib", "expat::expat"])
