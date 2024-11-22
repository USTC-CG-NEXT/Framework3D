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


def process_usd(targets, dry_run=False, keep_original_files=True, copy_only=False):
    if not copy_only:
        # First download and extract the source files
        url = "https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v24.11.zip"

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
        extract_path = os.path.join(
            os.path.dirname(__file__), "SDK", "OpenUSD", "source"
        )
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
                print(
                    "Warning: VULKAN_SDK is not in the path. Highly recommend setting it for Vulkan support."
                )

            build_variant_map = {
                "Debug": "debug",
                "Release": "release",
                "RelWithDebInfo": "relwithdebuginfo",
            }
            build_variant = build_variant_map.get(target, target.lower())
            if build_variant == "relwithdebuginfo":
                openvdb_args = 'OpenVDB,-DCMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO="RelWithDebInfo;Release;" '
            else:
                openvdb_args = " "

            build_command = f'python {build_script} --build-args USD,"-DPXR_ENABLE_GL_SUPPORT=ON {vulkan_support}" {openvdb_args}--openvdb {use_debug_python}--ptex --generator=Ninja --openimageio --opencolorio --no-examples --no-tutorials --build-variant {build_variant} ./SDK/OpenUSD/{target}'

            if dry_run:
                print(f"[DRY RUN] Would run: {build_command}")
            else:
                os.system(build_command)
                # A work around: usd currently has a bug with pch when building with ninja, so call it's doomed to fail. Call it again withouth specifying generator=ninja to build it with visual studio.
                if os.name == 'nt':  # Only do this on Windows
                    shutil.rmtree(os.path.join("SDK", "OpenUSD", target, "build", "OpenUSD-24.11"))
                    build_command = f'python {build_script} --build-args USD,"-DPXR_ENABLE_GL_SUPPORT=ON {vulkan_support}" {openvdb_args}--openvdb {use_debug_python}--ptex --openimageio --opencolorio --no-examples --no-tutorials --build-variant {build_variant} ./SDK/OpenUSD/{target}'
                    os.system(build_command)

    # Copy the built binaries to the Binaries folder
    for target in targets:
        copytree_common_to_binaries(
            os.path.join("OpenUSD", target, "bin"), target=target, dry_run=dry_run
        )
        copytree_common_to_binaries(
            os.path.join("OpenUSD", target, "lib"), target=target, dry_run=dry_run
        )
        copytree_common_to_binaries(
            os.path.join("OpenUSD", target, "plugin"), target=target, dry_run=dry_run
        )

        # Copy libraries and resources wholly
        copytree_common_to_binaries(
            os.path.join("OpenUSD", target, "libraries"),
            target=target,
            dst="libraries",
            dry_run=dry_run,
        )
        copytree_common_to_binaries(
            os.path.join("OpenUSD", target, "resources"),
            target=target,
            dst="resources",
            dry_run=dry_run,
        )

import concurrent.futures

def pack_sdk(dry_run=False):
    src_dir = os.path.join(os.getcwd(), "SDK")
    dst_dir = os.path.join(os.getcwd(), "SDK_temp")

    def copy_file(src_file, dst_file):
        if dry_run:
            print(f"[DRY RUN] Would copy {src_file} to {dst_file}")
        else:
            shutil.copy2(src_file, dst_file)

    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for root, dirs, files in os.walk(src_dir):
            # Skip build, cache directories and anything under */src/
            if any(skip_dir in root for skip_dir in ["\\build", "\\cache", "\\src", "\\source"]):
                continue

            # Create corresponding directory in destination
            relative_path = os.path.relpath(root, src_dir)
            dst_path = os.path.join(dst_dir, relative_path)
            if not dry_run:
                os.makedirs(dst_path, exist_ok=True)

            for file in files:
                if file.endswith(".pdb") or file == "libopenvdb.lib":
                    print(f"Skipping {os.path.join(root, file)}")
                    continue

                src_file = os.path.join(root, file)
                dst_file = os.path.join(dst_path, file)
                futures.append(executor.submit(copy_file, src_file, dst_file))

        # Wait for all threads to complete
        concurrent.futures.wait(futures)

        # Pack the SDK_temp directory into SDK.zip
        if dry_run:
            print(f"[DRY RUN] Would pack {dst_dir} into SDK.zip")
        else:
            shutil.make_archive("SDK", "zip", dst_dir)
            print(f"Packed {dst_dir} into SDK.zip")

        # Delete the SDK_temp directory
        if dry_run:
            print(f"[DRY RUN] Would delete {dst_dir}")
        else:
            shutil.rmtree(dst_dir)
            print(f"Deleted {dst_dir}")


def main():
    parser = argparse.ArgumentParser(description="Download and configure libraries.")
    parser.add_argument(
        "--build_variant", nargs="*", default=["Debug"], help="Specify build variants."
    )
    parser.add_argument(
        "--library",
        choices=["slang", "openusd"],
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
    parser.add_argument(
        "--copy-only",
        action="store_true",
        help="Only copy files, skip downloading and building.",
    )
    parser.add_argument(
        "--pack",
        action="store_true",
        help="Pack SDK files to SDK_temp, skipping pdb files and build/cache directories.",
    )
    args = parser.parse_args()

    targets = args.build_variant
    dry_run = args.dry_run
    keep_original_files = args.keep_original_files
    copy_only = args.copy_only

    if args.pack:
        pack_sdk(dry_run)
        return

    if args.all:
        args.library = ["openusd", "slang"]
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
    }
    folders = {"slang": "slang/bin"}

    for lib in args.library:
        if lib == "openusd":
            process_usd(targets, dry_run, keep_original_files, copy_only)
        else:
            if not copy_only:
                download_and_extract(
                    urls[lib], f"./SDK/{lib}", folders[lib], targets, dry_run
                )
            else:
                for target in targets:
                    copytree_common_to_binaries(
                        folders[lib], target=target, dry_run=dry_run
                    )


if __name__ == "__main__":
    main()
