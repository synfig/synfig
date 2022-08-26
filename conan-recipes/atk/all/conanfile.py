from conans import tools
from conan import ConanFile
from conan.tools.meson import Meson, MesonToolchain
from conan.tools import files
from conan.errors import ConanInvalidConfiguration
from conan.tools.gnu import PkgConfigDeps
import functools
import os
import glob

required_conan_version = ">=1.50"

class AtkConan(ConanFile):
    name = "atk"
    description = "set of accessibility interfaces that are implemented by other toolkits and applications"
    topics = ("conan", "atk", "accessibility")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://www.atk.org"
    license = "LGPL-2.1-or-later"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    exports_sources = "patches/**"

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

    def build_requirements(self):
        self.build_requires("meson/0.60.2")
        self.build_requires("pkgconf/1.7.4")

    def requirements(self):
        self.requires("glib/2.73.2")
        if self.settings.os != "Linux":
            self.requires("libgettext/0.21")

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd
        if self.options.shared:
            self.options["glib"].shared = True

    def validate(self):
        if self.options.shared and not self.options["glib"].shared:
            raise ConanInvalidConfiguration(
                "Linking a shared library against static glib can cause unexpected behaviour."
            )

    def source(self):
        files.get(self,
                  **self.conan_data["sources"][self.version],
                  strip_root=True)

    def generate(self):
        tc = MesonToolchain(self)
        tc.project_options["introspection"] = False
        tc.project_options["docs"] = False
        tc.project_options["localedir"] = os.path.join(self.package_folder, "bin", "share", "locale")
        tc.generate()

        td = PkgConfigDeps(self)
        td.generate()

    @functools.lru_cache(1)
    def _configure_meson(self):
        meson = Meson(self)
        meson.configure()
        return meson

    def build(self):
        files.apply_conandata_patches(self)
        with tools.environment_append({"PATH": self.deps_cpp_info["glib"].bin_paths}):
            meson = self._configure_meson()
            meson.build()

    def package(self):
        self.copy(pattern="COPYING", dst="licenses", src=self._source_subfolder)
        with tools.environment_append({"PATH": self.deps_cpp_info["glib"].bin_paths}):
            meson = self._configure_meson()
            meson.install()
        files.rmdir(self, os.path.join(
            self.package_folder, "lib", "pkgconfig"))
        files.rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
        if self.settings.compiler == "Visual Studio":
            for pdb in glob.glob(os.path.join(self.package_folder, "bin", "*.pdb")):
                os.unlink(pdb)
            if not self.options.shared:
                os.rename(os.path.join(self.package_folder, "lib", "libatk-1.0.a"), os.path.join(self.package_folder, "lib", "atk-1.0.lib"))

    def package_info(self):
        self.cpp_info.libs = ["atk-1.0"]
        self.cpp_info.includedirs = ["include/atk-1.0"]

    def package_id(self):
        if not self.options["glib"].shared:
            self.info.requires["glib"].full_package_mode()
