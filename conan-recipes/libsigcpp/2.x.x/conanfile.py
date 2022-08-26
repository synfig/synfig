from conans import ConanFile, Meson, tools
from conan.tools.files import rename
from conans.errors import ConanInvalidConfiguration
import glob
import os
import shutil

required_conan_version = ">=1.43.0"


class LibSigCppConanV2(ConanFile):
    name = "libsigcpp"
    homepage = "https://github.com/libsigcplusplus/libsigcplusplus"
    url = "https://github.com/conan-io/conan-center-index"
    license = "LGPL-3.0"
    description = "libsigc++ implements a typesafe callback system for standard C++."
    topics = ("libsigcpp", "callback")

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
    short_paths = True

    def validate(self):
        if hasattr(self, "settings_build") and tools.cross_building(self):
            raise ConanInvalidConfiguration("Cross-building not implemented")
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 11)

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def build_requirements(self):
        self.build_requires("meson/0.59.1")
        self.build_requires("pkgconf/1.7.4")

    def source(self):
        tools.get(
            **self.conan_data["sources"][self.version],
            strip_root=True,
            destination=self._source_subfolder
        )

    def build(self):
        if not self.options.shared:
            tools.replace_in_file(
                os.path.join(self._source_subfolder, "sigc++config.h.meson"),
                "define SIGC_DLL 1", "undef SIGC_DLL")
        with tools.environment_append(tools.RunEnvironment(self).vars):
            meson = self._configure_meson()
            meson.build()

    def _configure_meson(self):
        meson = Meson(self)
        defs = {}
        defs["build-examples"] = "false"
        defs["build-documentation"] = "false"
        defs["default_library"] = "shared" if self.options.shared else "static"
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
        if self.settings.compiler == "Visual Studio":
            tools.remove_files_by_mask(os.path.join(self.package_folder, "bin"), "*.pdb")
            if not self.options.shared:
                rename(self,
                       os.path.join(self.package_folder, "lib", "libsigc-2.0.a"),
                       os.path.join(self.package_folder, "lib", "sigc-2.0.lib"))

        for header_file in glob.glob(os.path.join(self.package_folder, "lib", "sigc++-2.0", "include", "*.h")):
            shutil.move(
                header_file,
                os.path.join(self.package_folder, "include",
                             "sigc++-2.0", os.path.basename(header_file))
            )

        for dir_to_remove in ["pkgconfig", "sigc++-2.0"]:
            tools.rmdir(os.path.join(
                self.package_folder, "lib", dir_to_remove))

    def package_info(self):
        self.cpp_info.components["sigc++-2.0"].names["pkg_config"] = "sigc++-2.0"
        self.cpp_info.components["sigc++-2.0"].set_property(
            "pkg_config_name", "sigc++-2.0")
        self.cpp_info.components["sigc++-2.0"].includedirs.append(os.path.join("include", "sigc++-2.0"))
        self.cpp_info.components["sigc++-2.0"].libs = ["sigc-2.0"]
