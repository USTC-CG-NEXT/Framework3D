import os
import sys
import re

def process_cpp_file(file_path):
    print(f"Processing file: {file_path}")
    with open(file_path, 'r') as file:
        lines = file.readlines()

    node_name = None
    for i, line in enumerate(lines):
        match = re.match(r'namespace USTC_CG::node_(\w+) {', line)
        if match:
            node_name = match.group(1)
            lines[i] = 'NODE_DEF_OPEN_SCOPE\n'
            print(f"Found node name: {node_name}")
            break

    if node_name:
        for i, line in enumerate(lines):
            declare_match = re.match(r'static void node_(\w+)\(NodeDeclarationBuilder& b\)', line)
            exec_match = re.match(r'static void node_(\w+)\(ExeParams params\)', line)
            if declare_match:
                func_name = declare_match.group(1)
                lines[i] = f'NODE_DECLARATION_FUNCTION({func_name})\n'
                print(f"Replaced node_declare function for node: {func_name}")
            elif exec_match:
                func_name = exec_match.group(1)
                lines[i] = f'NODE_EXECUTION_FUNCTION({func_name})\n'
                print(f"Replaced node_exec function for node: {func_name}")

        lines[-1] = f'NODE_DECLARATION_UI({node_name})\nNODE_DEF_CLOSE_SCOPE\n'
        print(f"Added UI and close scope for node: {node_name}")

        with open(file_path, 'w') as file:
            file.writelines(lines)
        print(f"Finished processing file: {file_path}")

def scan_directory(directory):
    print(f"Scanning directory: {directory}")
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.cpp'):
                process_cpp_file(os.path.join(root, file))
    print("Finished scanning directory")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python node_adaptor.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    scan_directory(directory)