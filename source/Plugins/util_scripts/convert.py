from pxr import Usd
import argparse
import os
from pxr import UsdGeom

def convert_usda_to_usd(input_file, output_file):
    stage = Usd.Stage.Open(input_file)
    if not stage:
        raise ValueError(f"Failed to open USD stage from file: {input_file}")
    stage.GetRootLayer().Export(output_file)
    print(f"Converted {input_file} to {output_file}")


def main():
    parser = argparse.ArgumentParser(
        description="Convert all USDA files in the directory and its subdirectories to USD files"
    )
    parser.add_argument("directory", help="Path to the directory containing USDA files")
    args = parser.parse_args()

    for root, _, files in os.walk(args.directory):
        for filename in files:
            if filename.endswith(".usda"):
                input_file = os.path.join(root, filename)
                output_file = os.path.splitext(input_file)[0] + ".usd"
                convert_usda_to_usd(input_file, output_file)


if __name__ == "__main__":
    main()
