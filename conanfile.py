from conans import ConanFile, AutoToolsBuildEnvironment, CMake

class SynfigConan(ConanFile):
   settings = "os", "compiler", "build_type", "arch"
   generators = "pkg_config"
   requires = "cairo/1.17.4",\
              "harfbuzz/2.8.2",\
              "fribidi/1.0.10",\
              "glib/2.68.3"
   default_options = {}

   def build(self):
      cmake = CMake(self)
      cmake.configure()
      cmake.build()
