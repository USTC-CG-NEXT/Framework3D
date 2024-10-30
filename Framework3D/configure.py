import zipfile
import shutil
import os
import requests
from tqdm import tqdm
import argparse

def copytree_common_to_binaries(folder, target="Debug", dst=None, dry_run=False):
    root_dir = os.getcwd()
    dst_path = os.path.join(root_dir, "Binaries", target, dst or "")
    if dry_run:
        print(f"[DRY RUN] Would copy {folder} to {dst_path}")
    else:
        shutil.copytree(
            os.path.join(root_dir, "SDK", folder), dst_path, dirs_exist_ok=True
        )
        print(f"Copied {folder} to {dst_path}")


def download_with_progress(url, zip_path, dry_run=False):
    if dry_run:
        print(f"[DRY RUN] Would download from {url} to {zip_path}")
        return

    # Ensure the directory exists
    os.makedirs(os.path.dirname(zip_path), exist_ok=True)

    response = requests.get(url, stream=True)
    file_size = int(response.headers.get("Content-Length", 0))
    with tqdm(total=file_size, unit="B", unit_scale=True, desc=zip_path) as pbar:
        with open(zip_path, "wb") as file_handle:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    file_handle.write(chunk)
                    pbar.update(len(chunk))


def download_and_extract(url, extract_path, folder, targets, dry_run=False):
    zip_path = "SDK/cache/" + url.split("/")[-1]
    if os.path.exists(zip_path):
        print(f"Using cached file {zip_path}")
    else:
        if not dry_run:
            print(f"Downloading from {url}...")
        download_with_progress(url, zip_path, dry_run)

    if dry_run:
        print(f"[DRY RUN] Would extract {zip_path} to {extract_path}")
        return

    print(f"Extracting to {extract_path}...")
    try:
        with zipfile.ZipFile(zip_path, "r") as zip_ref:
            zip_ref.extractall(extract_path)
        print(f"Downloaded and extracted successfully.")
        for target in targets:
            copytree_common_to_binaries(folder, target=target, dry_run=dry_run)
    except Exception as e:
        print(f"Error extracting {zip_path}: {e}")


def process_usd(targets, dry_run=False, keep_original_files=True):
    # First download and extract the source files
    url = (
        "https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v24.11.zip"
    )

    zip_path = os.path.join(
        os.path.dirname(__file__), "SDK", "cache", url.split("/")[-1]
    )
    if os.path.exists(zip_path):
        print(f"Using cached file {zip_path}")
    else:
        if not dry_run:
            print(f"Downloading from {url}...")
        download_with_progress(url, zip_path, dry_run)

    # Extract the downloaded zip file
    extract_path = os.path.join(os.path.dirname(__file__), "SDK", "OpenUSD", "source")
    if keep_original_files and os.path.exists(extract_path):
        print(f"Keeping original files in {extract_path}")
    else:
        if dry_run:
            print(f"[DRY RUN] Would extract {zip_path} to {extract_path}")
        else:
            try:
                with zipfile.ZipFile(zip_path, "r") as zip_ref:
                    zip_ref.extractall(extract_path)
                print(f"Downloaded and extracted successfully.")
            except Exception as e:
                print(f"Error extracting {zip_path}: {e}")
                return

    # Call the build script with the specified options
    build_script = os.path.join(
        extract_path, "OpenUSD-24.11", "build_scripts", "build_usd.py"
    )

    # Check if the user has a debug python installed
    has_python_d = os.system("python_d --version >nul 2>&1") == 0

    if has_python_d:
        use_debug_python = "--debug-python "
    else:
        use_debug_python = ""

    for target in targets:
        vulkan_support = ""
        if "VULKAN_SDK" in os.environ:
            vulkan_support = "-DPXR_ENABLE_VULKAN_SUPPORT=ON"
        else:
            print("Warning: VULKAN_SDK is not in the path. Highly recommend setting it for Vulkan support.")
        
        build_command = f"python {build_script} --build-args USD,\"-DPXR_ENABLE_GL_SUPPORT=ON {vulkan_support}\" --openvdb {use_debug_python}--ptex --openimageio --opencolorio --no-examples --no-tutorials --build-variant {target.lower()} ./SDK/OpenUSD/{target}"
        
        if dry_run:
            print(f"[DRY RUN] Would run: {build_command}")
        else:
            os.system(build_command)

    
    # Copy the built binaries to the Binaries folder
    for target in targets:
        copytree_common_to_binaries(os.path.join("OpenUSD", target, "bin"), target=target, dry_run=dry_run)
        copytree_common_to_binaries(os.path.join("OpenUSD", target, "lib"), target=target, dry_run=dry_run)
        copytree_common_to_binaries(os.path.join("OpenUSD", target, "plugin"), target=target, dry_run=dry_run)
        
        # Copy libraries and resources wholly
        copytree_common_to_binaries(os.path.join("OpenUSD", target, "libraries"), target=target, dst="libraries", dry_run=dry_run)
        copytree_common_to_binaries(os.path.join("OpenUSD", target, "resources"), target=target, dst="resources", dry_run=dry_run)

def main():
    parser = argparse.ArgumentParser(description="Download and configure libraries.")
    parser.add_argument(
        "--build_variant", nargs="*", default=["Debug"], help="Specify build variants."
    )
    parser.add_argument(
        "--library",
        choices=["slang", "dxc", "openusd"],
        help="Specify the library to configure.",
    )
    parser.add_argument("--all", action="store_true", help="Configure all libraries.")
    parser.add_argument(
        "--dry-run",
        "-n",
        action="store_true",
        help="Print actions without executing them.",
    )
    parser.add_argument(
        "--keep-original-files",
        type=bool,
        default=True,
        help="Keep original files if the extract path exists.",
    )
    args = parser.parse_args()

    targets = args.build_variant
    dry_run = args.dry_run
    keep_original_files = args.keep_original_files

    if args.all:
        args.library = ["openusd", "slang", "dxc"]
    elif not args.library:
        print(
            "No library specified and --all not set. No libraries will be configured."
        )
        return
    else:
        args.library = [args.library]

    if dry_run:
        print(f"[DRY RUN] Selected build variants: {targets}")

    urls = {
        "slang": "https://github.com/shader-slang/slang/releases/download/v2024.14.3/slang-2024.14.3-windows-x86_64.zip",
        "dxc": "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2407/dxc_2024_07_31.zip",
    }
    folders = {"slang": "slang/bin", "dxc": "dxc/bin/x64"}

    for lib in args.library:
        if lib == "openusd":
            process_usd(targets, dry_run, keep_original_files)
        else:
            download_and_extract(
                urls[lib], f"./SDK/{lib}", folders[lib], targets, dry_run
            )


if __name__ == "__main__":
    main()
