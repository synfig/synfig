import os
import shutil
import glob
import re
import subprocess

from conan import ConanFile
from conan.tools import microsoft
from conan.tools.gnu import PkgConfigDeps
from conan.tools.cmake import CMakeDeps, CMakeToolchain

required_conan_version = ">=1.50.0"


class SynfigConan(ConanFile):
    name = "synfig"
    homepage = "https://www.synfig.org/"
    license = "GPL-3"
    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.build_requires("pkgconf/1.7.4")
        self.build_requires("gettext/0.21")

    def requirements(self):
        self.requires("harfbuzz/5.1.0")
        self.requires("openexr/3.1.5")
        self.requires("boost/1.79.0")
        self.requires("fftw/3.3.9")
        self.requires("glib/2.73.2")
        self.requires("pango/1.50.8")
        self.requires("cairo/1.17.4")
        self.requires("gdk-pixbuf/2.42.8")
        self.requires("gtk/3.24.34")
        self.requires("libxmlpp/2.42.1")
        self.requires("glibmm/2.66.4")
        self.requires("cairomm/1.14.3")
        self.requires("pangomm/2.46.2")
        self.requires("libsigcpp/2.10.8")
        self.requires("gtkmm/3.24.6")
        self.requires("freetype/2.12.1")
        self.requires("libxml2/2.9.14")
        self.requires("libxmlpp/2.42.1")
        self.requires("libtool/2.4.6")
        self.requires("gettext/0.21")
        self.requires("atk/2.38.0")
        self.requires("imagemagick/7.1.0-45")
        self.requires("libgettext/0.21")

        # has to be here to get its binary path
        self.requires("pkgconf/1.7.4")

    def layout(self):
        self.folders.generators = os.path.join(os.getcwd(), "conan-prefix")

    @property
    def _pkgconf_dir(self):
        return os.path.join(self.generators_folder, "lib", "pkgconfig")

    @property
    def _cmake_dir(self):
        return os.path.join(self.generators_folder, "lib", "cmake")

    def _move_overwrite(self, src, dst):
        if os.path.isfile(os.path.join(dst, os.path.basename(src))):
            os.remove(os.path.join(dst, os.path.basename(src)))
        shutil.move(src, dst)

    def _copy_overwrite(self, src, dst):
        if os.path.isfile(os.path.join(dst, os.path.basename(src))):
            os.remove(os.path.join(dst, os.path.basename(src)))
        shutil.copy(src, dst)

    def _move_pkgconfig_files(self):
        os.makedirs(self._pkgconf_dir, exist_ok=True)

        for file in glob.glob(os.path.join(self.generators_folder, "*.pc")):
            self._move_overwrite(file, self._pkgconf_dir)

    def _move_cmake_files(self):
        modules_dir = os.path.join(
            self.generators_folder, "share", "cmake", "Modules")

        os.makedirs(modules_dir, exist_ok=True)

        config_files = glob.glob(os.path.join(
            self.generators_folder, "*-config.cmake"))
        config_files.extend(glob.glob(os.path.join(
            self.generators_folder, "*Config.cmake")))

        package_names = []
        for file in config_files:
            filename = os.path.basename(file)
            if filename.endswith("-config.cmake"):
                package_names.append(filename[:-len("-config.cmake")])
            elif filename.endswith("Config.cmake"):
                package_names.append(filename[:-len("Config.cmake")])

        package_names = list(set(package_names))
        package_names.sort(key=lambda x: -len(x))

        for p in package_names:
            os.makedirs(os.path.join(self._cmake_dir, p), exist_ok=True)
            self._copy_overwrite(
                os.path.join(self.generators_folder, "cmakedeps_macros.cmake"),
                os.path.join(self._cmake_dir, p))
        os.remove(os.path.join(self.generators_folder, "cmakedeps_macros.cmake"))

        for file in glob.glob(os.path.join(self.generators_folder, "*.cmake")):
            filename = os.path.basename(file)

            # find package module
            package_name = None
            if filename.startswith("Find"):
                self._move_overwrite(file, os.path.join(modules_dir))
            else:
                for p in package_names:
                    if p.lower() in filename.lower():
                        package_name = p
                        break

                if package_name:
                    self._move_overwrite(file, os.path.join(self._cmake_dir, package_name))
                else:
                    print(
                        f"ERROR: Could not determine the package corresponding to the cmake file {filename}")
                    self._move_overwrite(file, self._cmake_dir)

    # glib schemas in the glib pkgconfig fie refers to the schemas in the glib
    # package directory, but we're interested in the schemas defined by gtk
    def _fixup_glib_schemas(self):
        glib_schemas_path = os.path.join(
            self.generators_folder, "share", "glib-2.0").replace('\\', '/')

        if os.path.exists(glib_schemas_path):
            shutil.rmtree(glib_schemas_path)

        shutil.copytree(os.path.join(
            self.deps_cpp_info["glib"].rootpath,
            self.deps_cpp_info["glib"].resdirs[0], "glib-2.0"), glib_schemas_path,
            dirs_exist_ok=True)

        shutil.copytree(os.path.join(
            self.deps_cpp_info["gtk"].rootpath,
            self.deps_cpp_info["gtk"].resdirs[0], "share", "glib-2.0"), glib_schemas_path,
            dirs_exist_ok=True)

        with open(os.path.join(self._pkgconf_dir, "gio-2.0.pc"), "r") as f:
            content = f.read()
            content_new = re.sub(
                "^schemasdir.*$", f"schemasdir={glib_schemas_path}/schemas", content, flags=re.M)

        with open(os.path.join(self._pkgconf_dir, "gio-2.0.pc"), "w") as f:
            f.write(content_new)

    def generate(self):
        td = PkgConfigDeps(self)
        td.generate()
        self._move_pkgconfig_files()

        td = CMakeDeps(self)
        td.generate()
        self._move_cmake_files()
        self._fixup_glib_schemas()

        suffix = ".exe" if self.settings.os == 'Windows' else ""

        tc = CMakeToolchain(self)
        tc.variables["CMAKE_PREFIX_PATH"] = "${CMAKE_PREFIX_PATH};" + self.generators_folder.replace("\\", "/")
        tc.variables["CONAN_TOOLCHAIN"] = True
        tc.variables["CONAN_INSTALLS_PREFIX"] = self.generators_folder.replace("\\", "/")
        tc.variables["PKG_CONFIG_ARGN"] = "--dont-define-prefix"
        tc.variables["PKG_CONFIG_EXECUTABLE"] = os.path.join(
            self.deps_cpp_info["pkgconf"].bin_paths[0], f"pkgconf{suffix}").replace("\\", "/")
        tc.variables["GETTEXT_MSGMERGE_EXECUTABLE"] = os.path.join(
            self.deps_cpp_info["gettext"].rootpath, "bin", f"msgmerge{suffix}").replace("\\", "/")
        tc.variables["GETTEXT_MSGFMT_EXECUTABLE"] = os.path.join(
            self.deps_cpp_info["gettext"].rootpath, "bin", f"msgfmt{suffix}").replace("\\", "/")
        tc.preprocessor_definitions["CONAN_TOOLCHAIN"] = True
        tc.generate()

    def imports(self):
        if microsoft.is_msvc(self):
            bin_dir = os.path.join(self.generators_folder, "bin")
            self.copy("*.dll", bin_dir, "@bindirs")

        fontconf_file = self.deps_env_info["fontconfig"].FONTCONFIG_FILE
        fontconf_dir = os.path.join(self.generators_folder, "etc", "fonts")
        os.makedirs(fontconf_dir, exist_ok=True)
        self._copy_overwrite(fontconf_file, fontconf_dir)

        # Unfortunately fontconf recipe does not provide a variable locating
        # the conf.d directory, so we have to just hardcode it
        fontconf_conan_conf_dir = os.path.join(os.path.dirname(fontconf_file),
                                               os.pardir, os.pardir, "share",
                                               "fontconfig", "conf.avail")
        fontconf_conf_dir = os.path.join(fontconf_dir, "conf.d")
        os.makedirs(fontconf_conf_dir, exist_ok=True)

        # fontconfig has a list of available configuration files (some are
        # mutually exclusive). I chose the conf files that my system uses
        # (Arch Linux) and are available in the conan build
        conf_files = [
            "10-scale-bitmap-fonts.conf",
            "11-lcdfilter-default.conf",
            "20-unhint-small-vera.conf",
            "30-metric-aliases.conf",
            "40-nonlatin.conf",
            "45-generic.conf",
            "45-latin.conf",
            "49-sansserif.conf",
            "50-user.conf",
            "51-local.conf",
            "60-generic.conf",
            "60-latin.conf",
            "65-fonts-persian.conf",
            "65-nonlatin.conf",
            "69-unifont.conf",
            "80-delicious.conf",
            "90-synthetic.conf"
        ]

        for f in glob.glob(os.path.join(fontconf_conan_conf_dir, "*")):
            filename = os.path.basename(f)
            if filename in conf_files:
                self._copy_overwrite(f, fontconf_conf_dir)

    def config_options(self):
        self.options["glib"].shared = True
        self.options["harfbuzz"].shared = True
        self.options["pango"].shared = True
        self.options["cairo"].shared = True
        self.options["gdk-pixbuf"].shared = True
        self.options["gtk"].shared = True
        self.options["libsigcpp"].shared = True
        self.options["glibmm"].shared = True
        self.options["cairomm"].shared = True
        self.options["pangomm"].shared = True
        self.options["gtkmm"].shared = True
        self.options["libxmlpp"].shared = True
        self.options["atk"].shared = True
        self.options["imagemagick"].shared = True
