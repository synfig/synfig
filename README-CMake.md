### CMake status

We're currently in the [process of switching to CMake build system][cmake] and
you're welcome to take part in testing and improving it.

- all major components are buildable, installable and runnable
- building all components without installing ETL & core is not tested

Tested to work on (this will be updated as reports come in):

- Debian Sid


### Dependencies

For full list  of required libraries please check this page - https://synfig-docs-dev.readthedocs.io/en/latest/common/dependencies.html

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

Note for packagers:
On Unix systems, the Synfig Studio looks out for its symbols, sounds and plugins at CMAKE_INSTALL_PREFIX/share.
If you want to change the prefix you need to overwrite the `DATA_PREFIX` variable.
Pass `-DDATA_PREFIX=/usr` for example to made Synfig Studio look at /usr/share for its data.

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

### Auto building (portable)
You can also build synfig-studio and run it without installation.
To build change the directory to the project path (where README-CMake.md is located) and start the build process with:
```
$ ./2-build-cmake.sh
```

This can take some time.
If the build was successful, cd to _debug/out and after that you can run Synfig Studio directly with:
```
$ ./run-portable.sh
```

It is also passible to run the Synfig CLI program:
```
$ ./run-portable.sh "synfig [PARAMETERS]"
```

You can also create a bash session, where you can run the binary's directly:
```
$ ./run-portable.sh bash
```

Now you can type "synfig" or "synfigstudio" to run the build programs. It now behaves exactly as if Synfig Studio was installed to the system.
To exit the session just type the following:
```
$ exit
```

For for options open the build-cmake.sh in a text editor.
The available options are on the top of the file (the same also in the run-portable.sh).

[cmake]:        https://github.com/synfig/synfig/issues/279
