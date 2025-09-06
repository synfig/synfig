import logging
import os
import subprocess
from pathlib import Path
import sys

# Sets up logging configuration to output to both file and console
def setup_logging():
    """
    Configure logging to write to both a log file and the console.
    Uses INFO level with timestamp, level, and message formatting.
    """
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler("code_signing.log"),  # Log to file
            logging.StreamHandler()                   # Log to console
        ]
    )
    
def is_binary_file(file_path):
    """
    Check if a file is a Mach-O binary or dynamic library by running the 'file' command.
    
    Args:
        file_path (str): Path to the file to check
        
    Returns:
        bool: True if the file is a Mach-O binary, False otherwise
    """
    try:
        # Use the Unix 'file' command to determine file type
        result = subprocess.run(
            ["file", "-b", file_path],  # '-b' flag produces brief output without the filename
            capture_output=True, 
            text=True, 
            check=True
        )
        # Check if the output contains "Mach-O" which indicates a macOS binary
        return "Mach-O" in result.stdout
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to check if {file_path} is a binary: {e}")
        return False


def find_signable_files(app_bundle_path):
    """
    Find all files within an app bundle that need code signing.
    
    This includes:
    - Executable binaries
    - Dynamic libraries (.dylib)
    - Shared objects (.so)
    - Framework bundles (.framework)
    - Nested app bundles (.app)
    
    Args:
        app_bundle_path (str): Path to the .app bundle
        
    Returns:
        list: Paths to all files that need signing
    """
    signable_files = []
    
    # Define file extensions and directories to target
    extensions = {".dylib", ".so", ".framework", ".app", ""}  # Empty string for executables without extensions
    skip_dirs = {"Headers", "Resources", "Python.framework"}  # Skip directories that don't need signing
    
    # Walk through the entire bundle directory tree
    for root, dirs, files in os.walk(app_bundle_path):
        # Filter out directories we want to skip
        dirs[:] = [d for d in dirs if d not in skip_dirs]
        
        for file in files:
            file_path = os.path.join(root, file)
            
            # Check if it's a Mach-O binary
            if is_binary_file(file_path):
                signable_files.append(file_path)
            
            # Special handling for framework bundles - add the framework binary directly
            if file_path.endswith(".framework"):
                # The framework binary typically has the same name as the framework without the extension
                framework_binary = os.path.join(file_path, os.path.splitext(file)[0])
                if os.path.exists(framework_binary):
                    signable_files.append(framework_binary)
    
    return signable_files

def sign_file(file_path, signing_identity, entitlements=None):
    """
    Sign a single file with the specified code signing identity.
    
    Args:
        file_path (str): Path to the file to sign
        signing_identity (str): Code signing identity (e.g., 'Developer ID Application: Name (ID)')
        entitlements (str, optional): Path to entitlements plist file
    """
    cmd = [
        "codesign",
        "--force",         # Replace any existing signature
        "--timestamp",     # Add a secure timestamp for long-term validity
        "--options=runtime",  # Enable hardened runtime (required for notarization)
        "-s", signing_identity  # Specify the signing identity
    ]
    
    # Add entitlements if specified and the file exists
    if entitlements and os.path.exists(entitlements):
        cmd.extend(["--entitlements", entitlements])
    
    # Add the file to sign at the end of the command
    cmd.append(file_path)
    
    try:
        logging.info(f"Signing {file_path}")
        subprocess.run(cmd, check=True)  # check=True raises an exception if the command fails
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to sign {file_path}: {e}")
        raise  # Re-raise the exception to be handled by the caller

def sign_app_bundle(app_bundle_path, signing_identity, entitlements=None):
    """
    Sign an entire macOS application bundle, including all contained binaries.
    
    The signing follows Apple's recommended order:
    1. Sign embedded components first (reverse-order approach)
    2. Sign the main bundle at the end
    
    """
    setup_logging()
    
    # Step 1: Find all binary files that need signing
    signable_files = find_signable_files(app_bundle_path)
    
    # Step 2: Sort files to sign in the correct order.
    # - Sort by path depth (deeper paths first)
    # - Sort by type (.app bundle last)
    signable_files.sort(
        key=lambda x: (os.path.dirname(x).count("/"), x.endswith(".app")),
        reverse=True  # Deepest files first
    )
    # Step 3: Sign each individual file (except for .app bundle)
    for file in signable_files:
        if not file.endswith(".app"):  # .app bundle will be signed separately
            sign_file(file, signing_identity, entitlements)
    
    # Step 4: Sign the main app bundle
    sign_file(app_bundle_path, signing_identity, entitlements)
    
    # Step 5: Verify that everything was signed correctly
    verify_signature(app_bundle_path)

def verify_signature(app_bundle_path):
    """
    Verify the code signatures in the app bundle using Apple's tools.
    
    This performs two checks:
    1. codesign verification - checks the signature validity
    2. spctl assessment - checks if the app meets the requirements to run
    """
    try:
        # Verify code signature details with strict checking
        subprocess.run(
            ["codesign", "-dv", "--strict=all", app_bundle_path],
            check=True
        )
        # Verify the app passes Gatekeeper assessment
        subprocess.run(
            ["spctl", "-a", "-vv", app_bundle_path],
            check=True
        )
        logging.info("Code signing verification passed!")
    except subprocess.CalledProcessError as e:
        logging.error(f"Code signing verification failed: {e}")
        raise  

if __name__ == "__main__":
    # This section executes when the script is run directly (not imported)
    import argparse
    
    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(description="Sign macOS app bundle")
    parser.add_argument("--app", required=True, help="Path to .app bundle")
    parser.add_argument("--identity", required=True, help="Signing identity (e.g., 'Developer ID Application: Name (ID)')")
    parser.add_argument("--entitlements", help="Path to entitlements.plist")
    
    args = parser.parse_args()
    
    # Verify the app bundle exists
    if not os.path.exists(args.app):
        logging.error(f"App bundle not found: {args.app}")
        sys.exit(1)  # Exit with error code
    
    try:
        # Perform the signing process
        sign_app_bundle(args.app, args.identity, args.entitlements)
    except Exception as e:
        # Handle any exceptions that occurred during signing
        logging.error(f"Signing failed: {e}")
        sys.exit(1)  # Exit with error code