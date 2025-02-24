import sys

def generate_cpp_file(nodename):
    cpp_content = f"""
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION({nodename})
{{
    // Function content omitted
}}

NODE_EXECUTION_FUNCTION({nodename})
{{
    // Function content omitted
}}

NODE_DECLARATION_UI({nodename});
NODE_DEF_CLOSE_SCOPE
"""
    return cpp_content

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python add_node.py <nodename>")
        sys.exit(1)

    nodename = sys.argv[1]
    cpp_content = generate_cpp_file(nodename)
    output_file = f"{nodename}.cpp"

    with open(output_file, "w") as file:
        file.write(cpp_content)

    print(f"Generated {output_file}")