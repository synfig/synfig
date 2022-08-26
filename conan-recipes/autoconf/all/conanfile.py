from conans import AutoToolsBuildEnvironment, ConanFile, tools
import contextlib
import os

required_conan_version = ">=1.33.0"


class AutoconfConan(ConanFile):
    name = "autoconf"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://www.gnu.org/software/autoconf/"
    description = "Autoconf is an extensible package of M4 macros that produce shell scripts to automatically configure software source code packages"
    topics = ("autoconf", "configure", "build")
    license = ("GPL-2.0-or-later", "GPL-3.0-or-later")
    settings = "os", "arch", "compiler", "build_type"

    exports_sources = "patches/*"

    _autotools = None

    @property
    def _source_subfolder(self):
        return os.path.join(self.source_folder, "source_subfolder")

    @property
    def _settings_build(self):
        return getattr(self, "settings_build", self.settings)

    def requirements(self):
        self.requires("m4/1.4.19")

    def build_requirements(self):
        if hasattr(self, "settings_build"):
            self.build_requires("m4/1.4.19")
        if self._settings_build.os == "Windows" and not tools.get_env("CONAN_BASH_PATH"):
            self.build_requires("msys2/cci.latest")

    def package_id(self):
        self.info.header_only()

    def source(self):
        tools.get(**self.conan_data["sources"][self.version],
                  destination=self._source_subfolder, strip_root=True)

    @property
    def _datarootdir(self):
        return os.path.join(self.package_folder, "bin", "share")

    @property
    def _autoconf_datarootdir(self):
        return os.path.join(self._datarootdir, "autoconf")

    def _configure_autotools(self):
        if self._autotools:
            return self._autotools
        self._autotools = AutoToolsBuildEnvironment(self, win_bash=tools.os_info.is_windows)
        datarootdir = self._datarootdir
        prefix = self.package_folder
        if self.settings.os == "Windows":
            datarootdir = tools.unix_path(datarootdir, path_flavor=tools.MSYS2)
            prefix = tools.unix_path(prefix, path_flavor=tools.MSYS2)
        conf_args = [
            "--datarootdir={}".format(datarootdir),
            "--prefix={}".format(prefix),
        ]
        self._autotools.configure(args=conf_args, configure_dir=self._source_subfolder)
        return self._autotools

    def _patch_files(self):
        for patch in self.conan_data.get("patches", {}).get(self.version, []):
            tools.patch(**patch)

    @contextlib.contextmanager
    def _build_context(self):
        with tools.environment_append(tools.RunEnvironment(self).vars):
            yield

    def build(self):
        self._patch_files()
        with self._build_context():
            autotools = self._configure_autotools()
            autotools.make()

    def package(self):
        self.copy("COPYING*", src=self._source_subfolder, dst="licenses")
        with self._build_context():
            autotools = self._configure_autotools()
            autotools.install()
        tools.rmdir(os.path.join(self.package_folder, "bin", "share", "info"))
        tools.rmdir(os.path.join(self.package_folder, "bin", "share", "man"))

    def package_info(self):
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = []

        bin_path = os.path.join(self.package_folder, "bin")
        self.output.info("Appending PATH env var with : {}".format(bin_path))
        self.env_info.PATH.append(bin_path)

        ac_macrodir = tools.unix_path(
            self._autoconf_datarootdir, path_flavor=tools.MSYS2)
        self.output.info("Setting AC_MACRODIR to {}".format(ac_macrodir))
        self.env_info.AC_MACRODIR = ac_macrodir

        autoconf = tools.unix_path(os.path.join(
            self.package_folder, "bin", "autoconf"), path_flavor=tools.MSYS2)
        self.output.info("Setting AUTOCONF to {}".format(autoconf))
        self.env_info.AUTOCONF = autoconf

        autoreconf = tools.unix_path(os.path.join(
            self.package_folder, "bin", "autoreconf"), path_flavor=tools.MSYS2)
        self.output.info("Setting AUTORECONF to {}".format(autoreconf))
        self.env_info.AUTORECONF = autoreconf

        autoheader = tools.unix_path(os.path.join(
            self.package_folder, "bin", "autoheader"), path_flavor=tools.MSYS2)
        self.output.info("Setting AUTOHEADER to {}".format(autoheader))
        self.env_info.AUTOHEADER = autoheader

        autom4te = tools.unix_path(os.path.join(
            self.package_folder, "bin", "autom4te"), path_flavor=tools.MSYS2)
        self.output.info("Setting AUTOM4TE to {}".format(autom4te))
        self.env_info.AUTOM4TE = autom4te

        autom4te_perllibdir = self._autoconf_datarootdir
        self.output.info("Setting AUTOM4TE_PERLLIBDIR to {}".format(autom4te_perllibdir))
        self.env_info.AUTOM4TE_PERLLIBDIR = autom4te_perllibdir
