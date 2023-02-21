import os
import sys
import fileinput
import shlex


def fix_ninja_build(build_folder):
    for line in fileinput.input(os.path.join(build_folder, "build.ninja"), inplace=True):
        # the command response file for linking gtk exceeds msvc linker limit
        # this hack fixes this by removing duplicates in the link arguments
        # inside the build.ninja file
        # see https://docs.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk1170?view=msvc-170
        idx = line.find("LINK_ARGS =")
        if idx >= 0:
            parts = shlex.split(line[idx + len("LINK_ARGS ="):])
            print("{} {}".format(
                line[: idx + len("LINK_ARGS =")],
                " ".join((f'"{v}"' for v in dict.fromkeys(parts)))), end='')
            continue

        # in windows, ninja uses CreateProcess which has a limit of 32767 chars
        # gnome.mkenums uses absolute paths, which increases the command length
        # see: https://github.com/mesonbuild/meson/issues/6710
        # this hack replaces all absolute paths of the build subfolder
        # with relative paths to decrease the length of the command
        idx = line.find("COMMAND = ")
        if idx >= 0:
            parts = shlex.split(line[idx + len("COMMAND ="):])
            print("{} {}".format(
                line[: idx + len("COMMAND =")],
                " ".join((f'"{v.replace(build_folder, ".")}"' for v in parts))))
            continue

        print(line, end='')


if __name__ == "__main__":
    fix_ninja_build(sys.argv[1])
