from conan import ConanFile
from conan.tools.build import cross_building
from conan.tools.microsoft import is_msvc
from conans import tools, Meson, VisualStudioBuildEnvironment
from conans.errors import ConanInvalidConfiguration
import functools
import os
import glob
import shutil

required_conan_version = ">=1.45.0"


class GLibConan(ConanFile):
    name = "glib"
    description = "GLib provides the core application building blocks for libraries and applications written in C"
    topics = ("glib", "gobject", "gio", "gmodule")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://gitlab.gnome.org/GNOME/glib"
    license = "LGPL-2.1"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_pcre": [True, False],
        "with_elf": [True, False],
        "with_selinux": [True, False],
        "with_mount": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_pcre": True,
        "with_elf": True,
        "with_mount": True,
        "with_selinux": True,
    }

    short_paths = True
    generators = "pkg_config"

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def export_sources(self):
        self.copy("CMakeLists.txt")
        for patch in self.conan_data.get("patches", {}).get(self.version, []):
            self.copy(patch["patch_file"])

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
            if tools.Version(self.version) < "2.71.1":
                self.options.shared = True
        if self.settings.os != "Linux":
            del self.options.with_mount
            del self.options.with_selinux
        if is_msvc(self):
            del self.options.with_elf

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

    def requirements(self):
        self.requires("zlib/1.2.12")
        self.requires("libffi/3.4.2")
        if self.options.with_pcre:
            if tools.Version(self.version) >= "2.73.2":
                self.requires("pcre2/10.40")
            else:
                self.requires("pcre/8.45")
        if self.options.get_safe("with_elf"):
            self.requires("libelf/0.8.13")
        if self.options.get_safe("with_mount"):
            self.requires("libmount/2.36.2")
        if self.options.get_safe("with_selinux"):
            self.requires("libselinux/3.3")
        if self.settings.os != "Linux":
            # for Linux, gettext is provided by libc
            self.requires("libgettext/0.21")

        if tools.is_apple_os(self.settings.os):
            self.requires("libiconv/1.17")

    def validate(self):
        if hasattr(self, 'settings_build') and cross_building(self, skip_x64_x86=True):
            raise ConanInvalidConfiguration("Cross-building not implemented")
        if tools.Version(self.version) >= "2.69.0" and not self.options.with_pcre:
            raise ConanInvalidConfiguration("option glib:with_pcre must be True for glib >= 2.69.0")
        if self.settings.os == "Windows" and not self.options.shared and tools.Version(self.version) < "2.71.1":
            raise ConanInvalidConfiguration(
                "glib < 2.71.1 can not be built as static library on Windows. "
                "see https://gitlab.gnome.org/GNOME/glib/-/issues/692"
            )
        if tools.Version(self.version) < "2.67.0" and not is_msvc(self) and not self.options.with_elf:
            raise ConanInvalidConfiguration("libelf dependency can't be disabled in glib < 2.67.0")

    def build_requirements(self):
        self.build_requires("meson/0.61.2")
        self.build_requires("pkgconf/1.7.4")

    def source(self):
        tools.get(**self.conan_data["sources"][self.version], strip_root=True, destination=self._source_subfolder)

    @functools.lru_cache(1)
    def _configure_meson(self):
        meson = Meson(self)
        defs = {}
        if tools.is_apple_os(self.settings.os):
            defs["iconv"] = "external"  # https://gitlab.gnome.org/GNOME/glib/issues/1557
        defs["selinux"] = "enabled" if self.options.get_safe("with_selinux") else "disabled"
        defs["libmount"] = "enabled" if self.options.get_safe("with_mount") else "disabled"

        if tools.Version(self.version) < "2.69.0":
            defs["internal_pcre"] = not self.options.with_pcre

        if self.settings.os == "FreeBSD":
            defs["xattr"] = "false"
        if tools.Version(self.version) >= "2.67.2":
            defs["tests"] = "false"

        if tools.Version(self.version) >= "2.67.0":
            defs["libelf"] = "enabled" if self.options.get_safe("with_elf") else "disabled"

        meson.configure(
            source_folder=self._source_subfolder,
            args=["--wrap-mode=nofallback"],
            build_folder=self._build_subfolder,
            defs=defs,
        )
        return meson

    def _patch_sources(self):
        for patch in self.conan_data.get("patches", {}).get(self.version, []):
            tools.patch(**patch)
        if tools.Version(self.version) < "2.67.2":
            tools.replace_in_file(
                os.path.join(self._source_subfolder, "meson.build"),
                "build_tests = not meson.is_cross_build() or (meson.is_cross_build() and meson.has_exe_wrapper())",
                "build_tests = false",
            )
        tools.replace_in_file(
            os.path.join(self._source_subfolder, "meson.build"),
            "subdir('fuzzing')",
            "#subdir('fuzzing')",
        )  # https://gitlab.gnome.org/GNOME/glib/-/issues/2152
        if tools.Version(self.version) < "2.73.2":
            for filename in [
                os.path.join(self._source_subfolder, "meson.build"),
                os.path.join(self._source_subfolder, "glib", "meson.build"),
                os.path.join(self._source_subfolder, "gobject", "meson.build"),
                os.path.join(self._source_subfolder, "gio", "meson.build"),
            ]:
                tools.replace_in_file(filename, "subdir('tests')", "#subdir('tests')")
        if self.settings.os != "Linux":
            # allow to find gettext
            tools.replace_in_file(
                os.path.join(self._source_subfolder, "meson.build"),
                "libintl = cc.find_library('intl', required : false)" if tools.Version(self.version) < "2.73.1" \
                else "libintl = dependency('intl', required: false)",
                "libintl = dependency('libgettext', method : 'pkg-config', required : false)",
            )

        tools.replace_in_file(
            os.path.join(
                self._source_subfolder,
                "gio",
                "gdbus-2.0",
                "codegen",
                "gdbus-codegen.in",
            ),
            "'share'",
            "'res'",
        )

    def build(self):
        self._patch_sources()
        with tools.environment_append(
            VisualStudioBuildEnvironment(self).vars
        ) if is_msvc(self) else tools.no_op():
            meson = self._configure_meson()
            meson.build()

    def _fix_library_names(self):
        if self.settings.compiler == "Visual Studio":
            with tools.chdir(os.path.join(self.package_folder, "lib")):
                for filename_old in glob.glob("*.a"):
                    filename_new = filename_old[3:-2] + ".lib"
                    self.output.info(f"rename {filename_old} into {filename_new}")
                    shutil.move(filename_old, filename_new)

    def package(self):
        if tools.Version(self.version) < "2.73.0":
            self.copy(pattern="COPYING", dst="licenses", src=self._source_subfolder)
        else:
            self.copy(pattern="LGPL-2.1-or-later.txt", dst="licenses", src=os.path.join(self._source_subfolder, "LICENSES"))
        with tools.environment_append(
            VisualStudioBuildEnvironment(self).vars
        ) if is_msvc(self) else tools.no_op():
            meson = self._configure_meson()
            meson.install()
            self._fix_library_names()
        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))
        shutil.move(
            os.path.join(self.package_folder, "share"),
            os.path.join(self.package_folder, "res"),
        )
        for pdb_file in glob.glob(os.path.join(self.package_folder, "bin", "*.pdb")):
            os.unlink(pdb_file)

    def package_info(self):
        self.cpp_info.resdirs = ["res"]

        self.cpp_info.components["glib-2.0"].libs = ["glib-2.0"]
        self.cpp_info.components["glib-2.0"].names["pkg_config"] = "glib-2.0"
        self.cpp_info.components["glib-2.0"].set_property("pkg_config_name", "glib-2.0")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["glib-2.0"].system_libs.append("pthread")
        if self.settings.os == "Windows":
            self.cpp_info.components["glib-2.0"].system_libs.extend(
                ["ws2_32", "ole32", "shell32", "user32", "advapi32"]
            )
        if self.settings.os == "Macos":
            self.cpp_info.components["glib-2.0"].system_libs.append("resolv")
            self.cpp_info.components["glib-2.0"].frameworks.extend(
                ["Foundation", "CoreServices", "CoreFoundation"]
            )
        self.cpp_info.components["glib-2.0"].includedirs.append(
            os.path.join("include", "glib-2.0")
        )
        self.cpp_info.components["glib-2.0"].includedirs.append(
            os.path.join("lib", "glib-2.0", "include")
        )
        if self.options.with_pcre:
            if tools.Version(self.version) >= "2.73.2":
                self.cpp_info.components["glib-2.0"].requires.append("pcre2::pcre2")
            else:
                self.cpp_info.components["glib-2.0"].requires.append("pcre::pcre")
        if self.settings.os != "Linux":
            self.cpp_info.components["glib-2.0"].requires.append(
                "libgettext::libgettext"
            )
        if tools.is_apple_os(self.settings.os):
            self.cpp_info.components["glib-2.0"].requires.append("libiconv::libiconv")

        self.cpp_info.components["gmodule-no-export-2.0"].libs = ["gmodule-2.0"]
        self.cpp_info.components["gmodule-no-export-2.0"].set_property("pkg_config_name", "gmodule-no-export-2.0")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["gmodule-no-export-2.0"].system_libs.append(
                "pthread"
            )
            self.cpp_info.components["gmodule-no-export-2.0"].system_libs.append("dl")
        self.cpp_info.components["gmodule-no-export-2.0"].requires.append("glib-2.0")

        self.cpp_info.components["gmodule-export-2.0"].requires.extend(
            ["gmodule-no-export-2.0", "glib-2.0"]
        )
        self.cpp_info.components["gmodule-export-2.0"].set_property("pkg_config_name", "gmodule-export-2.0")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["gmodule-export-2.0"].sharedlinkflags.append(
                "-Wl,--export-dynamic"
            )

        self.cpp_info.components["gmodule-2.0"].set_property("pkg_config_name", "gmodule-2.0")
        self.cpp_info.components["gmodule-2.0"].requires.extend(
            ["gmodule-no-export-2.0", "glib-2.0"]
        )
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["gmodule-2.0"].sharedlinkflags.append(
                "-Wl,--export-dynamic"
            )

        self.cpp_info.components["gobject-2.0"].set_property("pkg_config_name", "gobject-2.0")
        self.cpp_info.components["gobject-2.0"].libs = ["gobject-2.0"]
        self.cpp_info.components["gobject-2.0"].requires.append("glib-2.0")
        self.cpp_info.components["gobject-2.0"].requires.append("libffi::libffi")

        self.cpp_info.components["gthread-2.0"].set_property("pkg_config_name", "gthread-2.0")
        self.cpp_info.components["gthread-2.0"].libs = ["gthread-2.0"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["gthread-2.0"].system_libs.append("pthread")
        self.cpp_info.components["gthread-2.0"].requires.append("glib-2.0")

        self.cpp_info.components["gio-2.0"].set_property("pkg_config_name", "gio-2.0")
        self.cpp_info.components["gio-2.0"].libs = ["gio-2.0"]
        if self.settings.os == "Linux":
            self.cpp_info.components["gio-2.0"].system_libs.append("resolv")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["gio-2.0"].system_libs.append("dl")
        if self.settings.os == "Windows":
            self.cpp_info.components["gio-2.0"].system_libs.extend(["iphlpapi", "dnsapi", "shlwapi"])
        self.cpp_info.components["gio-2.0"].requires.extend(
            ["glib-2.0", "gobject-2.0", "gmodule-2.0", "zlib::zlib"]
        )
        if self.settings.os == "Macos":
            self.cpp_info.components["gio-2.0"].frameworks.append("AppKit")
        if self.options.get_safe("with_mount"):
            self.cpp_info.components["gio-2.0"].requires.append("libmount::libmount")
        if self.options.get_safe("with_selinux"):
            self.cpp_info.components["gio-2.0"].requires.append(
                "libselinux::libselinux"
            )
        if self.settings.os == "Windows":
            self.cpp_info.components["gio-windows-2.0"].requires = [
                "gobject-2.0",
                "gmodule-no-export-2.0",
                "gio-2.0",
            ]
            self.cpp_info.components["gio-windows-2.0"].includedirs = [
                os.path.join("include", "gio-win32-2.0")
            ]
        else:
            self.cpp_info.components["gio-unix-2.0"].requires.extend(
                ["gobject-2.0", "gio-2.0"]
            )
            self.cpp_info.components["gio-unix-2.0"].includedirs = [
                os.path.join("include", "gio-unix-2.0")
            ]
        self.env_info.GLIB_COMPILE_SCHEMAS = os.path.join(
            self.package_folder, "bin", "glib-compile-schemas"
        )

        self.cpp_info.components["gresource"].set_property("pkg_config_name", "gresource")
        self.cpp_info.components["gresource"].libs = []  # this is actually an executable
        if self.options.get_safe("with_elf"):
            self.cpp_info.components["gresource"].requires.append(
                "libelf::libelf"
            )  # this is actually an executable

        bin_path = os.path.join(self.package_folder, "bin")
        self.output.info(f"Appending PATH env var with: {bin_path}")
        self.env_info.PATH.append(bin_path)

        pkgconfig_variables = {
            'libdir': '${prefix}/lib',
            'datadir': '${prefix}/res',
            'schemasdir': '${datadir}/glib-2.0/schemas',
            'bindir': '${prefix}/bin',
            'giomoduledir': '${libdir}/gio/modules',
            'gio': '${bindir}/gio',
            'gio_querymodules': '${bindir}/gio-querymodules',
            'glib_compile_schemas': '${bindir}/glib-compile-schemas',
            'glib_compile_resources': '${bindir}/glib-compile-resources',
            'gdbus': '${bindir}/gdbus',
            'gdbus_codegen': '${bindir}/gdbus-codegen',
            'gresource': '${bindir}/gresource',
            'gsettings': '${bindir}/gsettings'
        }
        self.cpp_info.components["gio-2.0"].set_property(
            "pkg_config_custom_content",
            "\n".join(f"{key}={value}" for key,value in pkgconfig_variables.items()))

        pkgconfig_variables = {
            'bindir': '${prefix}/bin',
            'glib_genmarshal': '${bindir}/glib-genmarshal',
            'gobject_query': '${bindir}/gobject-query',
            'glib_mkenums': '${bindir}/glib-mkenums'
        }
        self.cpp_info.components["glib-2.0"].set_property(
            "pkg_config_custom_content",
            "\n".join(f"{key}={value}" for key,value in pkgconfig_variables.items()))
