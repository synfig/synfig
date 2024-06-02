import os
import subprocess
import shutil
import glob
from distutils.dir_util import copy_tree as copytree

# Define ANSI color codes
COLOR_RED = '\033[91m'
COLOR_GREEN = '\033[92m'
COLOR_YELLOW = '\033[93m'
COLOR_RESET = '\033[0m'

PROJECT_ROOT = os.path.abspath(f"{os.path.dirname(__file__)}/../..")
#BUILD_ROOT = f"{PROJECT_ROOT}/cmake-build-artem/output/Debug"
BUILD_ROOT = f"{PROJECT_ROOT}/_production/build"
BUNDLE_ROOT = f"{PROJECT_ROOT}/autobuild/macos-bundler/SynfigStudio.app"
TEMPLATE_ROOT = f"{PROJECT_ROOT}/autobuild/osx/app-template/"
LAUNCHER_FILE = f"{PROJECT_ROOT}/autobuild/osx/synfig_osx_launcher.cpp"
PYTHON_VERSION = "3.12"
dmg_filename = "SynfigStudio-1.4.5-x86_64-macos.dmg"


def print_colored(text, color):
    print(color + text + COLOR_RESET)


def otool(lib_path):
    o = subprocess.Popen(['/usr/bin/otool', '-L', lib_path], stdout=subprocess.PIPE)
    for line in map(lambda s: s.decode('ascii'), o.stdout):
        if line[0] == '\t':
            yield line.split(' ', 1)[0][1:]


def is_system_library(lib):
    return lib.startswith("/System/Library") or lib.startswith("/usr/lib")


def is_relocatable_library(lib):
    return lib.startswith("@rpath/")


def is_loader_path(lib):
    return lib.startswith("@loader_path/")


def change_non_system_libraries_path(libraries, binary):
    args = []
    for lib in libraries.keys():
        new_path = os.path.join("@rpath/", os.path.basename(libraries[lib]))
        args.extend(["-change", lib, new_path])
    if len(args) == 0:
        return

    args.extend(["-add_rpath", "@executable_path/../lib/"])
    subprocess.run(['install_name_tool', *args, binary], check=True, capture_output=True)
    subprocess.run(['codesign', '--force', '-s', '-', binary], check=True, capture_output=True)


def extract_lc_rpaths(filename):
    """
    :param filename:
    :return: list of rpaths from filename or empty list if no rpaths is found

    otool -l example output:

Load command 31
          cmd LC_RPATH
      cmdsize 104
         path /Users/user/synfig/cmake-build-debug/output/Debug/lib (offset 12)
Load command 32
      cmd LC_FUNCTION_STARTS
    """

    output = subprocess.check_output(["otool", "-l", filename], encoding="utf-8").splitlines()

    lc_rpath_paths = []
    index = 0
    for line in output:
        index += 1
        if "cmd LC_RPATH" in line:
            path = output[index+1].split("path ")[1].split(" (offset")[0]
            lc_rpath_paths.append(path)

    return lc_rpath_paths


def resolve_loader_path(lib_path, owner):
    owner_folder = os.path.dirname(owner)
    full_path = lib_path.replace("@loader_path", owner_folder)
    real_path = os.path.realpath(full_path)
    if os.path.exists(real_path):
        return real_path

    print(f"Detailed info. lib_path: {lib_path}, owner: {owner}")
    raise FileNotFoundError({real_path})


def resolve_rpath(lib_path, owner):
    lib_path = lib_path[7:]  # skip `@rpath/`

    # first try to find dependency in the same folder as owner
    owner_folder = os.path.dirname(owner)
    main_real_path = os.path.realpath(f"{owner_folder}/{lib_path}")
    if os.path.exists(main_real_path):
        return main_real_path

    # if dependency is not found then try to extract LC_RPATH's from executable and look in this places
    for rpath in extract_lc_rpaths(owner):
        if is_loader_path(rpath):  # for @loader_path use binary folder
            rpath = rpath.replace("@loader_path", owner_folder)

        real_path = os.path.realpath(f"{rpath}/{lib_path}")
        if os.path.exists(real_path):
            return real_path

    print(f"Detailed info. lib_path: {lib_path}, owner: {owner}")
    raise FileNotFoundError({main_real_path})


def get_dependencies(binary_file_path):
    dependencies = set(otool(binary_file_path))
    collected = dict()  # key -> original dependency, value -> resolved full path to dependency
    for file in dependencies:
        if is_system_library(file):
            continue
        if is_relocatable_library(file):
            collected[file] = resolve_rpath(file, binary_file_path)
            continue
        if is_loader_path(file):
            collected[file] = resolve_loader_path(file, binary_file_path)
            continue

        try:
            collected[file] = os.path.realpath(file)
        except FileNotFoundError:
            print_colored(f"Error get dependency for binary: `{binary_file_path}` ({file})", COLOR_RED)

    return collected


# lib_path - path where dependency libraries will be copied
# binary_file_path - binary for which dependencies will be collected
# should be already inside App bundle
def copy_dependencies(binary_path, lib_path):
    # Update binary libraries
    binary_dependencies = get_dependencies(binary_path)
    change_non_system_libraries_path(binary_dependencies, binary_path)

    # Update dependencies libraries
    need_checked = binary_dependencies.values()
    already_processed = set()
    while need_checked:
        checking = set(need_checked)
        need_checked = set()
        for dependency_path in checking:
            new_path = os.path.join(lib_path, os.path.basename(dependency_path))
            if os.path.exists(new_path):  # file was already processed, skip it
                continue

            print(f"Processing {dependency_path}...")
            shutil.copyfile(dependency_path, new_path)

            need_relinked = get_dependencies(dependency_path)
            change_non_system_libraries_path(need_relinked, new_path)
            need_checked.update(need_relinked.values())
        already_processed.update(checking)
        need_checked.difference_update(already_processed)


def relocate_binary(src_file, dst, lib_dir):
    shutil.copy2(src_file, dst)
    full_dst_path = dst
    if os.path.isdir(dst):
        full_dst_path = os.path.join(dst, os.path.basename(src_file))
    subprocess.run(["chmod", "+w", full_dst_path], check=True)

    copy_dependencies(full_dst_path, lib_dir)


def relocate_folder(src_path, dst_path, lib_dir):
    os.makedirs(dst_path, exist_ok=True)
    print_colored(f"Relocating `{src_path}`...", COLOR_GREEN)
    for file in glob.glob(f'{src_path}'):
        relocate_binary(file, os.path.join(dst_path, os.path.basename(file)), lib_dir)


def make_dmg():
    subprocess.run(['./mk_dmg.sh'], check=True, capture_output=False)

def main():
s    if os.path.exists(BUNDLE_ROOT):
        print("Cleaning up from previous packaging...")
        shutil.rmtree(BUNDLE_ROOT)

    pkg_dir = subprocess.run(['brew', '--prefix'], check=True, capture_output=True, text=True).stdout.strip()

    resource_dir = os.path.join(BUNDLE_ROOT, 'Contents', 'Resources')
    lib_dir = os.path.join(resource_dir, 'lib')

    print_colored(f"Homebrew packages dir: {pkg_dir}", COLOR_YELLOW)
    # print(f"'App template dir: {template_dir}'")

    print(f"Copying .app template from {TEMPLATE_ROOT}...")
    shutil.copytree(TEMPLATE_ROOT, BUNDLE_ROOT)
    os.makedirs(lib_dir, exist_ok=True)

    print_colored("Preparing FontConfig...", COLOR_YELLOW)
    shutil.copytree(f"{pkg_dir}/etc/fonts/", f"{resource_dir}/etc/fonts/")

    print_colored("Preparing Synfig...", COLOR_YELLOW)
    os.makedirs(os.path.join(resource_dir, 'bin'), exist_ok=True)
    relocate_binary(f"{BUILD_ROOT}/bin/synfig", f"{resource_dir}/bin/", lib_dir)
    relocate_binary(f"{BUILD_ROOT}/bin/synfigstudio", f"{resource_dir}/bin/", lib_dir)
    # example of how syntax can be improved
    # synfig_files = ['bin/synfig', 'bin/synfigstudio', 'lib/synfig/modules/*.so', 'etc/', 'share/']
    # relocate(BUILD_ROOT, resource_dir, synfig_files, lib_dir)

    # Cairo (for v1.4)


    # Image Magick
    magick_dir = f"{pkg_dir}/opt/imagemagick"
    magick_modules_dir = f"lib/ImageMagick/modules-Q16HDRI"
    magick_config_dir = f"lib/ImageMagick/config-Q16HDRI"
    #magick_modules_dir = f"{magick_dir}/lib/ImageMagick/modules-Q16HDRI"
    for file in ['animate', 'composite', 'convert']:
        relocate_binary(f"{magick_dir}/bin/{file}", f"{resource_dir}/bin/", lib_dir)

    # ImageMagick modules
    relocate_folder(f"{magick_dir}/{magick_modules_dir}/coders/*.so",
                    f"{resource_dir}/{magick_modules_dir}/coders/", lib_dir)
    relocate_folder(f"{magick_dir}/{magick_modules_dir}/filters/*.so",
                    f"{resource_dir}/{magick_modules_dir}/filters/", lib_dir)

    copytree(f"{magick_dir}/{magick_config_dir}/", f"{resource_dir}/{magick_config_dir}/")
    copytree(f"{magick_dir}/etc/", f"{resource_dir}/etc/")

    relocate_folder(f"{BUILD_ROOT}/lib/synfig/modules/*.so", f"{resource_dir}/lib/synfig/modules", lib_dir)
    copytree(f"{BUILD_ROOT}/etc/", f"{resource_dir}/etc/")
    copytree(f"{BUILD_ROOT}/share/", f"{resource_dir}/share/")

    print_colored("Preparing FFMPEG...", COLOR_YELLOW)
    for file in ['ffmpeg', 'ffprobe', 'encodedv', 'sox']:
        relocate_binary(f"{pkg_dir}/bin/{file}", f"{resource_dir}/bin/", lib_dir)

    print_colored("Preparing Gtk3...", COLOR_YELLOW)
    relocate_binary(f"{pkg_dir}/bin/gdk-pixbuf-query-loaders", f"{resource_dir}/bin/", lib_dir)
    relocate_binary(f"{pkg_dir}/bin/gdk-pixbuf-pixdata", f"{resource_dir}/bin/", lib_dir)
    pixbuf_loaders = "lib/gdk-pixbuf-2.0/2.10.0/loaders"
    relocate_folder(f"{pkg_dir}/{pixbuf_loaders}/*.so", f"{resource_dir}/{pixbuf_loaders}/", lib_dir)

    copytree(f"{pkg_dir}/share/gir-1.0", f"{resource_dir}/share/gir-1.0")
    copytree(f"{pkg_dir}/share/locale", f"{resource_dir}/share/locale")

    for folder in ['lib/gtk-3.0/3.0.0/immodules', 'lib/gtk-3.0/3.0.0/printbackends']:
        relocate_folder(f"{pkg_dir}/{folder}/*.so", f"{resource_dir}/{folder}/", lib_dir)

    print_colored("Preparing themes and icons...", COLOR_YELLOW)
    copytree(f"{pkg_dir}/share/themes/", f"{resource_dir}/share/themes/")
    copytree(f"{pkg_dir}/share/icons/", f"{resource_dir}/share/icons/")

    print_colored("Preparing gsettings-desktop-schemas...", COLOR_YELLOW)
    copytree(f"{pkg_dir}/share/glib-2.0/schemas/", f"{resource_dir}/share/glib-2.0/schemas/")
    subprocess.run(['glib-compile-schemas', f"{resource_dir}/share/glib-2.0/schemas"], check=True)

    print_colored("Preparing MLT...", COLOR_YELLOW)
    relocate_folder(f"{pkg_dir}/lib/mlt/*.so", f"{resource_dir}/lib/mlt/", lib_dir)

    print_colored("Preparing Python3...", COLOR_YELLOW)
    python_path = f"Frameworks/Python.framework/Versions/{PYTHON_VERSION}"
    python_lib_path = f"{python_path}/lib/python{PYTHON_VERSION}/"
    python_app_path = f"{python_path}/Resources/Python.app/Contents/MacOS"
    python_local_site = f"{resource_dir}/lib/python{PYTHON_VERSION}/site-packages/"
    os.makedirs(f"{resource_dir}/{python_lib_path}")
    #os.makedirs(f"{lib_dir}/{python_app_path}")
    relocate_binary(f"{pkg_dir}/{python_app_path}/Python", f"{resource_dir}/bin/python3", lib_dir)

    #mkdir -p ../../../../../../lib/python${PYTHON_VERSION}/site-packages
    os.makedirs(python_local_site)
    # rsync -av --exclude "__pycache__" "${MACPORTS}${PKG_PREFIX}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/" "${APPCONTENTS}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/"
    subprocess.run(['rsync', '-av', '--exclude', '__pycache__', f"{pkg_dir}/{python_lib_path}",
                    f"{resource_dir}/{python_lib_path}"], check=True, capture_output=True)
    # rsync -av --exclude "__pycache__" /usr/local/lib/python${PYTHON_VERSION}/site-packages/lxml* "${APPCONTENTS}/lib/python${PYTHON_VERSION}/site-packages/"
    subprocess.run(['rsync', '-avL', '--exclude', '__pycache__', f"{pkg_dir}/{python_lib_path}/site-packages/lxml",
                    python_local_site], check=True, capture_output=True)
    # ln -sf ../../../../../../lib/python${PYTHON_VERSION}/site-packages site-packages
    subprocess.run(['ln', '-sf', f"../../../../../../lib/python{PYTHON_VERSION}/site-packages",
                    f"{resource_dir}/{python_lib_path}/site-packages"], check=True, capture_output=True)

    print_colored("Compiling launcher...", COLOR_YELLOW)
    subprocess.run(["clang++", LAUNCHER_FILE, '-O3', '-o', f'{BUNDLE_ROOT}/Contents/MacOS/SynfigStudio'], check=True)

    print_colored("Test signature...", COLOR_YELLOW)
    subprocess.run(['codesign', '--force', '-s', '-', f"{BUNDLE_ROOT}/Contents/MacOS/SynfigStudio.sh"], check=True)
    subprocess.run(['codesign', '--force', '-s', '-', BUNDLE_ROOT], check=True)
    subprocess.run(['codesign', '-vv', '--deep-verify', BUNDLE_ROOT], check=True)

    print_colored("Packing .dmg archive...", COLOR_YELLOW)
    # subprocess.run(['tar', 'czf', './test.tgz', BUNDLE_ROOT])

    subprocess.run(['hdiutil', 'create', '-volname', 'Synfig', '-ov',  # overwrite existing file
                    '-megabytes', '700', dmg_filename, '-srcfolder', BUNDLE_ROOT], check=True)

    print_colored("Signing...", COLOR_YELLOW)
    subprocess.run(['codesign', '--force', '-s', '-', dmg_filename], check=True)

    print_colored("Packing done!", COLOR_GREEN)


if __name__ == "__main__":
    main()
