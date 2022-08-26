from conans import tools
from conan import ConanFile
from conan.tools import files
from conan.tools.cmake import CMake, CMakeToolchain
import os, functools

required_conan_version = ">=1.50.0"


class OpenEXRConan(ConanFile):
    name = "openexr"
    description = "OpenEXR is a high dynamic-range (HDR) image file format developed by Industrial Light & " \
                  "Magic for use in computer imaging applications."
    topics = ("openexr", "hdr", "image", "picture")
    license = "BSD-3-Clause"
    homepage = "https://github.com/AcademySoftwareFoundation/openexr"
    url = "https://github.com/conan-io/conan-center-index"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }
    exports_sources = "patches/*"
    generators = "cmake_find_package_multi"

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def layout(self):
        self.folders.source = self._source_subfolder
        self.folders.build = self._build_subfolder

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def requirements(self):
        self.requires("zlib/1.2.12")

        # Note: OpenEXR and Imath are versioned independently.
        self.requires("imath/3.1.5")

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 11)

    def source(self):
        files.get(self,
                  **self.conan_data["sources"][self.version],
                  strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["OPENEXR_INSTALL_EXAMPLES"] = False
        tc.variables["DOCS"] = False
        tc.generate()

    @functools.lru_cache(1)
    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake

    def build(self):
        files.apply_conandata_patches(self)

        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("LICENSE.md", src=self._source_subfolder, dst="licenses")
        cmake = self._configure_cmake()
        cmake.install()
        tools.rmdir(os.path.join(self.package_folder, "share"))
        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))
        tools.rmdir(os.path.join(self.package_folder, "lib", "cmake"))

    @staticmethod
    def _conan_comp(name):
        return f"openexr_{name.lower()}"

    def _add_component(self, name):
        component = self.cpp_info.components[self._conan_comp(name)]
        component.set_property("cmake_target_name", f"OpenEXR::{name}")
        component.names["cmake_find_package"] = name
        component.names["cmake_find_package_multi"] = name
        return component

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "OpenEXR")
        self.cpp_info.set_property("pkg_config_name", "OpenEXR")

        self.cpp_info.names["cmake_find_package"] = "OpenEXR"
        self.cpp_info.names["cmake_find_package_multi"] = "OpenEXR"
        self.cpp_info.names["pkg_config"] = "OpenEXR"

        openexr_version = tools.Version(self.version)
        lib_suffix = "-{}_{}".format(openexr_version.major, openexr_version.minor)
        if self.settings.build_type == "Debug":
            lib_suffix += "_d"

        # OpenEXR::OpenEXRConfig
        OpenEXRConfig = self._add_component("OpenEXRConfig")
        OpenEXRConfig.includedirs.append(os.path.join("include", "OpenEXR"))

        # OpenEXR::IexConfig
        IexConfig = self._add_component("IexConfig")
        IexConfig.includedirs = OpenEXRConfig.includedirs

        # OpenEXR::IlmThreadConfig
        IlmThreadConfig = self._add_component("IlmThreadConfig")
        IlmThreadConfig.includedirs = OpenEXRConfig.includedirs

        # OpenEXR::Iex
        Iex = self._add_component("Iex")
        Iex.libs = ["Iex{}".format(lib_suffix)]
        Iex.requires = [self._conan_comp("IexConfig")]

        # OpenEXR::IlmThread
        IlmThread = self._add_component("IlmThread")
        IlmThread.libs = ["IlmThread{}".format(lib_suffix)]
        IlmThread.requires = [
            self._conan_comp("IlmThreadConfig"), self._conan_comp("Iex"),
        ]
        if self.settings.os in ["Linux", "FreeBSD"]:
            IlmThread.system_libs = ["pthread"]

        # OpenEXR::OpenEXRCore
        OpenEXRCore = self._add_component("OpenEXRCore")
        OpenEXRCore.libs = ["OpenEXRCore{}".format(lib_suffix)]
        OpenEXRCore.requires = [self._conan_comp("OpenEXRConfig"), "zlib::zlib"]

        # OpenEXR::OpenEXR
        OpenEXR = self._add_component("OpenEXR")
        OpenEXR.libs = ["OpenEXR{}".format(lib_suffix)]
        OpenEXR.requires = [
            self._conan_comp("OpenEXRCore"), self._conan_comp("IlmThread"),
            self._conan_comp("Iex"), "imath::imath",
        ]

        # OpenEXR::OpenEXRUtil
        OpenEXRUtil = self._add_component("OpenEXRUtil")
        OpenEXRUtil.libs = ["OpenEXRUtil{}".format(lib_suffix)]
        OpenEXRUtil.requires = [self._conan_comp("OpenEXR")]

        # Add tools directory to PATH
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
