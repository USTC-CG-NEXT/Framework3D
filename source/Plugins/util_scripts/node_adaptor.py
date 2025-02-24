import os
import sys
import re


def process_cpp_file(file_path):
    print(f"Processing file: {file_path}")
    with open(file_path, "r", encoding="utf-8") as file:
        lines = file.readlines()

    node_name = None
    in_register_function = False
    new_lines = []

    for i, line in enumerate(lines):
        if in_register_function:
            if line.strip() == "}":
                in_register_function = False
            continue

        if re.match(r"static void node_register\(\)", line):
            in_register_function = True
            continue

        match = re.match(r"namespace USTC_CG::node_(\w+) {", line)
        if match:
            node_name = match.group(1)
            new_lines.append('#include "nodes/core/def/node_def.hpp"\n')
            new_lines.append('NODE_DEF_OPEN_SCOPE\n')
            print(f"Found node name: {node_name}")
            continue

        declare_match = re.match(r"static void node_(\w+)\(NodeDeclarationBuilder& b\)", line)
        exec_match = re.match(r"static void node_(\w+)\(ExeParams params\)", line)
        if declare_match:
            func_name = declare_match.group(1)
            new_lines.append(f'NODE_DECLARATION_FUNCTION({node_name})\n')
            print(f"Replaced node_declare function for node: {func_name}")
        elif exec_match:
            func_name = exec_match.group(1)
            new_lines.append(f'NODE_EXECUTION_FUNCTION({node_name})\n')
            print(f"Replaced node_exec function for node: {func_name}")
        elif re.match(r'#include "Nodes/.*"', line):
            print(f"Removed include statement: {line.strip()}")
            continue
        else:
            # Replace decl::Int with int, decl::Float with float, decl::Geometry with Geometry
            line = line.replace("decl::Int", "int")
            line = line.replace("decl::Float", "float")
            line = line.replace("decl::Geometry", "Geometry")
            line = line.replace("decl::Bool", "bool")
            line = line.replace("decl::String", "std::string")
            line = line.replace("decl::Any", "entt::meta_any")
            new_lines.append(line)

    if node_name:
        new_lines[-1] = f"NODE_DECLARATION_UI({node_name});\nNODE_DEF_CLOSE_SCOPE\n"
        print(f"Added UI and close scope for node: {node_name}")

        with open(file_path, "w", encoding="utf-8") as file:
            file.writelines(new_lines)
        print(f"Finished processing file: {file_path}")


def scan_directory(directory):
    print(f"Scanning directory: {directory}")
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".cpp"):
                process_cpp_file(os.path.join(root, file))
    print("Finished scanning directory")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python node_adaptor.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    scan_directory(directory)
