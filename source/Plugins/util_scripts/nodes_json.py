import os
import re
import json
import argparse

def scan_cpp_files(directories, files, pattern):
    compiled_pattern = re.compile(pattern)
    nodes = {}

    for directory in directories:
        for root, _, dir_files in os.walk(directory):
            for file in dir_files:
                if file.endswith('.cpp'):
                    file_path = os.path.join(root, file)
                    with open(file_path, 'r', encoding = 'utf-8') as f:
                        content = f.read()
                        matches = compiled_pattern.findall(content)
                        if matches:
                            file_name_without_suffix = os.path.splitext(file)[0]
                            nodes[file_name_without_suffix] = matches

    for file in files:
        if file.endswith('.cpp'):
            with open(file, 'r', encoding = 'utf-8') as f:
                content = f.read()
                matches = compiled_pattern.findall(content)
                if matches:
                    file_name_without_suffix = os.path.splitext(os.path.basename(file))[0]
                    nodes[file_name_without_suffix] = matches

    return nodes


def main():
    parser = argparse.ArgumentParser(description='Scan cpp files for NODE_EXECUTION_FUNCTION and CONVERSION_EXECUTION_FUNCTION and generate JSON.')
    parser.add_argument('--nodes-dir', nargs='+', type=str, help='Paths to the directories containing node cpp files', default=[])
    parser.add_argument('--nodes-files', nargs='+', type=str, help='Paths to the node cpp files', default=[])
    parser.add_argument('--conversions-dir', nargs='+', type=str, help='Paths to the directories containing conversion cpp files', default=[])
    parser.add_argument('--conversions-files', nargs='+', type=str, help='Paths to the conversion cpp files', default=[])
    parser.add_argument('--output', type=str, help='Path to the output JSON file')
    args = parser.parse_args()

    result = {}

    if args.nodes_dir or args.nodes_files:
        node_pattern = r'NODE_EXECUTION_FUNCTION\((\w+)\)'
        result['nodes'] = scan_cpp_files(args.nodes_dir, args.nodes_files, node_pattern)
    else:
        result["nodes"] = {}

    if args.conversions_dir or args.conversions_files:
        conversion_pattern = r'CONVERSION_EXECUTION_FUNCTION\((\w+),\s*(\w+)\)'
        conversions = scan_cpp_files(args.conversions_dir, args.conversions_files, conversion_pattern)
        result['conversions'] = {k: [f"{match[0]}_to_{match[1]}" for match in v] for k, v in conversions.items()}
    else:
        result["conversions"] = {}

    with open(args.output, "w", encoding="utf-8") as json_file:
        json.dump(result, json_file, indent=4)


if __name__ == "__main__":
    main()
