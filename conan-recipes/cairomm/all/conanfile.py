from conans import ConanFile, Meson, tools
from conan.tools.files import rename
from conan.tools.microsoft import is_msvc
from conans.errors import ConanInvalidConfiguration
import glob
import os
import shutil


class CairommConan(ConanFile):
    name = "cairomm"
    homepage = "https://github.com/freedesktop/cairomm"
    url = "https://github.com/conan-io/conan-center-index"
    license = "LGPL-2.0"
    description = "cairomm is a C++ wrapper for the cairo graphics library."
    topics = ["cairo", "wrapper", "graphics"]

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    generators = "pkg_config"
    exports_sources = "patches/**"
    short_paths = True

    def _abi_version(self):
        return "1.16" if tools.Version(self.version) >= "1.16.0" else "1.0"

    def validate(self):
        if hasattr(self, "settings_build") and tools.cross_building(self):
            raise ConanInvalidConfiguration("Cross-building not implemented")
        if self.settings.compiler.get_safe("cppstd"):
            if self._abi_version() == "1.16":
                tools.check_min_cppstd(self, 17)
            else:
                tools.check_min_cppstd(self, 11)
        if self.options.shared and not self.options["cairo"].shared:
            raise ConanInvalidConfiguration(
                "Linking against static cairo would cause shared cairomm to link "
                "against static glib which can cause problems."
            )

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def _patch_sources(self):
        for patch in self.conan_data["patches"][self.version]:
            tools.patch(**patch)
        if is_msvc(self):
            # when using cpp_std=c++11 the /permissive- flag is added which
            # attempts enforcing standard conformant c++ code
            # the problem is that older versions of Windows SDK is not standard
            # conformant! see:
            # https://developercommunity.visualstudio.com/t/error-c2760-in-combaseapih-with-windows-sdk-81-and/185399
            tools.replace_in_file(
                os.path.join(self._source_subfolder, "meson.build"),
                "cpp_std=c++", "cpp_std=vc++")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        if self.options.shared:
            self.options["cairo"].shared = True

    def build_requirements(self):
        self.build_requires("meson/0.59.1")
        self.build_requires("pkgconf/1.7.4")

    def requirements(self):
        self.requires("cairo/1.17.4")

        if self._abi_version() == "1.16":
            self.requires("libsigcpp/3.0.7")
        else:
            self.requires("libsigcpp/2.10.8")

    def source(self):
        tools.get(
            **self.conan_data["sources"][self.version],
            strip_root=True,
            destination=self._source_subfolder,
        )

    def build(self):
        self._patch_sources()
        with tools.environment_append(tools.RunEnvironment(self).vars):
            meson = self._configure_meson()
            meson.build()

    def _configure_meson(self):
        meson = Meson(self)
        defs = {
            "build-examples": "false",
            "build-documentation": "false",
            "build-tests": "false",
            "msvc14x-parallel-installable": "false",
            "default_library": "shared" if self.options.shared else "static",
        }
        meson.configure(
            defs=defs,
            build_folder=self._build_subfolder,
            source_folder=self._source_subfolder,
            pkg_config_paths=[self.install_folder],
        )
        return meson

    def package(self):
        self.copy("COPYING", dst="licenses", src=self._source_subfolder)
        meson = self._configure_meson()
        meson.install()
        if is_msvc(self):
            tools.remove_files_by_mask(
                os.path.join(self.package_folder, "bin"), "*.pdb")
            if not self.options.shared:
                rename(
                    self,
                    os.path.join(
                        self.package_folder,
                        "lib",
                        f"libcairomm-{self._abi_version()}.a",
                    ),
                    os.path.join(self.package_folder, "lib",
                                 f"cairomm-{self._abi_version()}.lib"),
                )

        for header_file in glob.glob(
                os.path.join(
                    self.package_folder,
                    "lib",
                    f"cairomm-{self._abi_version()}",
                    "include",
                    "*.h",
                )):
            shutil.move(
                header_file,
                os.path.join(
                    self.package_folder,
                    "include",
                    f"cairomm-{self._abi_version()}",
                    os.path.basename(header_file),
                ),
            )

        for dir_to_remove in ["pkgconfig", f"cairomm-{self._abi_version()}"]:
            tools.rmdir(os.path.join(self.package_folder, "lib",
                                     dir_to_remove))

    def package_info(self):
        if self._abi_version() == "1.16":
            self.cpp_info.components["cairomm-1.16"].names[
                "pkg_config"] = "cairomm-1.16"
            self.cpp_info.components["cairomm-1.16"].set_property(
                "pkg_config_name", "cairomm-1.16")
            self.cpp_info.components["cairomm-1.16"].includedirs = [
                os.path.join("include", "cairomm-1.16")
            ]
            self.cpp_info.components["cairomm-1.16"].libs = ["cairomm-1.16"]
            self.cpp_info.components["cairomm-1.16"].requires = [
                "libsigcpp::sigc++", "cairo::cairo_"
            ]
            if tools.is_apple_os(self.settings.os):
                self.cpp_info.components["cairomm-1.16"].frameworks = [
                    "CoreFoundation"
                ]
        else:
            self.cpp_info.components["cairomm-1.0"].names[
                "pkg_config"] = "cairomm-1.0"
            self.cpp_info.components["cairomm-1.0"].set_property(
                "pkg_config_name", "cairomm-1.0")
            self.cpp_info.components["cairomm-1.0"].includedirs = [
                os.path.join("include", "cairomm-1.0")
            ]
            self.cpp_info.components["cairomm-1.0"].libs = ["cairomm-1.0"]
            self.cpp_info.components["cairomm-1.0"].requires = [
                "libsigcpp::sigc++-2.0", "cairo::cairo_"
            ]
            if tools.is_apple_os(self.settings.os):
                self.cpp_info.components["cairomm-1.0"].frameworks = [
                    "CoreFoundation"
                ]
