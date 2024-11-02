import os
import re
import json
import argparse

def scan_cpp_files(directories):
    node_pattern = re.compile(r'NODE_EXECUTION_FUNCTION\((\w+)\)')
    nodes = {}

    for directory in directories:
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith('.cpp'):
                    file_path = os.path.join(root, file)
                    with open(file_path, 'r') as f:
                        content = f.read()
                        matches = node_pattern.findall(content)
                        if matches:
                            file_name_without_suffix = os.path.splitext(file)[0]
                            nodes[file_name_without_suffix] = matches

    return nodes

def main():
    parser = argparse.ArgumentParser(description='Scan cpp files for NODE_EXECUTION_FUNCTION and generate JSON.')
    parser.add_argument('--nodes', nargs='+', type=str, help='Paths to the directories containing node cpp files', default=[])
    parser.add_argument('--conversions', nargs='+', type=str, help='Paths to the directories containing conversion cpp files', default=[])
    parser.add_argument('--output', type=str, help='Path to the output JSON file')
    args = parser.parse_args()

    result = {}

    if args.nodes:
        result['nodes'] = scan_cpp_files(args.nodes)
    else:
        result['nodes'] = {}

    if args.conversions:
        result['conversions'] = scan_cpp_files(args.conversions)
    else:
        result['conversions'] = {}

    with open(args.output, 'w') as json_file:
        json.dump(result, json_file, indent=4)

if __name__ == "__main__":
    main()