import os
import re
import json
import argparse

def scan_cpp_files(directories, pattern):
    compiled_pattern = re.compile(pattern)
    nodes = {}

    for directory in directories:
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith('.cpp'):
                    file_path = os.path.join(root, file)
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        matches = compiled_pattern.findall(content)
                        if matches:
                            file_name_without_suffix = os.path.splitext(file)[0]
                            nodes[file_name_without_suffix] = matches

    return nodes

def main():
    parser = argparse.ArgumentParser(description='Scan cpp files for NODE_EXECUTION_FUNCTION and CONVERSION_EXECUTION_FUNCTION and generate JSON.')
    parser.add_argument('--nodes', nargs='+', type=str, help='Paths to the directories containing node cpp files', default=[])
    parser.add_argument('--conversions', nargs='+', type=str, help='Paths to the directories containing conversion cpp files', default=[])
    parser.add_argument('--output', type=str, help='Path to the output JSON file')
    args = parser.parse_args()

    result = {}

    if args.nodes:
        node_pattern = r'NODE_EXECUTION_FUNCTION\((\w+)\)'
        result['nodes'] = scan_cpp_files(args.nodes, node_pattern)
    else:
        result['nodes'] = {}

    if args.conversions:
        conversion_pattern = r'CONVERSION_EXECUTION_FUNCTION\((\w+),\s*(\w+)\)'
        conversions = scan_cpp_files(args.conversions, conversion_pattern)
        result['conversions'] = {k: [f"{match[0]}_to_{match[1]}" for match in v] for k, v in conversions.items()}
    else:
        result['conversions'] = {}

    with open(args.output, 'w') as json_file:
        json.dump(result, json_file, indent=4)

if __name__ == "__main__":
    main()