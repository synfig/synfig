import argparse
import os
import shutil
import subprocess
import logging
import re
import glob
import sys

def setup_logging(log_file_path=None):
    """
    Initializes logging to both the console and an optional file.
    """
    # Get the root logger
    logger = logging.getLogger()
    logger.setLevel(logging.INFO) # Set the lowest level of messages to handle

    # Clear any existing handlers to prevent duplicate logging
    if logger.hasHandlers():
        logger.handlers.clear()

    # Create a standard formatter
    log_formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

    # Always log to the console (stdout)
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setFormatter(log_formatter)
    logger.addHandler(console_handler)

    # If a log file path is provided, also log to the file
    if log_file_path:
        try:
            # Create a file handler. mode='w' will overwrite the log file on each run.
            file_handler = logging.FileHandler(log_file_path, mode='w')
            file_handler.setFormatter(log_formatter)
            logger.addHandler(file_handler)
            # Log the creation of the log file itself
            logging.info(f"Detailed log will be saved to: {log_file_path}")
        except IOError as e:
            logging.error(f"Could not open log file {log_file_path} for writing: {e}")

def find_system_executable(name):
    """
    Searches for an executable in standard system paths.
    """
    logging.info(f"Searching for system executable: '{name}'")
    
    # A list of standard locations where command-line tools are installed.
    search_paths = [
        "/usr/local/bin",
        "/opt/local/bin",
        "/usr/bin",
    ]
    
    homebrew_prefix = get_homebrew_prefix()
    if homebrew_prefix:
        # Prepending the brew path
        search_paths.insert(0, os.path.join(homebrew_prefix, "bin"))

    for path in search_paths:
        candidate = os.path.join(path, name)
        # Check if a file exists at this path and if it's executable.
        if os.path.exists(candidate) and os.access(candidate, os.X_OK):
            logging.info(f"Found '{name}' at: {candidate}")
            return os.path.realpath(candidate)

    logging.warning(f"Could not find executable '{name}' in any standard search path.")
    return None

def get_dependencies(binary_path):
    """Uses otool to find all non-system library dependencies for a given binary."""
    try:
        output = subprocess.check_output(["otool", "-L", binary_path], text=True)
        # Regex to capture the dependency path from each line
        deps = [m.group(1) for line in output.strip().split('\n')[1:] if (m := re.match(r'^\s*(@?[^\s]+)', line))]
        # Filter out standard macOS system libraries, which should not be bundled
        return [d for d in deps if not d.startswith(("/usr/lib", "/System/Library"))]
    except subprocess.CalledProcessError as e:
        logging.error(f"Error getting dependencies for {os.path.basename(binary_path)}: {e}")
        return []

def get_homebrew_prefix():
    """Dynamically finds the Homebrew installation prefix."""
    try:
        prefix = subprocess.check_output(["brew", "--prefix"], text=True, stderr=subprocess.DEVNULL).strip()
        logging.info(f"Detected Homebrew prefix: {prefix}")
        return prefix
    except (FileNotFoundError, subprocess.CalledProcessError):
        logging.warning("Could not execute 'brew --prefix'. Falling back to standard hardcoded paths.")
        return None

def resolve_library_path(lib_path, binary_path=None):
    """
    Tries to find the real path of a library on the system.
    It handles @rpath, absolute paths, and searches standard locations.
    """
    try:
        # 1. Handle @rpath by using the rpaths from the referring binary
        if lib_path.startswith("@rpath") and binary_path:
            rpath_lib = lib_path.split("@rpath/", 1)[1]
            resolved = resolve_rpath(binary_path, rpath_lib)
            if resolved: return resolved

        # 2. Handle direct paths that already exist
        if os.path.exists(lib_path):
            return os.path.realpath(lib_path)

        # 3. Search in standard and Homebrew locations
        lib_name_base = os.path.basename(lib_path).split('.dylib', 1)[0] # # Gets base name without extension
        search_paths = [
            "/opt/homebrew/lib",  # Core Homebrew libraries
            "/opt/homebrew/opt/*/lib",  # to cover all Homebrew formulae
            "/usr/local/opt/*/lib",  # Intel Homebrew formula libraries
            "/usr/local/opt/sqlite/lib", # Intel Homebrew SQLite
            "/opt/homebrew/opt/sqlite/lib", # ARM Homebrew SQLite
            "/usr/local/lib", # legacy homebrew installation directory
            "/opt/local/lib", # MacPorts installation directory
            "/usr/lib", # System libraries
            "/Library/Frameworks", # System-wide frameworks
            os.path.join(os.path.dirname(binary_path), "..", "lib") if binary_path else None
        ]

        # Regex pattern for versioned libraries
        version_pattern = re.compile(rf'^{re.escape(lib_name_base)}(\.\d+)*\.dylib$')

        for path in filter(None, search_paths):
            # Check for exact match first
            candidate = os.path.join(path, lib_name_base + ".dylib")
            if os.path.exists(candidate):
                return os.path.realpath(candidate)

            # Check for versioned matches
            if os.path.exists(path):
                for f in os.listdir(path):
                    if version_pattern.match(f):
                        return os.path.realpath(os.path.join(path, f))

        logging.warning(f"Could not resolve library path: {lib_path}")
        return None
    except Exception as e:
        logging.error(f"An unexpected error occurred while resolving library path for '{lib_path}': {e}")
        return None

def _extract_rpaths_from_otool(output, binary_path):
    """Parses 'otool -l' output to find and expand LC_RPATH entries."""
    rpaths = []
    binary_dir = os.path.dirname(binary_path)
    lines = output.splitlines()
    for i, line in enumerate(lines):
        if "cmd LC_RPATH" in line:
            for j in range(1, 5):
                if i + j < len(lines) and "path" in lines[i+j]:
                    path_line = lines[i+j]
                    rpath = path_line.strip().split("path ")[1].split(" (")[0].strip()
                    expanded = rpath
                    if "@loader_path" in rpath:
                        expanded = os.path.normpath(rpath.replace("@loader_path", binary_dir))
                    elif "@executable_path" in rpath:
                        # Assuming @executable_path points to the MacOS directory in a bundle
                        executable_dir = os.path.dirname(os.path.dirname(binary_dir))
                        expanded = os.path.normpath(rpath.replace("@executable_path", executable_dir))
                    rpaths.append(expanded)
                    break
    return rpaths

def resolve_rpath(binary_path, rpath_lib):
    """Finds a library using the rpaths embedded in the binary that needs it."""
    try:
        output = subprocess.check_output(["otool", "-l", binary_path], text=True)
        rpaths = _extract_rpaths_from_otool(output, binary_path)
        for rpath in rpaths:
            possible_path = os.path.join(rpath, rpath_lib)
            if os.path.exists(possible_path): return os.path.realpath(possible_path)
        return None
    except Exception as e:
        logging.error(f"Error resolving @rpath for '{rpath_lib}': {e}")
        return None

def update_library_paths(binary_path, dependencies, app_bundle_path):
    """
    Uses install_name_tool to change a binary's dependency references
    to be relative to the new .app bundle structure.
    """
    for original_path in dependencies:
        lib_name = os.path.basename(original_path)
        actual_path = resolve_library_path(original_path, binary_path)
        
        # If a dependency can't be found, log it and skip to the next one.
        # This prevents the script from crashing with a TypeError (But the error can be examined in the log file)
        if not actual_path:
            logging.error(f"Cannot update path for '{original_path}' in '{os.path.basename(binary_path)}' because it could not be found. Skipping.")
            continue
        
        # Determine the new, portable path inside the bundle
        if ".framework" in original_path:
            framework_parts = original_path.split(".framework/")
            framework_name = os.path.basename(framework_parts[0] + ".framework")
            new_path = f"@executable_path/../Frameworks/{framework_name}/{framework_parts[1] if len(framework_parts) > 1 else framework_name}"
        else:
            file_type = subprocess.run(["file", "-b", actual_path], capture_output=True, text=True).stdout.lower()
            if "executable" in file_type:
                new_path = f"@executable_path/{lib_name}"
            else:
                new_path = f"@executable_path/../Frameworks/{lib_name}"
        
        logging.info(f"In {os.path.basename(binary_path)}: changing '{original_path}' -> '{new_path}'")
        try:
            subprocess.run(["install_name_tool", "-change", original_path, new_path, binary_path], check=True)
        except subprocess.CalledProcessError as e:
            logging.error(f"Error updating reference: {e}")

def update_library_id(lib_path):
    """Uses install_name_tool to update a library's own ID to be self-contained."""
    if not os.path.exists(lib_path): return
    new_id = ""
    if "Contents/Frameworks" in lib_path:
        lib_name = os.path.basename(lib_path)
        if ".framework" in lib_path:
            framework_parts = lib_path.split("Frameworks/")[1].split(".framework/")
            framework_name = framework_parts[0]
            new_id = f"@executable_path/../Frameworks/{framework_name}.framework/{framework_parts[1] if len(framework_parts) > 1 else framework_name}"
        else:
            new_id = f"@executable_path/../Frameworks/{lib_name}"
    elif "Contents/MacOS" in lib_path:
        new_id = f"@executable_path/{os.path.basename(lib_path)}"
    else:
        # Don't try to change the ID of files not in the bundle
        return
    
    logging.info(f"Updating ID of {os.path.basename(lib_path)} to {new_id}")
    try:
        subprocess.run(["install_name_tool", "-id", new_id, lib_path], check=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"Error updating library ID: {e}")

def process_and_bundle_dependencies(src_path, app_bundle_path, processed_set, dest_basename=None):
    """
    Recursively finds all dependencies for a file, copies them into the
    bundle, rewires their paths, and then processes their own dependencies.
    """
    real_src_path = os.path.realpath(src_path)
    if real_src_path in processed_set: return

    # Use the desired destination name if provided, otherwise use the real file's name.
    # This ensures symlinked libraries are renamed correctly in the bundle.
    lib_name = dest_basename if dest_basename else os.path.basename(real_src_path)
    logging.info(f"--- Analyzing: {os.path.basename(real_src_path)} (as {lib_name}) ---")

    # Determine if it's an executable or a library and set destination
    if ".dylib" in lib_name or ".so" in lib_name:
        dest_dir = os.path.join(app_bundle_path, "Contents", "Frameworks")
    else:
        dest_dir = os.path.join(app_bundle_path, "Contents", "MacOS")

    os.makedirs(dest_dir, exist_ok=True)
    dest_path = os.path.join(dest_dir, lib_name)

    logging.info(f"Relocating '{os.path.basename(real_src_path)}' to '{os.path.relpath(dest_dir, app_bundle_path)}' as '{lib_name}'")
    shutil.copy2(real_src_path, dest_path)
    os.chmod(dest_path, 0o755)
    processed_set.add(real_src_path)

    # After copying, get its dependencies and rewire paths and ID
    dependencies = get_dependencies(dest_path)
    update_library_paths(dest_path, dependencies, app_bundle_path)
    update_library_id(dest_path)

    # Recurse for all found dependencies
    logging.info(f"Found {len(dependencies)} dependencies for {lib_name} to process.")
    for lib_path in dependencies:
        actual_path = resolve_library_path(lib_path, src_path) # Use original src_path for context
        if actual_path:
            # Pass the symlink's name as the intended destination filename for the next recursion
            process_and_bundle_dependencies(actual_path, app_bundle_path, processed_set, dest_basename=os.path.basename(lib_path))
        else:
            # This error is critical, as it means the final bundle will be broken
            logging.error(f"COULD NOT FIND dependency '{lib_path}' required by '{lib_name}'.")
   
def bundle_data_resources(app_bundle_path, args):
    """Copies shared data resources (icons, themes, etc.) into the bundle."""
    logging.info("--- Bundling App Resources (share directory) ---")

    # This path assumes the 'share' directory is in the build directory, one level up from bin_dir
    source_share_dir = os.path.join(os.path.dirname(args.bin_dir), "share")
    dest_share_dir = os.path.join(app_bundle_path, "Contents", "share")

    if os.path.isdir(source_share_dir):
        logging.info(f"Copying resources from '{source_share_dir}' to '{os.path.relpath(dest_share_dir, app_bundle_path)}'")
        # dirs_exist_ok=True is useful to avoid errors if the dir already exists
        shutil.copytree(source_share_dir, dest_share_dir, dirs_exist_ok=True)
    else:
        logging.warning(f"RESOURCE DIRECTORY NOT FOUND at '{source_share_dir}'. UI may be broken.")


def bundle_synfig_modules(app_bundle_path, args):
    """Finds, bundles, and fixes Synfig's functional modules."""
    logging.info("--- Processing Synfig Modules ---")

    homebrew_prefix = get_homebrew_prefix()
    build_modules_path = os.path.normpath(os.path.join(args.bin_dir, "..", "lib", "synfig", "modules"))

    module_search_paths = [
        build_modules_path,
        os.path.join(homebrew_prefix, "lib/synfig/modules") if homebrew_prefix else None,
        "/usr/local/lib/synfig/modules",
    ]

    synfig_modules_dir = None
    for path in filter(None, module_search_paths):
        if os.path.isdir(path):
            synfig_modules_dir = path
            logging.info(f"Found Synfig modules at: {synfig_modules_dir}")
            break

    if not synfig_modules_dir:
        logging.warning("Could not find Synfig modules directory. Plugin functionality may be limited.")
        return

    bundle_modules_dest = os.path.join(app_bundle_path, "Contents", "Resources", "synfig", "modules")
    logging.info(f"Copying modules to: {os.path.relpath(bundle_modules_dest, app_bundle_path)}")
    shutil.copytree(synfig_modules_dir, bundle_modules_dest, dirs_exist_ok=True)

    logging.info("Fixing dependencies for all copied modules...")
    for module_file in glob.glob(os.path.join(bundle_modules_dest, "*")):
        if not os.path.isfile(module_file) or ".DS_Store" in module_file: continue

        logging.info(f"Processing module: {os.path.basename(module_file)}")
        module_deps = get_dependencies(module_file)
        update_library_paths(module_file, module_deps, app_bundle_path)
        update_library_id(module_file) 


def generate_gtk_caches(app_bundle_path):
    """Generates the necessary cache files for bundled GTK resources to work correctly."""
    logging.info("--- Generating GTK Resource Caches ---")
    homebrew_prefix = get_homebrew_prefix()

    # Update the Icon Caches (Hicolor and Adwaita)
    gtk_icon_cache_tool = find_system_executable("gtk-update-icon-cache")
    if gtk_icon_cache_tool:
        icon_themes = ["hicolor", "Adwaita"]
        for theme in icon_themes:
            theme_dir = os.path.join(app_bundle_path, "Contents", "share", "icons", theme)
            if os.path.isdir(theme_dir):
                try:
                    logging.info(f"Updating icon cache in '{theme_dir}'...")
                    subprocess.run([gtk_icon_cache_tool, "--force", "--quiet", theme_dir], check=True)
                    logging.info(f"Successfully updated {theme} icon cache.")
                except subprocess.CalledProcessError as e:
                    logging.error(f"Failed to update {theme} icon cache: {e}")
            else:
                logging.warning(f"Icon theme '{theme}' not found in bundle. Skipping cache update.")
    else:
        logging.warning("'gtk-update-icon-cache' not found. Icons may fail to load.")

    # Compile GSettings Schemas
    glib_schema_tool = find_system_executable("glib-compile-schemas")
    if glib_schema_tool:
        schemas_dir = os.path.join(app_bundle_path, "Contents", "share", "glib-2.0", "schemas")
        if os.path.isdir(schemas_dir):
            try:
                logging.info(f"Compiling GSettings schemas in '{schemas_dir}'...")
                subprocess.run([glib_schema_tool, schemas_dir], check=True)
                logging.info("Successfully compiled GSettings schemas.")
            except subprocess.CalledProcessError as e:
                logging.error(f"Failed to compile GSettings schemas: {e}")
        else:
            logging.warning(f"GSettings schemas directory not found at '{schemas_dir}'.")
    else:
        logging.warning("'glib-compile-schemas' not found. App settings may not work correctly.")


def create_launcher_script(app_bundle_path, executable_name):
    """Creates a shell script to set environment variables and launch the real executable."""
    logging.info(f"Creating launcher script for '{executable_name}'...")

    macos_dir = os.path.join(app_bundle_path, "Contents", "MacOS")
    original_executable_path = os.path.join(macos_dir, executable_name)
    real_executable_path = os.path.join(macos_dir, f"{executable_name}_real")

    if not os.path.exists(original_executable_path):
        logging.error(f"Cannot create launcher script. Original executable not found at '{original_executable_path}'.")
        return

    # Only create the launcher if it hasn't been done already
    if not os.path.exists(real_executable_path):
        os.rename(original_executable_path, real_executable_path)
        logging.info(f"Renamed original executable to '{os.path.basename(real_executable_path)}'")

        launcher_script_content = f"""#!/bin/bash
# This script sets up the complete environment for a bundled GTK application.

DIR=$(cd "$(dirname "$0")" && pwd)

# Prioritize our bundled libraries to avoid system conflicts.
export DYLD_LIBRARY_PATH="$DIR/../Frameworks"

# Point the module loader to our bundled Synfig modules.
export LTDL_LIBRARY_PATH="$DIR/../Resources/synfig/modules"

# Point GTK to our bundled data files (icons, themes, etc.).
export XDG_DATA_DIRS="$DIR/../share"

# Execute the real binary, passing along all arguments.
exec "$DIR/{os.path.basename(real_executable_path)}" "$@"
"""
        with open(original_executable_path, 'w') as f:
            f.write(launcher_script_content)

        os.chmod(original_executable_path, 0o755)
        logging.info(f"Created new executable launcher script at '{os.path.basename(original_executable_path)}'")


def main():
    setup_logging()
    parser = argparse.ArgumentParser(description="Creates and populates a macOS app bundle from a skeleton.")
    parser.add_argument("--output-dir", required=True, help="The final directory where the .app bundle will be created.")
    parser.add_argument("--build-dir", required=True, help="The top-level CMake build directory (which contains the configured Info.plist).")
    parser.add_argument("--bin-dir", required=True, help="The directory inside the build folder that contains the compiled executables.")
    parser.add_argument("--skeleton-dir", required=True, help="Path to the skeleton app directory.")
    parser.add_argument("--binaries", required=True, help="Semicolon-separated list of primary binaries to bundle.")
    parser.add_argument("--log-file", help="Optional. Path to save the detailed log file.")

    parser.add_argument(
        "--extra-binaries",
        help="Semicolon-separated list of third-party binaries to find and bundle (e.g., 'ffmpeg;magick')."
    )
    args = parser.parse_args()

    setup_logging(args.log_file)

    app_name = "SynfigStudio.app"
    app_bundle_path = os.path.join(args.output_dir, app_name)

    logging.info(f"Creating bundle '{app_name}' from skeleton '{args.skeleton_dir}'")
    if os.path.exists(app_bundle_path):
        logging.info("Removing existing app bundle.")
        shutil.rmtree(app_bundle_path)

    ignore_list = shutil.ignore_patterns('Info.plist.in', '.DS_Store')
    # Copy the skeleton, excluding the ignored files
    shutil.copytree(args.skeleton_dir, app_bundle_path, ignore=ignore_list)
    
    # Use the argument for the path, not a hardcoded one ---
    configured_plist = os.path.join(args.build_dir, "Info.plist")
    if os.path.exists(configured_plist):
        shutil.copy2(configured_plist, os.path.join(app_bundle_path, "Contents", "Info.plist"))
    else:
        logging.error(f"Configured Info.plist not found at {configured_plist}"); return

    processed_files = set()
    primary_binaries = args.binaries.split(';')
    logging.info(f"Relocating primary binaries: {', '.join(primary_binaries)}")
    for binary_name in primary_binaries:
        source_path = os.path.join(args.bin_dir, binary_name)
        if os.path.exists(source_path):
            process_and_bundle_dependencies(source_path, app_bundle_path, processed_files)
        else:
            logging.error(f"PRIMARY BINARY NOT FOUND at '{source_path}'. Please check the --bin-dir path.")

    if args.extra_binaries:
        extra_binaries_list = args.extra_binaries.split(';')
        logging.info(f"Processing extra third-party binaries: {', '.join(extra_binaries_list)}")

        for binary_name in extra_binaries_list:
            source_path = find_system_executable(binary_name)

            if source_path:
                process_and_bundle_dependencies(source_path, app_bundle_path, processed_files)
            else:
                logging.warning(f"SKIPPING: Could not find extra binary '{binary_name}' on the system.")

    
    # Bundle application-specific resources and plugins
    bundle_data_resources(app_bundle_path, args)
    bundle_synfig_modules(app_bundle_path, args)
    
    # Generate necessary caches for GTK
    generate_gtk_caches(app_bundle_path)

    # Create the launcher script
    create_launcher_script(app_bundle_path, "synfigstudio")
    logging.info(f"Successfully created and populated {app_name}")

if __name__ == "__main__":
    main()