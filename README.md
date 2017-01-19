Synfig Studio
=============

About
-----

Synfig Studio is a free and open-source 2D animation software, designed as
powerful industrial-strength solution for creating film-quality animation using
a vector and bitmap artwork. It eliminates the need to create animation
frame-by frame, allowing you to produce 2D animation of a higher quality with
fewer people and resources. Synfig Studio is available for Windows, Linux and
MacOS X.

http://synfig.org/

(TODO: add more info)


Installing
----------

Old autotools instructions can be found [here][autotools].

We're currently in the [process of switching to CMake build system][cmake] and
you're welcome to take part in testing and improving it.


### CMake status

- all major components are buildable, installable and runnable
- building all components without installing ETL & core is not tested

Tested to work on (this will be updated as reports come in):

- Debian Sid


### Cleanup

If you have previous synfig build installed in system path (e.g. `/usr/local/`),
you are recommended to uninstall that.


### Dependencies

You need development & library packages of the following libs:

- boost (system, filesystem, program_options, chrono)
- zlib
- libsigc++-2.0
- glibmm-2.4
- giomm-2.4
- cairo
- libxml++-2.6
- mlt++
- fftw3
- pango
- gtkmm-3.0 (only for studio)
- gettext (probably optional)
- some threading support (e.g. pthread)

Generally CMake will throw error if it doesn't find something, so you can just
run it and see what's missing. Also note that this list might not be full.


### CMake backend

CMake provides generators for multiple build systems. You can use default `make`
or `ninja`, which should generally work somewhat faster. The following
configuration commands assume you want to use `ninja`. If you don't, remove
`-GNinja` from all commands containing it. All the building commands here are
invoked via cmake to make them (almost) backend-agnostic, but you can run `make`
or `ninja` directly (i.e.
`ninja all test` instead of `cmake --build . -- all test`).

### Build options

You may want to add `-jN` (where N is amount of threads you want to run) option
to build commands, because default for `make` is to run single-threaded and
`ninja` tends to use too much threads which eat up your RAM (may vary).

### Building

```
$ pushd ETL
$ mkdir build && pushd build
$ cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ cmake --build . -- all test
$ sudo cmake --build . -- install
$ popd # build
$ popd # ETL
$ pushd synfig-core
$ mkdir build && pushd build
$ cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fdiagnostics-color"
$ cmake --build . -- all
$ sudo cmake --build . -- install
$ popd # build
$ popd # synfig-core
$ pushd synfig-studio
$ mkdir build && pushd build
$ cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fdiagnostics-color"
$ cmake --build . -- all
# this will take a while; alternatively, you can move/copy required images
# to build/images directory and skip this step
$ cmake --build . -- build_images
$ sudo cmake --build . -- install
$ popd # build
$ popd # synfig-studio
```

[cmake]:        https://github.com/synfig/synfig/issues/279
[autotools]:    http://wiki.synfig.org/Dev:Build_Instructions
