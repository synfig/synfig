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

https://synfig.org/

[![Build Status](https://travis-ci.com/synfig/synfig.svg?branch=master)](https://travis-ci.com/synfig/synfig)

(TODO: add more info)


Installing
----------

Old autotools instructions can be found [here][autotools].

We're currently in the [process of switching to CMake build system][cmake] and
you're welcome to take part in testing and improving it.

#### Debian/Ubuntu
1. Download the appimage file from the website.
2. Left-click on the file and select `properties`. Navigate to `permissions` and check the box that says `Allow executing file as program`. Finally, run the  file.

For more information see https://discourse.appimage.org/t/how-to-make-an-appimage-executable/80.

#### Windows
1. Download the `.exe` file from the website.
2. Run the file. A wizard will then guide you through the rest of the installation.

#### OSX
1. Download the `.dmg` file from the website.
2. Drag the file into your applications directory, then run it.

If you get an error message like "can't be opened because it comes from a non identified developer", then do the fllowing:

1. Locate the Synfig app in Finder (don’t use Launchpad to do this - Launchpad doesn’t allow you to access the shortcut menu)
2. Control-click the app icon, choose "Open" from the shortcut menu.
3. Click Open.

The app is saved as an exception to your security settings, and you can open it in the future by double-clicking it just as you can any registered app.

Note: You can also grant an exception for a blocked app by clicking the “Open Anyway” button in the General pane of Security & Privacy preferences. This button is available for about an hour after you try to open the app. To open this pane, choose Apple menu > System Preferences, click Security & Privacy, then click General.

For more details please refer to this page - https://support.apple.com/kb/PH25088?locale=en_US


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

- boost (system)
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
