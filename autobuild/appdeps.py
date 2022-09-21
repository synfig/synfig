import argparse
import subprocess
import shutil
import os


def _change_rpath(lib, rpath, patchelf_program):
    val = subprocess.run(args=[patchelf_program, "--set-rpath", rpath, lib])
    if val.returncode != 0:
        print(f"Error: Failed setting rpath of {lib} to {rpath}!")
        exit(-1)


def change_rpath(libs, rpath, patchelf_program):
    for lib in libs:
        _change_rpath(lib, rpath, patchelf_program)


def is_executable_or_shared(file):
    val = subprocess.run(args=["file", "-L", "--mime-type", file],
                         capture_output=True)
    output = val.stdout.decode("utf-8")

    mime_types = [
        "application/x-sharedlib", "application/x-executable",
        "application/x-pie-executable"
    ]
    for mime_type in mime_types:
        if mime_type in output:
            return True

    return False


def find_deps(file, libdirs):
    val = subprocess.run(args=["ldd", file],
                         env={"LD_LIBRARY_PATH": ":".join(libdirs)},
                         capture_output=True)

    output = val.stdout.decode("utf-8")

    deps = {}
    for line in output.splitlines():
        # ldd writes lines in the form of "<lib-name> => <lib-path> (<addr>)"
        # or in the form "<lib-name> (<addr>)"
        # we're only interested in the first form
        parts = line.split("=>")
        if len(parts) == 1:
            continue

        lib_name = parts[0].strip()
        lib_path = parts[1].split(" (")[0].strip()
        if lib_path == "not found":
            lib_path = None

        deps[lib_name] = lib_path

    return deps


def copy_deps(deps, outdir, libdirs, rpath):
    copied_deps = []
    for libname, libpath in deps.items():
        reallibpath = os.path.realpath(libpath)
        found_lib = False
        for libdir in libdirs:
            if os.path.commonpath([reallibpath,
                                   libdir]) == os.path.commonpath([libdir]):
                found_lib = True
                break

        if not found_lib:
            continue

        newlibpath = os.path.join(outdir, libname)
        shutil.copy(reallibpath, newlibpath)

        copied_deps.append(newlibpath)

    return copied_deps


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=
        "Copy dependencies of executables and shared objects, and change their rpath"
    )
    parser.add_argument("file_path", metavar="file", type=str)
    parser.add_argument("-L",
                        action="append",
                        metavar="libdirs",
                        dest="libdirs",
                        type=str)
    parser.add_argument("--outdir",
                        metavar="outdir",
                        dest="outdir",
                        type=str,
                        required=True)
    parser.add_argument("--patchelf-program",
                        metavar="patchelf-program",
                        dest="patchelf_program",
                        type=str)
    parser.add_argument("--set-rpath", metavar="rpath", dest="rpath", type=str)
    args = parser.parse_args()

    if not os.path.exists(args.file_path):
        print(f"Error: No such file or directory ({args.file_path})!")
        exit(-1)

    file_path = os.path.abspath(args.file_path)

    libdirs = []
    for searchdir in (args.libdirs or []):
        if not os.path.exists(searchdir):
            print(f"Error: No such file or directory ({searchdir})!")
            exit(-1)
        else:
            libdirs.append(os.path.abspath(searchdir))

    if not os.path.exists(args.outdir):
        os.makedirs(args.outdir)
    outdir = os.path.abspath(args.outdir)

    if not is_executable_or_shared(args.file_path):
        print("Error: File is neither a shared library nor an executable!")
        exit(-1)

    if args.patchelf_program:
        patchelf_program = args.patchelf_program
    else:
        patchelf_program = "patchelf"

    deps = find_deps(file_path, libdirs)

    unresolved_deps = [key for key, value in deps.items() if value is None]
    if len(unresolved_deps) > 0:
        for dep in unresolved_deps:
            print(f"Error: Could not resolve path for dependency \"{dep}\"")
        exit(-1)

    copied_deps = copy_deps(deps, outdir, libdirs, args.rpath)

    if args.rpath:
        change_rpath(copied_deps, args.rpath, patchelf_program)
