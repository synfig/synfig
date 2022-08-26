from conan import ConanFile
from conan.tools import files, microsoft, scm
from conan .errors import ConanInvalidConfiguration
from conans import CMake, tools

import functools
import os

required_conan_version = ">=1.33.0"


class HarfbuzzConan(ConanFile):
    name = "harfbuzz"
    description = "HarfBuzz is an OpenType text shaping engine."
    topics = ("opentype", "text", "engine")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "http://harfbuzz.org"
    license = "MIT"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_freetype": [True, False],
        "with_icu": [True, False],
        "with_glib": [True, False],
        "with_gdi": [True, False],
        "with_uniscribe": [True, False],
        "with_directwrite": [True, False],
        "with_subset": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_freetype": True,
        "with_icu": False,
        "with_glib": True,
        "with_gdi": True,
        "with_uniscribe": True,
        "with_directwrite": False,
        "with_subset": False,
    }

    short_paths = True

    generators = "cmake", "cmake_find_package"

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
        else:
            del self.options.with_gdi
            del self.options.with_uniscribe
            del self.options.with_directwrite

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        if self.options.shared and self.options.with_glib:
            self.options["glib"].shared = True

    def validate(self):
        if self.options.shared and self.options.with_glib and not self.options["glib"].shared:
            raise ConanInvalidConfiguration(
                "Linking a shared library against static glib can cause unexpected behaviour."
            )
        if scm.Version(self.version) >= "4.4.0":
            if self.settings.compiler == "gcc" and scm.Version(self.settings.compiler.version) < "7":
                raise ConanInvalidConfiguration("New versions of harfbuzz require at least gcc 7")

        if self.options["glib"].shared and microsoft.is_msvc_static_runtime(self):
            raise ConanInvalidConfiguration(
                "Linking shared glib with the MSVC static runtime is not supported"
            )

    def requirements(self):
        if self.options.with_freetype:
            self.requires("freetype/2.12.1")
        if self.options.with_icu:
            self.requires("icu/71.1")
        if self.options.with_glib:
            self.requires("glib/2.73.1")

    def source(self):
        files.get(self, **self.conan_data["sources"][self.version],
                  destination=self._source_subfolder, strip_root=True)

    @functools.lru_cache(1)
    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["HB_HAVE_FREETYPE"] = self.options.with_freetype
        cmake.definitions["HB_HAVE_GRAPHITE2"] = False
        cmake.definitions["HB_HAVE_GLIB"] = self.options.with_glib
        cmake.definitions["HB_HAVE_ICU"] = self.options.with_icu
        if tools.is_apple_os(self.settings.os):
            cmake.definitions["HB_HAVE_CORETEXT"] = True
        elif self.settings.os == "Windows":
            cmake.definitions["HB_HAVE_GDI"] = self.options.with_gdi
            cmake.definitions["HB_HAVE_UNISCRIBE"] = self.options.with_uniscribe
            cmake.definitions["HB_HAVE_DIRECTWRITE"] = self.options.with_directwrite
        cmake.definitions["HB_BUILD_UTILS"] = False
        cmake.definitions["HB_BUILD_SUBSET"] = self.options.with_subset
        cmake.definitions["HB_HAVE_GOBJECT"] = False
        cmake.definitions["HB_HAVE_INTROSPECTION"] = False
        # fix for MinGW debug build
        if self.settings.compiler == "gcc" and self.settings.os == "Windows":
            cmake.definitions["CMAKE_C_FLAGS"] = "-Wa,-mbig-obj"
            cmake.definitions["CMAKE_CXX_FLAGS"] = "-Wa,-mbig-obj"

        cmake.configure(build_folder=self._build_subfolder)
        return cmake

    def build(self):
        files.apply_conandata_patches(self)
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("COPYING", dst="licenses", src=self._source_subfolder)
        cmake = self._configure_cmake()
        cmake.install()
        files.rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "harfbuzz"
        self.cpp_info.names["cmake_find_package_multi"] = "harfbuzz"
        if self.options.with_icu:
            self.cpp_info.libs.append("harfbuzz-icu")
        if self.options.with_subset:
            self.cpp_info.libs.append("harfbuzz-subset")
        self.cpp_info.libs.append("harfbuzz")
        self.cpp_info.includedirs.append(os.path.join("include", "harfbuzz"))
        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("m")
        if self.settings.os == "Windows" and not self.options.shared:
            self.cpp_info.system_libs.append("user32")
            if self.options.with_gdi or self.options.with_uniscribe:
                self.cpp_info.system_libs.append("gdi32")
            if self.options.with_uniscribe or self.options.with_directwrite:
                self.cpp_info.system_libs.append("rpcrt4")
            if self.options.with_uniscribe:
                self.cpp_info.system_libs.append("usp10")
            if self.options.with_directwrite:
                self.cpp_info.system_libs.append("dwrite")
        if tools.is_apple_os(self.settings.os):
            self.cpp_info.frameworks.extend(["CoreFoundation", "CoreGraphics", "CoreText"])
        if not self.options.shared:
            libcxx = tools.stdcpp_library(self)
            if libcxx:
                self.cpp_info.system_libs.append(libcxx)

    def package_id(self):
        if self.options.with_glib and not self.options["glib"].shared:
            self.info.requires["glib"].full_package_mode()
