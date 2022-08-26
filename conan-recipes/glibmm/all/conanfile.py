from conans import ConanFile, Meson, tools
from conan.tools.files import rename
from conan.tools.microsoft import is_msvc
from conans.errors import ConanInvalidConfiguration
import os
import glob
import shutil


class GlibmmConan(ConanFile):
    name = "glibmm"
    homepage = "https://gitlab.gnome.org/GNOME/glibmm"
    license = "LGPL-2.1"
    url = "https://github.com/conan-io/conan-center-index"
    description = "glibmm is a C++ API for parts of glib that are useful for C++."
    topics = ["glibmm", "giomm"]
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    generators = "pkg_config"
    exports_sources = "patches/**"
    short_paths = True

    def _abi_version(self):
        return "2.68" if tools.Version(self.version) >= "2.68.0" else "2.4"

    def _glibmm_lib(self):
        return f"glibmm-{self._abi_version()}"

    def _giomm_lib(self):
        return f"giomm-{self._abi_version()}"

    def validate(self):
        if hasattr(self, "settings_build") and tools.cross_building(self):
            raise ConanInvalidConfiguration("Cross-building not implemented")

        if self.settings.compiler.get_safe("cppstd"):
            if self._abi_version() == "2.68":
                tools.check_min_cppstd(self, 17)
            else:
                tools.check_min_cppstd(self, 11)
        if self.options.shared and not self.options["glib"].shared:
            raise ConanInvalidConfiguration(
                "Linking a shared library against static glib can cause unexpected behaviour."
            )


    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build_requirements(self):
        self.build_requires("meson/0.59.1")
        self.build_requires("pkgconf/1.7.4")

    def requirements(self):
        self.requires("glib/2.73.0")

        if self._abi_version() == "2.68":
            self.requires("libsigcpp/3.0.7")
        else:
            self.requires("libsigcpp/2.10.8")

    def source(self):
        tools.get(
            **self.conan_data["sources"][self.version],
            strip_root=True,
            destination=self._source_subfolder,
        )

    def _patch_sources(self):
        for patch in self.conan_data["patches"][self.version]:
            tools.patch(**patch)

        if is_msvc(self):
            # GLiBMM_GEN_EXTRA_DEFS_STATIC is not defined anywhere and is not
            # used anywhere except here
            # when building a static build !defined(GLiBMM_GEN_EXTRA_DEFS_STATIC)
            # evaluates to 0
            if not self.options.shared:
                tools.replace_in_file(
                    os.path.join(self._source_subfolder, "tools",
                                 "extra_defs_gen", "generate_extra_defs.h"),
                    "#if defined (_MSC_VER) && !defined (GLIBMM_GEN_EXTRA_DEFS_STATIC)",
                    "#if 0",
                )

            # when using cpp_std=c++NM the /permissive- flag is added which
            # attempts enforcing standard conformant c++ code
            # the problem is that older versions of Windows SDK is not standard
            # conformant! see:
            # https://developercommunity.visualstudio.com/t/error-c2760-in-combaseapih-with-windows-sdk-81-and/185399
            tools.replace_in_file(
                os.path.join(self._source_subfolder, "meson.build"),
                "cpp_std=c++", "cpp_std=vc++")

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        if self.options.shared:
            self.options["glib"].shared = True

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
                    os.path.join(self.package_folder, "lib",
                                 f"libglibmm-{self._abi_version()}.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"{self._glibmm_lib()}.lib"),
                )
                rename(
                    self,
                    os.path.join(self.package_folder, "lib",
                                 f"libgiomm-{self._abi_version()}.a"),
                    os.path.join(self.package_folder, "lib",
                                 f"{self._giomm_lib()}.lib"),
                )
                rename(
                    self,
                    os.path.join(
                        self.package_folder,
                        "lib",
                        f"libglibmm_generate_extra_defs-{self._abi_version()}.a",
                    ),
                    os.path.join(
                        self.package_folder,
                        "lib",
                        f"glibmm_generate_extra_defs-{self._abi_version()}.lib",
                    ),
                )

        for directory in [self._glibmm_lib(), self._giomm_lib()]:
            directory_path = os.path.join(self.package_folder, "lib",
                                          directory, "include", "*.h")
            for header_file in glob.glob(directory_path):
                shutil.move(
                    header_file,
                    os.path.join(
                        self.package_folder,
                        "include",
                        directory,
                        os.path.basename(header_file),
                    ),
                )

        for dir_to_remove in [
                "pkgconfig",
                self._glibmm_lib(),
                self._giomm_lib()
        ]:
            tools.rmdir(os.path.join(self.package_folder, "lib",
                                     dir_to_remove))

    def package_info(self):
        if self._abi_version() == "2.68":
            self.cpp_info.components["glibmm-2.68"].names[
                "pkg_config"] = "glibmm-2.68"
            self.cpp_info.components["glibmm-2.68"].set_property(
                "pkg_config_name", "glibmm-2.68")
            self.cpp_info.components["glibmm-2.68"].libs = ["glibmm-2.68"]
            self.cpp_info.components["glibmm-2.68"].libs = ["glibmm-2.68"]
            self.cpp_info.components["glibmm-2.68"].includedirs = [
                os.path.join("include", "glibmm-2.68")
            ]
            self.cpp_info.components["glibmm-2.68"].requires = [
                "glib::gobject-2.0", "libsigcpp::sigc++"
            ]

            self.cpp_info.components["giomm-2.68"].names[
                "pkg_config"] = "giomm-2.68"
            self.cpp_info.components["giomm-2.68"].set_property(
                "pkg_config_name", "giomm-2.68")
            self.cpp_info.components["giomm-2.68"].libs = ["giomm-2.68"]
            self.cpp_info.components["giomm-2.68"].includedirs = [
                os.path.join("include", "giomm-2.68")
            ]
            self.cpp_info.components["giomm-2.68"].requires = [
                "glibmm-2.68", "glib::gio-2.0"
            ]

        else:
            self.cpp_info.components["glibmm-2.4"].names[
                "pkg_config"] = "glibmm-2.4"
            self.cpp_info.components["glibmm-2.4"].set_property(
                "pkg_config_name", "glibmm-2.4")
            self.cpp_info.components["glibmm-2.4"].libs = ["glibmm-2.4"]
            self.cpp_info.components["glibmm-2.4"].libs = ["glibmm-2.4"]
            self.cpp_info.components["glibmm-2.4"].includedirs = [
                os.path.join("include", "glibmm-2.4")
            ]
            self.cpp_info.components["glibmm-2.4"].requires = [
                "glib::gobject-2.0", "libsigcpp::sigc++-2.0"
            ]

            self.cpp_info.components["giomm-2.4"].names[
                "pkg_config"] = "giomm-2.4"
            self.cpp_info.components["giomm-2.4"].set_property(
                "pkg_config_name", "giomm-2.4")
            self.cpp_info.components["giomm-2.4"].libs = ["giomm-2.4"]
            self.cpp_info.components["giomm-2.4"].includedirs = [
                os.path.join("include", "giomm-2.4")
            ]
            self.cpp_info.components["giomm-2.4"].requires = [
                "glibmm-2.4", "glib::gio-2.0"
            ]

    def package_id(self):
        if not self.options["glib"].shared:
            self.info.requires["glib"].full_package_mode()
