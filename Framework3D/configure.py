import zipfile
import shutil
import os


# def copytree_to_binaries(folder, dst=None, target="Debug"):
#     root_dir = os.getcwd()
#     print(
#         "Copying {0} to the binary directory for target {1}...".format(folder, target)
#     )
#     if dst is None:
#         shutil.copytree(
#             root_dir + "/SDK/" + target + "/" + folder,
#             root_dir + "/Binaries/" + target + "/",
#             dirs_exist_ok=True,
#         )
#     else:
#         shutil.copytree(
#             root_dir + "/SDK/" + target + "/" + folder,
#             root_dir + "/Binaries/" + target + "/" + dst,
#             dirs_exist_ok=True,
#         )
#     return


def copytree_common_to_binaries(folder, target="Debug", dst=None):
    root_dir = os.getcwd()
    print(
        "Copying {0} to the binary directory for target {1}...".format(folder, target)
    )
    if dst is None:
        shutil.copytree(
            root_dir + "/SDK/" + "/" + folder,
            root_dir + "/Binaries/" + target + "/",
            dirs_exist_ok=True,
        )
    else:
        shutil.copytree(
            root_dir + "/SDK/" + "/" + folder,
            root_dir + "/Binaries/" + target + "/" + dst,
            dirs_exist_ok=True,
        )
    return


# def fix_usd_path(target):
#     if os.name == "posix":
#         # macOS: no python folder
#         return
#     root_dir = os.getcwd()
#     binary_dir = root_dir + "/Binaries/" + target + "/"
#     if os.path.exists(binary_dir + "pxr"):
#         shutil.rmtree(binary_dir + "pxr")
#     shutil.move(binary_dir + "python/pxr", binary_dir + "pxr")
#     shutil.rmtree(binary_dir + "python")


# def check_is_script(filename):
#     if os.name == "nt":
#         return filename.endswith("cmd") and filename.startswith("usd")
#     else:
#         return filename.startswith("usd") and not filename.endswith("so")


# def is_binary(file_name):
#     try:
#         with open(file_name, "tr") as check_file:
#             check_file.read()
#             return False
#     except:
#         return True


# def platform_path_name():
#     if os.name == "nt":
#         return "path"
#     else:
#         return "PATH"


# def get_platform_splitter():
#     if os.name == "nt":
#         return ";"
#     else:
#         return ":"


# def fix_usd_python_code(target):
#     root_dir = os.getcwd()
#     binary_dir = root_dir + "/Binaries/" + target + "/"
#     for filename in os.listdir(binary_dir):
#         if check_is_script(filename):
#             python_script_name = filename.split(".")[0]
#             if not is_binary(binary_dir + python_script_name):
#                 with open(binary_dir + python_script_name, "r") as file:
#                     filedata = file.read()
#                 filedata = filedata.replace(
#                     "from __future__ import print_function\n",
#                     "from __future__ import print_function\nimport os\nos.environ['{0}'] += '{1}' + os.getcwd()\n".format(
#                         platform_path_name(), get_platform_splitter()
#                     ),
#                 )

#                 # Allow the script to run multiple times
#                 filedata = filedata.replace(
#                     "import os\nos.environ['{0}'] += '{1}' + os.getcwd()\nimport os\nos.environ['{0}'] += '{1}' + os.getcwd()\n".format(
#                         platform_path_name(), get_platform_splitter()
#                     ),
#                     "import os\nos.environ['{0}'] += '{1}' + os.getcwd()\n".format(
#                         platform_path_name(), get_platform_splitter()
#                     ),
#                 )
#                 # Write the file out again
#                 with open(binary_dir + python_script_name, "w") as file:
#                     file.write(filedata)


# def process_SDK(target):
#     os.makedirs("./Binaries/" + target + "/", exist_ok=True)
#     if os.name == "nt":
#         tbb_lib = "tbb/bin"
#         folders_list = [
#             "OpenUSD/bin",
#             "OpenUSD/bin",
#             "OpenUSD/lib",
#             tbb_lib,
#             "MaterialX/bin",
#             "openvdb/bin",
#             "c-blosc/bin",
#             "zlib/bin",
#             "Imath/bin",
#         ]
#     else:
#         tbb_lib = "tbb/lib"  # TODO: fix
#         folders_list = [
#             "OpenUSD/bin",
#             "OpenUSD/lib",
#             "OpenUSD/lib",
#             tbb_lib,
#             "MaterialX/lib",
#             "openvdb/lib",
#             "c-blosc/lib",
#             "zlib/lib",
#             "Imath/lib",
#         ]

#     if os.name == "nt":
#         folders_list.append("boost/lib64-msvc-14.3")
#     for folder in folders_list:
#         copytree_to_binaries(folder, target=target)
#     copytree_to_binaries("OpenUSD/plugin/usd", "usd", target=target)

#     copytree_common_to_binaries("embree/bin", target=target)
#     copytree_common_to_binaries("dxc/bin/x64", target=target)
#     copytree_common_to_binaries("slang/prelude", dst="slang/prelude", target=target)

#     copytree_to_binaries("libtorch/lib", target=target)

#     if os.name == "nt":
#         copytree_common_to_binaries("slang/bin/windows-x64/release", target=target)
#     else:
#         copytree_common_to_binaries("slang/bin/macosx-aarch64/release", target=target)

#     fix_usd_path(target=target)
#     if os.name == "nt":
#         fix_usd_python_code(target=target)


# if __name__ == "__main__":
#     process_SDK("Debug")
#     process_SDK("Release")
#     process_SDK("RelWithDebInfo")

import urllib.request

targets = ["Debug", "Release", "RelWithDebInfo"]


def download_and_extract_slang():
    if os.name == "nt":
        url = "https://github.com/shader-slang/slang/releases/download/v2024.14.3/slang-2024.14.3-windows-x86_64.zip"
        zip_path = "slang-2024.14.3-windows-x86_64.zip"

    extract_path = "./SDK/slang"

    print("Downloading Slang from {0}...".format(url))
    urllib.request.urlretrieve(url, zip_path)
    print("Extracting Slang to {0}...".format(extract_path))
    with zipfile.ZipFile(zip_path, "r") as zip_ref:
        zip_ref.extractall(extract_path)
    os.remove(zip_path)
    print("Slang downloaded and extracted successfully.")
    if os.name == "nt":
        for target in targets:
            copytree_common_to_binaries("slang/bin", target=target)


def download_and_extract_dxc():
    if os.name == "nt":
        url = "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2407/dxc_2024_07_31.zip"
        zip_path = "dxc_2024_07_31.zip"

    extract_path = "./SDK/dxc"

    print("Downloading DXC from {0}...".format(url))
    urllib.request.urlretrieve(url, zip_path)
    print("Extracting DXC to {0}...".format(extract_path))
    with zipfile.ZipFile(zip_path, "r") as zip_ref:
        zip_ref.extractall(extract_path)
    os.remove(zip_path)
    print("DXC downloaded and extracted successfully.")
    if os.name == "nt":
        for target in targets:
            copytree_common_to_binaries("dxc/bin/x64", target=target)


if __name__ == "__main__":
    download_and_extract_slang()
    download_and_extract_dxc()
