import sys
import os
import argparse


def generate_header(library_name):
    header_template = f"""
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }}

#if defined(_MSC_VER)
#  define {library_name}_EXPORT   __declspec(dllexport)
#  define {library_name}_IMPORT   __declspec(dllimport)
#  define {library_name}_NOINLINE __declspec(noinline)
#  define {library_name}_INLINE   __forceinline
#else
#  define {library_name}_EXPORT    __attribute__ ((visibility("default")))
#  define {library_name}_IMPORT
#  define {library_name}_NOINLINE  __attribute__ ((noinline))
#  define {library_name}_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_{library_name}_MODULE
#  define {library_name}_API {library_name}_EXPORT
#  define {library_name}_EXTERN extern
#else
#  define {library_name}_API {library_name}_IMPORT
#  if defined(_MSC_VER)
#    define {library_name}_EXTERN
#  else
#    define {library_name}_EXTERN extern
#  endif
#endif
"""
    return header_template


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate a C++ API header file for a given library.")
    parser.add_argument("library_name", help="The name of the library.")
    parser.add_argument(
        "output_directory",
        nargs="?",
        default=os.getcwd(),
        help="The directory where the header file will be saved (default: current working directory).",
    )
    parser.add_argument(
        "--output_file", default="api.h", help="The name of the output header file (default: api.h)."
    )

    args = parser.parse_args()

    library_name = args.library_name.upper()
    output_directory = args.output_directory
    output_file_name = args.output_file

    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    header = generate_header(library_name)
    output_file = os.path.join(output_directory, output_file_name)

    with open(output_file, "w", encoding="utf-8") as file:
        file.write(header)

    print(f"Header file generated at: {output_file}")
