from conan import ConanFile
from conans import tools
from conan.errors import ConanInvalidConfiguration
from conan.tools.gnu import Autotools, AutotoolsDeps, PkgConfigDeps, AutotoolsToolchain
from conan.tools import files, microsoft, scm
import os

required_conan_version = ">=1.50.0"

class ImageMagicConan(ConanFile):
    name = "imagemagick"
    description = (
        "ImageMagick is a free and open-source software suite for displaying, converting, and editing "
        "raster image and vector image files"
    )
    topics = ("imagemagick", "images", "manipulating")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://imagemagick.org"
    license = "ImageMagick"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "hdri": [True, False],
        "quantum_depth": [8, 16, 32],
        "with_zlib": [True, False],
        "with_bzlib": [True, False],
        "with_lzma": [True, False],
        "with_lcms": [True, False],
        "with_openexr": [True, False],
        "with_heic": [True, False],
        "with_jbig": [True, False],
        "with_jpeg": [None, "libjpeg", "libjpeg-turbo"],
        "with_openjp2": [True, False],
        "with_pango": [True, False],
        "with_png": [True, False],
        "with_tiff": [True, False],
        "with_webp": [True, False],
        "with_xml2": [True, False],
        "with_freetype": [True, False],
        "with_djvu": [True, False],
        "with_openmp": [True, False],
        "utilities": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "hdri": True,
        "quantum_depth": 16,
        "with_zlib": True,
        "with_bzlib": True,
        "with_lzma": True,
        "with_lcms": True,
        "with_openexr": True,
        "with_heic": True,
        "with_jbig": True,
        "with_jpeg": "libjpeg",
        "with_openjp2": True,
        "with_pango": True,
        "with_png": True,
        "with_tiff": True,
        "with_webp": False,
        "with_xml2": True,
        "with_freetype": True,
        "with_djvu": False,
        "with_openmp": True,
        "utilities": True,
    }
    exports_sources = "patches/*"
    win_bash = True
    _autotools = None

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def layout(self):
        self.folders.source = self._source_subfolder
        self.folders.build = self._build_subfolder

    @property
    def _modules(self):
        return ["Magick++", "MagickWand", "MagickCore"]

    def validate(self):
        if self.settings.os == "Windows" and scm.Version(self.version) < "7.1.0-45":
            raise ConanInvalidConfiguration(
                "This version of ImageMagick can't be built on Windows!"
            )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        if microsoft.is_msvc(self):
            self.options.with_openmp = False
            # FIXME: openexr causes link errors on windows
            self.options.with_openexr = False

    @property
    def _settings_build(self):
        return getattr(self, "settings_build", self.settings)

    def build_requirements(self):
        if hasattr(self, "settings_build"):
            self.build_requires("automake/1.16.5")
        if self._settings_build.os == "Windows" and not tools.get_env("CONAN_BASH_PATH"):
            self.build_requires("msys2/cci.latest")
        self.build_requires("pkgconf/1.7.4")

    def requirements(self):
        if self.options.with_zlib:
            self.requires("zlib/1.2.12")
        if self.options.with_bzlib:
            self.requires("bzip2/1.0.8")
        if self.options.with_lzma:
            self.requires("xz_utils/5.2.5")
        if self.options.with_lcms:
            self.requires("lcms/2.11")
        if self.options.with_openexr:
            self.requires("openexr/2.5.7")
        if self.options.with_heic:
            self.requires("libheif/1.12.0")
        if self.options.with_jbig:
            self.requires("jbig/20160605")
        if self.options.with_jpeg == "libjpeg":
            self.requires("libjpeg/9d")
        elif self.options.with_jpeg == "libjpeg-turbo":
            self.requires("libjpeg-turbo/2.1.0")
        if self.options.with_openjp2:
            self.requires("openjpeg/2.4.0")
        if self.options.with_pango:
            self.requires("pango/1.50.7")
        if self.options.with_png:
            self.requires("libpng/1.6.37")
        if self.options.with_tiff:
            self.requires("libtiff/4.3.0")
        if self.options.with_webp:
            self.requires("libwebp/1.2.0")
        if self.options.with_xml2:
            self.requires("libxml2/2.9.10")
        if self.options.with_freetype:
            self.requires("freetype/2.10.4")
        if self.options.with_djvu:
            # FIXME: missing djvu recipe
            self.output.warn(
                "There is no djvu package available on Conan (yet). This recipe will use the one present on the system (if available)."
            )

    def source(self):
        files.get(self,
                  **self.conan_data["sources"][self.version],
                  strip_root=True)

    def generate(self):
        tc = AutotoolsToolchain(self)
        env = tc.environment()

        if microsoft.is_msvc(self):
            # FIXME: it seems that PKG_CONFIG_PATH is added as a unix style path
            env.unset("PKG_CONFIG_PATH")
            env.define("PKG_CONFIG_PATH", self.generators_folder)

            # FIXME: otherwise configure reports "ld" as its linker
            env.define("LD", "link")
            # use "compile" script as a compiler driver for cl, since autotools
            # doesn't work well with cl
            compile_script = microsoft.unix_path(self, os.path.join(
                os.getcwd(), self._source_subfolder, "config", "compile"))
            env.define("CC", f'{compile_script} cl.exe -nologo')
            env.define("CXX", f"{compile_script} cl.exe -nologo")

        tc.generate(env)

        # FIXME: without this, AutotoolsDeps generates unix style path
        self.win_bash = False
        td = AutotoolsDeps(self)

        # AutotoolsDeps uses /LIBPATH: which is a linker argument that
        # will be pased incorrectly to the msvc compiler by autotools
        # this replaces /LIBPATH: by  -L, which the "compile" script will
        # fix for us, and replaces /I by -I, which works with the compile script
        env = td.environment
        if microsoft.is_msvc(self):
            ldflags = env.vars(self)["LDFLAGS"].replace("/LIBPATH:", "-L").replace("\\", "/")
            cppflags = env.vars(self)["CPPFLAGS"].replace("/I", "-I").replace("\\", "/")
            env.define("LDFLAGS", ldflags)
            env.define("CPPFLAGS", cppflags)

        # AutotoolsDeps adds all dependencies in the LIBS variable.
        # all these libs make `libtool` do the extra work of finding where they
        # are, which seems to fail on Windows
        # this variable is not needed anyways, since autotools will link against
        # required libraries by itself
        env.unset("LIBS")
        td.generate()

        self.win_bash = True

        pd = PkgConfigDeps(self)
        pd.generate()

    def build(self):
        files.apply_conandata_patches(self)

        if microsoft.is_msvc(self):
            # jpeg library is named as libjpeg in Windows
            files.replace_in_file(self, os.path.join(
                self.source_folder, "configure"), "-ljpeg", "-llibjpeg")

            # AutotoolsDeps makes cl include directories of all dependencies and
            # link against all dependencies, which is useful since this is the only
            # way for imagemagick to find libjpeg for example
            # but one of these dependencies defines the "select" function, which
            # makes imagemagick incorrectly think that Windows supports the POSIX
            # select function
            files.replace_in_file(self, os.path.join(self.source_folder, "configure"),
                                  "#define HAVE_SELECT 1", "/* #undef HAVE_SELECT */")

            self.conf["tools.microsoft.bash:subsystem"] = "msys2"

        # FIXME: change pangocairo pkg-config component name in the pango recipe
        try:
            files.rename(
                self,
                os.path.join(self.folders.generators_folder, "pango-pangocairo.pc"),
                os.path.join(self.folders.generators_folder, "pangocairo.pc"))
        except:
            pass

        autotools = self._build_configure()
        autotools.make()

    def _build_configure(self):
        if self._autotools:
            return self._autotools
        self._autotools = Autotools(self)

        def yes_no(o):
            return "yes" if o else "no"

        args = [
            "--disable-docs",
            "--with-perl=no",
            "--with-x=no",
            "--with-fontconfig=no",
            f"--enable-shared={format(yes_no(self.options.shared))}",
            f"--enable-static={format(yes_no(not self.options.shared))}",
            f"--enable-hdri={format(yes_no(self.options.hdri))}",
            f"--with-quantum-depth={format(self.options.quantum_depth)}",
            f"--with-zlib={format(yes_no(self.options.with_zlib))}",
            f"--with-bzlib={format(yes_no(self.options.with_bzlib))}",
            f"--with-lzma={format(yes_no(self.options.with_lzma))}",
            f"--with-lcms={format(yes_no(self.options.with_lcms))}",
            f"--with-openexr={format(yes_no(self.options.with_openexr))}",
            f"--with-heic={format(yes_no(self.options.with_heic))}",
            f"--with-jbig={format(yes_no(self.options.with_jbig))}",
            f"--with-jpeg={format(yes_no(self.options.with_jpeg))}",
            f"--with-openjp2={format(yes_no(self.options.with_openjp2))}",
            f"--with-pango={format(yes_no(self.options.with_pango))}",
            f"--with-png={format(yes_no(self.options.with_png))}",
            f"--with-tiff={format(yes_no(self.options.with_tiff))}",
            f"--with-webp={format(yes_no(self.options.with_webp))}",
            f"--with-xml={format(yes_no(self.options.with_xml2))}",
            f"--with-freetype={format(yes_no(self.options.with_freetype))}",
            f"--with-djvu={format(yes_no(self.options.with_djvu))}",
            f"--with-utilities={format(yes_no(self.options.utilities))}",
            "--without-gdi32" # FIXME
        ]
        if not self.options.with_openmp:
            args.append("--disable-openmp")

        self._autotools.configure(args=args)

        return self._autotools

    def package(self):
        autotools = self._build_configure()
        if microsoft.is_msvc(self):
            # FIXME: On Windows, Autotools sets DESTDIR as a Windows path
            autotools.install([f"DESTDIR={microsoft.unix_path(self, self.package_folder)}"])
        else:
            autotools.install()

        with tools.chdir(self.package_folder):
            # remove undesired files
            files.rmdir(self, os.path.join("lib", "pkgconfig"))  # pc files
            files.rmdir(self, "etc")
            files.rmdir(self, "share")
            tools.remove_files_by_mask("lib", "*.la")

            if microsoft.is_msvc(self) and self.options.shared:
                for m in self._modules:
                    files.rename(self,
                                 os.path.join("lib", f"{self._libname(m)}.dll.lib"),
                                 os.path.join("lib", f"{self._libname(m)}.lib"))

        self.copy(pattern="LICENSE", dst="licenses", src=self._source_subfolder)

    def _libname(self, library):
        suffix = "HDRI" if self.options.hdri else ""
        return "{}-{}.Q{}{}".format(
            library,
            scm.Version(self.version).major,
            self.options.quantum_depth,
            suffix,
        )

    def package_info(self):
        # FIXME model official FindImageMagick https://cmake.org/cmake/help/latest/module/FindImageMagick.html
        bin_path = os.path.join(self.package_folder, "bin")
        self.output.info(f"Appending PATH environment variable: {bin_path}")
        self.env_info.PATH.append(bin_path)

        core_requires = []
        if self.options.with_zlib:
            core_requires.append("zlib::zlib")
        if self.options.with_bzlib:
            core_requires.append("bzip2::bzip2")
        if self.options.with_lzma:
            core_requires.append("xz_utils::xz_utils")
        if self.options.with_lcms:
            core_requires.append("lcms::lcms")
        if self.options.with_openexr:
            core_requires.append("openexr::openexr")
        if self.options.with_heic:
            core_requires.append("libheif::libheif")
        if self.options.with_jbig:
            core_requires.append("jbig::jbig")
        if self.options.with_jpeg:
            core_requires.append("{0}::{0}".format(self.options.with_jpeg))
        if self.options.with_openjp2:
            core_requires.append("openjpeg::openjpeg")
        if self.options.with_pango:
            core_requires.append("pango::pango")
        if self.options.with_png:
            core_requires.append("libpng::libpng")
        if self.options.with_tiff:
            core_requires.append("libtiff::libtiff")
        if self.options.with_webp:
            core_requires.append("libwebp::libwebp")
        if self.options.with_xml2:
            core_requires.append("libxml2::libxml2")
        if self.options.with_freetype:
            core_requires.append("freetype::freetype")

        if self.settings.os == "Linux":
            self.cpp_info.components["MagickCore"].system_libs.append("pthread")

        self.cpp_info.components["MagickCore"].defines.append(
            f"MAGICKCORE_QUANTUM_DEPTH={self.options.quantum_depth}"
        )
        self.cpp_info.components["MagickCore"].defines.append(
            f"MAGICKCORE_HDRI_ENABLE={int(bool(self.options.hdri))}"
        )
        self.cpp_info.components["MagickCore"].defines.append(
            "_MAGICKDLL_=1" if self.options.shared else "_MAGICKLIB_=1"
        )

        imagemagick_include_dir = (
            f"include/ImageMagick-{scm.Version(self.version).major}"
        )

        self.cpp_info.components["MagickCore"].includedirs = [imagemagick_include_dir]
        self.cpp_info.components["MagickCore"].libs.append(self._libname("MagickCore"))
        self.cpp_info.components["MagickCore"].requires = core_requires
        self.cpp_info.components["MagickCore"].names["pkg_config"] = ["MagicCore"]
        self.cpp_info.components["MagickCore"].set_property("pkg_config_name", "MagickCore")

        if self.options.with_openmp:
            self.cpp_info.components["MagickCore"].cflags.append("-fopenmp")
            self.cpp_info.components["MagickCore"].cxxflags.append("-fopenmp")
            self.cpp_info.components["MagickCore"].sharedlinkflags.append("-fopenmp")

        self.cpp_info.components[self._libname("MagickCore")].requires = ["MagickCore"]
        self.cpp_info.components[self._libname("MagickCore")].names["pkg_config"] = [
            self._libname("MagickCore")
        ]
        self.cpp_info.components[self._libname("MagickCore")].set_property(
            "pkg_config_name", self._libname("MagickCore"))

        self.cpp_info.components["MagickWand"].includedirs = [
            imagemagick_include_dir + "/MagickWand"
        ]
        self.cpp_info.components["MagickWand"].libs = [self._libname("MagickWand")]
        self.cpp_info.components["MagickWand"].requires = ["MagickCore"]
        self.cpp_info.components["MagickWand"].names["pkg_config"] = ["MagickWand"]
        self.cpp_info.components["MagickWand"].set_property("pkg_config_name", "MagickWand")

        self.cpp_info.components[self._libname("MagickWand")].requires = ["MagickWand"]
        self.cpp_info.components[self._libname("MagickWand")].names[
            "pkg_config"
        ] = self._libname("MagickWand")
        self.cpp_info.components[self._libname("MagickWand")].set_property(
            "pkg_config_name", self._libname("MagickWand"))

        self.cpp_info.components["Magick++"].includedirs = [
            imagemagick_include_dir + "/Magick++"
        ]
        self.cpp_info.components["Magick++"].libs = [self._libname("Magick++")]
        self.cpp_info.components["Magick++"].requires = ["MagickWand"]
        self.cpp_info.components["Magick++"].names["pkg_config"] = ["Magick++"]
        self.cpp_info.components["Magick++"].set_property("pkg_config_name", "Magick++")

        self.cpp_info.components[self._libname("Magick++")].requires = ["Magick++"]
        self.cpp_info.components[self._libname("Magick++")].names[
            "pkg_config"
        ] = self._libname("Magick++")
        self.cpp_info.components[self._libname("Magick++")].set_property(
            "pkg_config_name", self._libname("Magick++"))
