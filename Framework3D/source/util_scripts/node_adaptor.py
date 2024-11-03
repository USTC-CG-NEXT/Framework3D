import os
import sys
import re

def process_cpp_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    node_name = None
    for i, line in enumerate(lines):
        match = re.match(r'namespace USTC_CG::node_(\w+) {', line)
        if match:
            node_name = match.group(1)
            lines[i] = 'NODE_DEF_OPEN_SCOPE\n'
            break

    if node_name:
        for i, line in enumerate(lines):
            if 'static void node_declare(NodeDeclarationBuilder& b)' in line:
                lines[i] = f'NODE_DECLARATION_FUNCTION({node_name})\n'
            elif 'static void node_exec(ExeParams params)' in line:
                lines[i] = f'NODE_EXECUTION_FUNCTION({node_name})\n'

        lines[-1] = f'NODE_DECLARATION_UI({node_name})\nNODE_DEF_CLOSE_SCOPE\n'

        with open(file_path, 'w') as file:
            file.writelines(lines)

def scan_directory(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.cpp'):
                process_cpp_file(os.path.join(root, file))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python node_adaptor.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    scan_directory(directory)