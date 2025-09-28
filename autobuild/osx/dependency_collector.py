import argparse
import os
import shutil
import subprocess
import logging
import re
import glob
import sys
import tarfile
import urllib.request
import platform

# Import code signing functions
try:
    from . import code_signing
except ImportError:
    # Handle case when running as script directly
    import code_signing

try:
	import lxml
	LXML_AVAILABLE = True
except ImportError:
	LXML_AVAILABLE = False

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
    Prioritizes build directory paths over system paths when resolving @rpath references.
    """
    try:
        # 1. Handle @rpath by using the rpaths from the referring binary
        if lib_path.startswith("@rpath") and binary_path:
            rpath_lib = lib_path.split("@rpath/", 1)[1]
            resolved = resolve_rpath(binary_path, rpath_lib)
            if resolved: 
                logging.info(f"Resolved @rpath reference '{lib_path}' to '{resolved}'")
                return resolved

        # 2. Handle direct paths that already exist
        if os.path.exists(lib_path):
            return os.path.realpath(lib_path)

        # 3. Handle frameworks specially
        if ".framework" in lib_path:
            # Extract framework name and internal path
            if ".framework/Versions/A/" in lib_path:
                parts = lib_path.split(".framework/Versions/A/")
                framework_base = os.path.basename(parts[0]) + ".framework"
                internal_path = "Versions/A/" + parts[1] if len(parts) > 1 else "Versions/A/" + framework_base.replace(".framework", "")
            else:
                parts = lib_path.split(".framework/")
                framework_base = os.path.basename(parts[0]) + ".framework"
                internal_path = parts[1] if len(parts) > 1 else framework_base.replace(".framework", "")
            
            # Search for frameworks
            framework_search_paths = [
                "/opt/homebrew/lib",
                "/opt/homebrew/opt/qt/lib",
                "/usr/local/lib", 
                "/Library/Frameworks",
                "/System/Library/Frameworks"
            ]
            
            homebrew_prefix = get_homebrew_prefix()
            if homebrew_prefix:
                framework_search_paths.extend([
                    os.path.join(homebrew_prefix, "opt", "*", "lib"),
                    os.path.join(homebrew_prefix, "lib"),
                ])
            
            for search_path in framework_search_paths:
                if '*' in search_path:
                    for expanded_path in glob.glob(search_path):
                        candidate = os.path.join(expanded_path, framework_base, internal_path)
                        if os.path.exists(candidate):
                            resolved_path = os.path.realpath(candidate)
                            logging.info(f"Found framework '{framework_base}' at '{resolved_path}'")
                            return resolved_path
                else:
                    candidate = os.path.join(search_path, framework_base, internal_path)
                    if os.path.exists(candidate):
                        resolved_path = os.path.realpath(candidate)
                        logging.info(f"Found framework '{framework_base}' at '{resolved_path}'")
                        return resolved_path

        # 4. Search in standard and Homebrew locations for regular libraries
        lib_name_base = os.path.basename(lib_path).split('.dylib', 1)[0] # Gets base name without extension
        
        # Build directory paths should be prioritized over system paths
        build_search_paths = []
        if binary_path:
            # Add build directory paths first
            build_search_paths.append(os.path.join(os.path.dirname(binary_path), "..", "lib"))
            # Also try to extract rpaths and add them to build search paths
            try:
                output = subprocess.check_output(["otool", "-l", binary_path], text=True)
                rpaths = _extract_rpaths_from_otool(output, binary_path)
                build_search_paths.extend(rpaths)
            except subprocess.CalledProcessError:
                pass
        
        # System paths (lower priority)
        system_search_paths = [
            "/opt/homebrew/lib",  # Core Homebrew libraries
            "/opt/homebrew/opt/gcc/lib/gcc/current",  # GCC runtime libraries
            "/usr/local/opt/*/lib",  # Intel Homebrew formula libraries
            "/usr/local/opt/sqlite/lib", # Intel Homebrew SQLite
            "/opt/homebrew/opt/sqlite/lib", # ARM Homebrew SQLite
            "/usr/local/lib", # legacy homebrew installation directory
            "/opt/local/lib", # MacPorts installation directory
            "/usr/lib", # System libraries
            "/Library/Frameworks", # System-wide frameworks
        ]
        
        # Add specific Homebrew package lib directories
        homebrew_prefix = get_homebrew_prefix()
        if homebrew_prefix:
            system_search_paths.extend([
                os.path.join(homebrew_prefix, "opt", "*", "lib"),
                os.path.join(homebrew_prefix, "lib"),
            ])
        
        # Combine search paths with build paths first
        search_paths = build_search_paths + system_search_paths

        # Regex pattern for versioned libraries
        version_pattern = re.compile(rf'^{re.escape(lib_name_base)}(\.\d+)*\.dylib$')

        for path in filter(None, search_paths):
            # Handle glob patterns in paths
            if '*' in path:
                for expanded_path in glob.glob(path):
                    candidate = os.path.join(expanded_path, lib_name_base + ".dylib")
                    if os.path.exists(candidate):
                        resolved_path = os.path.realpath(candidate)
                        logging.info(f"Found library '{lib_name_base}' at '{resolved_path}'")
                        return resolved_path
                    
                    # Check for versioned matches in expanded paths
                    if os.path.exists(expanded_path):
                        try:
                            for f in os.listdir(expanded_path):
                                if version_pattern.match(f):
                                    resolved_path = os.path.realpath(os.path.join(expanded_path, f))
                                    logging.info(f"Found versioned library '{f}' at '{resolved_path}'")
                                    return resolved_path
                        except (OSError, PermissionError):
                            continue
            else:
                # Check for exact match first
                candidate = os.path.join(path, lib_name_base + ".dylib")
                if os.path.exists(candidate):
                    resolved_path = os.path.realpath(candidate)
                    logging.info(f"Found library '{lib_name_base}' at '{resolved_path}'")
                    return resolved_path

                # Check for versioned matches
                if os.path.exists(path):
                    try:
                        for f in os.listdir(path):
                            if version_pattern.match(f):
                                resolved_path = os.path.realpath(os.path.join(path, f))
                                logging.info(f"Found versioned library '{f}' at '{resolved_path}'")
                                return resolved_path
                    except (OSError, PermissionError):
                        continue

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
                        # @executable_path should be replaced with the directory containing the executable
                        # For a binary in build/output/Debug/bin/, @executable_path should be build/output/Debug/bin/
                        expanded = os.path.normpath(rpath.replace("@executable_path", binary_dir))
                    rpaths.append(expanded)
                    break
    return rpaths

def resolve_rpath(binary_path, rpath_lib):
    """Finds a library using the rpaths embedded in the binary that needs it."""
    try:
        output = subprocess.check_output(["otool", "-l", binary_path], text=True)
        rpaths = _extract_rpaths_from_otool(output, binary_path)
        logging.info(f"Resolving @rpath for '{rpath_lib}' using rpaths: {rpaths}")
        
        # First, try the rpaths from the binary
        for rpath in rpaths:
            possible_path = os.path.join(rpath, rpath_lib)
            if os.path.exists(possible_path): 
                resolved_path = os.path.realpath(possible_path)
                logging.info(f"Found @rpath library '{rpath_lib}' at '{resolved_path}'")
                return resolved_path
        
        # If not found in rpaths, try build directory paths as fallback
        build_search_paths = []
        if binary_path:
            # Add build directory paths
            build_search_paths.append(os.path.join(os.path.dirname(binary_path), "..", "lib"))
            # Also try to extract rpaths and add them to build search paths
            try:
                rpaths_from_binary = _extract_rpaths_from_otool(output, binary_path)
                build_search_paths.extend(rpaths_from_binary)
            except:
                pass
        
        for build_path in build_search_paths:
            possible_path = os.path.join(build_path, rpath_lib)
            if os.path.exists(possible_path):
                resolved_path = os.path.realpath(possible_path)
                logging.info(f"Found @rpath library '{rpath_lib}' in build directory at '{resolved_path}'")
                return resolved_path
        
        logging.warning(f"Could not find @rpath library '{rpath_lib}' in any rpath: {rpaths}")
        return None
    except Exception as e:
        logging.error(f"Error resolving @rpath for '{rpath_lib}': {e}")
        return None

def update_library_paths(binary_path, dependencies, app_bundle_path, original_source_path=None):
    """
    Uses install_name_tool to change a binary's dependency references
    to be relative to the new .app bundle structure.
    """
    for original_path in dependencies:
        lib_name = os.path.basename(original_path)
        # Use original source path for rpath resolution if available, otherwise use binary_path
        context_path = original_source_path if original_source_path else binary_path
        actual_path = resolve_library_path(original_path, context_path)
        
        # If a dependency can't be found, log it and skip to the next one.
        # This prevents the script from crashing with a TypeError (But the error can be examined in the log file)
        if not actual_path:
            logging.error(f"Cannot update path for '{original_path}' in '{os.path.basename(binary_path)}' because it could not be found. Skipping.")
            continue
        
        # Determine the new, portable path inside the bundle
        frameworks_dir = os.path.join(app_bundle_path, "Contents", "Frameworks")
        binary_dir = os.path.dirname(binary_path)
        relative_path_to_frameworks = os.path.relpath(frameworks_dir, start=binary_dir)

        if ".framework" in original_path:
            framework_parts = original_path.split(".framework/")
            framework_name = os.path.basename(framework_parts[0] + ".framework")
            new_path = f"@loader_path/{relative_path_to_frameworks}/{framework_name}/{framework_parts[1] if len(framework_parts) > 1 else framework_name}"
        else:
            new_path = f"@loader_path/{relative_path_to_frameworks}/{lib_name}"
        
        # Cleanup the path for the case where the relative path is '.'
        new_path = new_path.replace("/./", "/")

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

    # Determine destination based on file type
    if ".framework" in real_src_path:
        # Handle frameworks specially - copy entire framework structure
        if ".framework/Versions/A/" in real_src_path:
            # Extract framework base directory
            framework_part = real_src_path.split(".framework/Versions/A/")[0] + ".framework"
        else:
            framework_part = real_src_path.split(".framework/")[0] + ".framework"
        
        framework_name = os.path.basename(framework_part)
        dest_dir = os.path.join(app_bundle_path, "Contents", "Frameworks")
        os.makedirs(dest_dir, exist_ok=True)
        dest_framework_path = os.path.join(dest_dir, framework_name)
        dest_path = os.path.join(dest_framework_path, os.path.relpath(real_src_path, framework_part))
        
        # Copy entire framework if not already copied
        if not os.path.exists(dest_framework_path):
            logging.info(f"Copying framework '{framework_name}' to '{os.path.relpath(dest_dir, app_bundle_path)}'")
            shutil.copytree(framework_part, dest_framework_path, symlinks=True)
            # Set executable permissions on the main framework binary
            os.chmod(dest_path, 0o755)
        else:
            logging.info(f"Framework '{framework_name}' already exists in bundle")
        
        processed_set.add(real_src_path)
    elif ".dylib" in lib_name or ".so" in lib_name:
        dest_dir = os.path.join(app_bundle_path, "Contents", "Frameworks")
        os.makedirs(dest_dir, exist_ok=True)
        dest_path = os.path.join(dest_dir, lib_name)
        
        logging.info(f"Relocating '{os.path.basename(real_src_path)}' to '{os.path.relpath(dest_dir, app_bundle_path)}' as '{lib_name}'")
        shutil.copy2(real_src_path, dest_path)
        os.chmod(dest_path, 0o755)
        processed_set.add(real_src_path)
    else:
        dest_dir = os.path.join(app_bundle_path, "Contents", "MacOS")
        os.makedirs(dest_dir, exist_ok=True)
        dest_path = os.path.join(dest_dir, lib_name)
        
        logging.info(f"Relocating '{os.path.basename(real_src_path)}' to '{os.path.relpath(dest_dir, app_bundle_path)}' as '{lib_name}'")
        shutil.copy2(real_src_path, dest_path)
        os.chmod(dest_path, 0o755)
        processed_set.add(real_src_path)

    # First, get dependencies and rewire all paths (skip for entire frameworks)
    if not ".framework" in real_src_path or ".framework/Versions/A/" in real_src_path:
        dependencies = get_dependencies(real_src_path)
        update_library_paths(dest_path, dependencies, app_bundle_path, original_source_path=real_src_path)
        update_library_id(dest_path)

        # Now, with all modifications complete, apply the ad-hoc signature.
        code_signing.sign_file_adhoc(dest_path)

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
    else:
        logging.info(f"Skipping dependency processing for framework directory (not main binary): {lib_name}")
   
def bundle_support_executable(executable_name, app_bundle_path, processed_set):
    """
    Finds a required support executable, bundles it into Resources/bin,
    and processes its dependencies.
    """
    logging.info(f"--- Bundling support executable: {executable_name} ---")
    source_path = find_system_executable(executable_name)
    if not source_path:
        logging.error(f"Could not find required support executable '{executable_name}'. Image loading will likely fail.")
        return

    dest_dir = os.path.join(app_bundle_path, "Contents", "Resources", "bin")
    os.makedirs(dest_dir, exist_ok=True)
    dest_path = os.path.join(dest_dir, executable_name)

    if os.path.realpath(source_path) in processed_set:
        return

    logging.info(f"Copying '{executable_name}' to '{os.path.relpath(dest_dir, app_bundle_path)}'")
    shutil.copy2(source_path, dest_path)
    os.chmod(dest_path, 0o755)
    processed_set.add(os.path.realpath(source_path))

    # First, fix the library paths for the support executable's dependencies.
    dependencies = get_dependencies(source_path)
    update_library_paths(dest_path, dependencies, app_bundle_path, original_source_path=source_path)

    # Now, with all modifications complete, sign the support executable.
    # Apply ad-hoc signature to the executable after all modifications
    code_signing.sign_file_adhoc(dest_path)

    # Recursively bundle the dependencies of the support tool.
    for lib_path in dependencies:
        actual_path = resolve_library_path(lib_path, source_path)
        if actual_path:
            process_and_bundle_dependencies(actual_path, app_bundle_path, processed_set, dest_basename=os.path.basename(lib_path))
        else:
            logging.error(f"COULD NOT FIND dependency '{lib_path}' required by '{executable_name}'.")

def bundle_data_resources(app_bundle_path, args, processed_set):
    """Copies shared data resources (icons, themes, etc.) into the bundle."""
    logging.info("--- Bundling App Resources (share directory) ---")

    # This path assumes the 'share' directory is in the build directory, one level up from bin_dir
    source_share_dir = os.path.join(os.path.dirname(args.bin_dir), "share")
    dest_share_dir = os.path.join(app_bundle_path, "Contents", "Resources", "share")

    if os.path.isdir(source_share_dir):
        logging.info(f"Copying resources from '{source_share_dir}' to '{os.path.relpath(dest_share_dir, app_bundle_path)}'")
        # dirs_exist_ok=True is useful to avoid errors if the dir already exists
        shutil.copytree(source_share_dir, dest_share_dir, dirs_exist_ok=True)
    else:
        logging.warning(f"RESOURCE DIRECTORY NOT FOUND at '{source_share_dir}'. UI may be broken.")

    # Also copy GTK resources from system
    homebrew_prefix = get_homebrew_prefix()
    if homebrew_prefix:
        # --- Package-specific resources ---
        pkg_specific_resources = [
            ("glib", "glib-2.0"),
            ("gtk+3", "gtk-3.0"),
            ("gdk-pixbuf", "gdk-pixbuf-2.0"),
            ("pango", "pango"),
        ]
        
        for pkg_name, resource_folder in pkg_specific_resources:
            try:
                pkg_prefix = subprocess.check_output(["brew", "--prefix", pkg_name], text=True, stderr=subprocess.DEVNULL).strip()
                # The resource folder is directly inside the package's share directory.
                src_path = os.path.join(pkg_prefix, "share", resource_folder)
                dest_path = os.path.join(dest_share_dir, resource_folder)
                
                if os.path.isdir(src_path):
                    logging.info(f"Copying GTK resource '{resource_folder}' from '{src_path}'")
                    if os.path.exists(dest_path):
                        shutil.rmtree(dest_path)
                    shutil.copytree(src_path, dest_path)
                else:
                    # Fallback for packages where the content is the share folder itself
                    src_path_alt = os.path.join(pkg_prefix, "share")
                    if os.path.isdir(src_path_alt):
                        logging.info(f"Copying GTK resource content from '{src_path_alt}' for package '{pkg_name}'")
                        shutil.copytree(src_path_alt, dest_path, dirs_exist_ok=True)
                    else:
                        logging.warning(f"GTK resource '{resource_folder}' not found at '{src_path}' or in '{src_path_alt}' for package '{pkg_name}'")

            except (subprocess.CalledProcessError, FileNotFoundError):
                 logging.warning(f"Could not find brew package '{pkg_name}'. Skipping its resources.")

        # --- GDK Pixbuf Loaders ---
        try:
            pkg_prefix = subprocess.check_output(["brew", "--prefix", "gdk-pixbuf"], text=True, stderr=subprocess.DEVNULL).strip()
            # The loaders are in the lib directory
            src_path = os.path.join(pkg_prefix, "lib", "gdk-pixbuf-2.0")
            dest_path = os.path.join(app_bundle_path, "Contents", "Resources", "lib", "gdk-pixbuf-2.0")
            
            if os.path.isdir(src_path):
                logging.info(f"Copying GDK Pixbuf loaders from '{src_path}'")
                if os.path.exists(dest_path):
                    shutil.rmtree(dest_path)
                shutil.copytree(src_path, dest_path)
                
                # The GDK Pixbuf loader plugins are libraries themselves and have dependencies that need bundling.
                logging.info("Fixing dependencies for all copied GDK Pixbuf loader plugins...")
                for root, _, files in os.walk(dest_path):
                    for file in files:
                        if file.endswith(".so"):
                            plugin_path = os.path.join(root, file)
                            logging.info(f"Processing GDK Pixbuf loader plugin: {file}")
                            original_plugin_path = os.path.join(src_path, os.path.relpath(plugin_path, dest_path))
                            plugin_deps = get_dependencies(original_plugin_path)
                            update_library_paths(plugin_path, plugin_deps, app_bundle_path, original_source_path=original_plugin_path)
                            update_library_id(plugin_path)
                            # Re-sign the plugin after modification
                            # Apply ad-hoc signature to the plugin
                            code_signing.sign_file_adhoc(plugin_path)
                            
                            # Now recursively bundle all dependencies of this plugin
                            for dep_path in plugin_deps:
                                actual_path = resolve_library_path(dep_path, original_plugin_path)
                                if actual_path:
                                    process_and_bundle_dependencies(actual_path, app_bundle_path, processed_set, dest_basename=os.path.basename(dep_path))
                                else:
                                    logging.error(f"COULD NOT FIND dependency '{dep_path}' required by GDK Pixbuf plugin '{file}'.")
                                    
            else:
                logging.warning(f"GDK Pixbuf loaders not found at '{src_path}'")
        except (subprocess.CalledProcessError, FileNotFoundError):
                logging.warning(f"Could not find brew package 'gdk-pixbuf'. Image loaders will not be bundled.")

        # --- Fontconfig Configuration ---
        try:
            pkg_prefix = subprocess.check_output(["brew", "--prefix", "fontconfig"], text=True, stderr=subprocess.DEVNULL).strip()
            src_path = os.path.join(pkg_prefix, "share", "fontconfig")
            dest_path = os.path.join(app_bundle_path, "Contents", "Resources", "etc", "fonts")

            if os.path.isdir(src_path):
                logging.info(f"Copying Fontconfig data from '{src_path}'")
                if os.path.exists(dest_path):
                    shutil.rmtree(dest_path)
                shutil.copytree(src_path, dest_path)
            else:
                logging.warning(f"Fontconfig data not found at '{src_path}'")
        except (subprocess.CalledProcessError, FileNotFoundError):
            logging.warning("Could not find brew package 'fontconfig'. Fonts may not work correctly.")
            
        # --- MLT Plugins ---
        try:
            pkg_prefix = subprocess.check_output(["brew", "--prefix", "mlt"], text=True, stderr=subprocess.DEVNULL).strip()
            src_path = os.path.join(pkg_prefix, "lib", "mlt")
            dest_path = os.path.join(app_bundle_path, "Contents", "Resources", "lib", "mlt")

            if os.path.isdir(src_path):
                logging.info(f"Copying MLT plugins from '{src_path}'")
                if os.path.exists(dest_path):
                    shutil.rmtree(dest_path)
                shutil.copytree(src_path, dest_path)
                
                # The MLT plugins are libraries themselves and may have dependencies that need fixing.
                logging.info("Fixing dependencies for all copied MLT plugins...")
                for root, _, files in os.walk(dest_path):
                    for file in files:
                        if file.endswith(".so"):
                            module_path = os.path.join(root, file)
                            logging.info(f"Processing MLT plugin: {file}")
                            original_module_path = os.path.join(src_path, os.path.relpath(module_path, dest_path))
                            module_deps = get_dependencies(original_module_path)
                            update_library_paths(module_path, module_deps, app_bundle_path, original_source_path=original_module_path)
                            update_library_id(module_path)
                            # Re-sign the module after modification
                            # Apply ad-hoc signature to the plugin
                            code_signing.sign_file_adhoc(module_path)
                            
                            # Now recursively bundle all dependencies of this MLT plugin
                            for dep_path in module_deps:
                                actual_path = resolve_library_path(dep_path, original_module_path)
                                if actual_path:
                                    process_and_bundle_dependencies(actual_path, app_bundle_path, processed_set, dest_basename=os.path.basename(dep_path))
                                else:
                                    logging.error(f"COULD NOT FIND dependency '{dep_path}' required by MLT plugin '{file}'.")

            else:
                logging.warning(f"MLT plugins not found at '{src_path}'")
        except (subprocess.CalledProcessError, FileNotFoundError):
            logging.warning("Could not find brew package 'mlt'. Some media functionality may not work.")

        # --- Set Writable Permissions ---
        # Ensure all bundled resources are writable so that their extended attributes can be cleared.
        logging.info("Ensuring all bundled resources are user-writable...")
        for dir_name in ["lib", "share", "etc"]:
            path = os.path.join(app_bundle_path, "Contents", "Resources", dir_name)
            if os.path.isdir(path):
                try:
                    subprocess.run(["chmod", "-R", "u+w", path], check=True)
                except (subprocess.CalledProcessError, FileNotFoundError):
                    logging.warning(f"Could not make the contents of '{dir_name}' writable. Clearing attributes may fail.")

        # --- Shared resources (like icon themes) ---
        icon_themes = ["hicolor", "Adwaita"]
        for theme in icon_themes:
            src_path = os.path.join(homebrew_prefix, "share", "icons", theme)
            dest_path = os.path.join(dest_share_dir, "icons", theme)
            if os.path.isdir(src_path):
                logging.info(f"Copying icon theme '{theme}' from '{src_path}'")
                if os.path.exists(dest_path):
                    shutil.rmtree(dest_path)
                shutil.copytree(src_path, dest_path)
            else:
                logging.warning(f"Icon theme '{theme}' not found at '{src_path}'.")


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
        # Get the original source path for the module
        module_name = os.path.basename(module_file)
        original_module_path = os.path.join(synfig_modules_dir, module_name)
        module_deps = get_dependencies(original_module_path)
        update_library_paths(module_file, module_deps, app_bundle_path, original_source_path=original_module_path)
        update_library_id(module_file) 


def bundle_python_framework(app_bundle_path, build_dir):
    """
    Downloads, extracts, and bundles a portable Python environment,
    then uses its pip to install lxml into it.
    """
    logging.info("--- Bundling Portable Python Framework ---")

    # Configuration for the portable Python
    machine_arch = platform.machine()
    if machine_arch == "arm64":
        python_arch_str = "aarch64"
    elif machine_arch == "x86_64":
        python_arch_str = "x86_64"
    else:
        logging.error(f"Unsupported macOS architecture: {machine_arch}. Cannot download portable Python.")
        return

    python_url = f"https://github.com/astral-sh/python-build-standalone/releases/download/20250918/cpython-3.12.11+20250918-{python_arch_str}-apple-darwin-install_only.tar.gz"
    download_dir = os.path.join(build_dir, "cache")
    archive_name = os.path.basename(python_url)
    archive_path = os.path.join(download_dir, archive_name)
    extracted_dir_name = "python" # The archive extracts to a 'python' folder
    extracted_path = os.path.join(download_dir, extracted_dir_name)

    os.makedirs(download_dir, exist_ok=True)

    # 1. Download and Extract if not already present
    if not os.path.isdir(extracted_path):
        logging.info(f"Portable Python not found at {extracted_path}.")
        
        # Download
        if not os.path.exists(archive_path):
            logging.info(f"Downloading from {python_url}...")
            try:
                with urllib.request.urlopen(python_url) as response, open(archive_path, 'wb') as out_file:
                    shutil.copyfileobj(response, out_file)
                logging.info(f"Successfully downloaded to {archive_path}")
            except Exception as e:
                logging.error(f"Failed to download Python: {e}")
                return # Cannot proceed
        else:
            logging.info(f"Archive already exists at {archive_path}.")

        # Extract
        logging.info(f"Extracting {archive_name}...")
        try:
            with tarfile.open(archive_path, "r:gz") as tar:
                tar.extractall(path=download_dir)
            logging.info(f"Successfully extracted to {extracted_path}")
        except Exception as e:
            logging.error(f"Failed to extract Python archive: {e}")
            return # Cannot proceed
    else:
        logging.info(f"Found existing portable Python at {extracted_path}.")

    # 2. Use the portable Python's pip to install lxml
    python_executable = os.path.join(extracted_path, "bin", "python3")
    
    # First, ensure pip is installed in the portable environment
    try:
        logging.info("Ensuring pip is installed in the portable Python environment...")
        subprocess.run([python_executable, "-m", "ensurepip", "--upgrade"], check=True, capture_output=True, text=True)
        logging.info("pip installed or upgraded successfully.")
    except subprocess.CalledProcessError as e:
        logging.error("Failed to install pip using ensurepip.")
        logging.error(f"STDOUT: {e.stdout.strip()}")
        logging.error(f"STDERR: {e.stderr.strip()}")

    pip_executable = os.path.join(extracted_path, "bin", "pip3")

    if os.path.exists(pip_executable):
        logging.info("Attempting to install lxml using the portable pip...")
        try:
            # We target the installation to the portable python's own site-packages
            subprocess.run([pip_executable, "install", "lxml"], check=True, capture_output=True, text=True)
            logging.info("Successfully installed lxml into the portable Python environment.")
        except subprocess.CalledProcessError as e:
            logging.error(f"Failed to install lxml using portable pip.")
            logging.error(f"STDOUT: {e.stdout.strip()}")
            logging.error(f"STDERR: {e.stderr.strip()}")
            # Continue anyway, as lxml might not be critical for all users
    else:
        logging.warning(f"Could not find pip at {pip_executable}. Cannot install lxml.")


    # 3. Copy the entire portable Python installation into the app bundle
    dest_python_path = os.path.join(app_bundle_path, "Contents", "Resources", "python")
    logging.info(f"Copying portable Python to: {os.path.relpath(dest_python_path, app_bundle_path)}")
    if os.path.exists(dest_python_path):
        shutil.rmtree(dest_python_path)
    shutil.copytree(extracted_path, dest_python_path, symlinks=True)

    logging.info("Portable Python bundled successfully.")


def generate_gtk_caches(app_bundle_path):
    """Generates the necessary cache files for bundled GTK resources to work correctly."""
    logging.info("--- Generating GTK Resource Caches ---")
    homebrew_prefix = get_homebrew_prefix()

    # Bundle complete icon themes from system
    icon_themes = ["hicolor", "Adwaita"]
    for theme in icon_themes:
        system_theme_dir = os.path.join(homebrew_prefix, "share", "icons", theme)
        bundle_theme_dir = os.path.join(app_bundle_path, "Contents", "Resources", "share", "icons", theme)
        
        if os.path.isdir(system_theme_dir):
            logging.info(f"Bundling {theme} icon theme from '{system_theme_dir}'...")
            try:
                # Remove existing theme directory in bundle
                if os.path.exists(bundle_theme_dir):
                    shutil.rmtree(bundle_theme_dir)
                
                # Copy complete theme from system
                shutil.copytree(system_theme_dir, bundle_theme_dir)
                logging.info(f"Successfully bundled {theme} icon theme.")
            except Exception as e:
                logging.error(f"Failed to bundle {theme} icon theme: {e}")
        else:
            logging.warning(f"Icon theme '{theme}' not found in system at '{system_theme_dir}'.")
    
    # Update the Icon Caches (Hicolor and Adwaita)
    gtk_icon_cache_tool = find_system_executable("gtk-update-icon-cache")
    if gtk_icon_cache_tool:
        icon_themes = ["hicolor", "Adwaita"]
        for theme in icon_themes:
            theme_dir = os.path.join(app_bundle_path, "Contents", "Resources", "share", "icons", theme)
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

    # Bundle GSettings schemas from system
    glib_schema_tool = find_system_executable("glib-compile-schemas")
    if glib_schema_tool:
        system_schemas_dir = os.path.join(homebrew_prefix, "share", "glib-2.0", "schemas") if homebrew_prefix else None
        bundle_schemas_dir = os.path.join(app_bundle_path, "Contents", "Resources", "share", "glib-2.0", "schemas")

        # Define a list of packages that provide essential schemas.
        schema_packages = ["glib", "gsettings-desktop-schemas", "gtk+3"]
        
        # Create the destination directory if it doesn't exist
        os.makedirs(bundle_schemas_dir, exist_ok=True)
        
        logging.info(f"Bundling GSettings schemas into '{bundle_schemas_dir}'...")

        # Find and copy schemas from each specified package
        for package in schema_packages:
            try:
                package_prefix = subprocess.check_output(["brew", "--prefix", package], text=True, stderr=subprocess.DEVNULL).strip()
                source_schemas_dir = os.path.join(package_prefix, "share", "glib-2.0", "schemas")
                
                if os.path.isdir(source_schemas_dir):
                    logging.info(f"Copying schemas from '{package}' package at '{source_schemas_dir}'")
                    shutil.copytree(source_schemas_dir, bundle_schemas_dir, dirs_exist_ok=True)
                else:
                    logging.warning(f"Schema directory for '{package}' not found at '{source_schemas_dir}'")
            except (FileNotFoundError, subprocess.CalledProcessError):
                logging.warning(f"Could not find brew prefix for package '{package}'. Schemas may be incomplete.")
        
        # Now that all schemas are collected, compile them.
        try:
            logging.info(f"Compiling all collected GSettings schemas in '{bundle_schemas_dir}'...")
            subprocess.run([glib_schema_tool, bundle_schemas_dir], check=True, capture_output=True, text=True)
            logging.info("Successfully compiled GSettings schemas.")
        except subprocess.CalledProcessError as e:
            # Log the detailed error from the tool if it fails
            logging.error(f"Failed to compile GSettings schemas.")
            logging.error(f"STDOUT: {e.stdout.strip()}")
            logging.error(f"STDERR: {e.stderr.strip()}")
    else:
        logging.warning("'glib-compile-schemas' not found. App settings may not work correctly.")




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

    # --- Bundle Required Support Binaries ---
    bundle_support_executable("gdk-pixbuf-query-loaders", app_bundle_path, processed_files)
    
    # Bundle application-specific resources and plugins
    bundle_data_resources(app_bundle_path, args, processed_files)
    bundle_synfig_modules(app_bundle_path, args)
    
    # Generate necessary caches for GTK
    generate_gtk_caches(app_bundle_path)

    # --- Bundle the Python Framework ---
    bundle_python_framework(app_bundle_path, args.build_dir)

    logging.info(f"--- Clearing extended attributes before signing ---")
    try:
        subprocess.run(["xattr", "-cr", app_bundle_path], check=True, capture_output=True, text=True)
        logging.info(f"Successfully cleared extended attributes from {app_name}")
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        logging.warning(f"Could not clear extended attributes: {e}. Code signing may fail.")


    logging.info(f"Successfully created and populated {app_name}")

if __name__ == "__main__":
    main()