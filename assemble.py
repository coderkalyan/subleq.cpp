import sys

def assemble_to_memory(instructions):
    """
    Assemble list of (A, B, C) instructions into C++ memory<> format,
    formatted in triples across multiple lines.
    """
    lines = []
    for a, b, c in instructions:
        lines.append(
            f"    constant<{a}>, constant<{b}>, constant<{c}>"
        )
    body = ",\n".join(lines)
    return f"using program = memory<\n{body}\n>;"

def read_program_file(filename):
    """
    Read instructions from a file. Each line must contain 3 integers.
    """
    instructions = []
    with open(filename, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):  # allow comments
                continue
            parts = line.split()
            if len(parts) != 3:
                raise ValueError(f"Invalid line: {line}")
            instructions.append(tuple(int(p) for p in parts))
    return instructions

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python assemble.py <inputfile>")
        sys.exit(1)

    filename = sys.argv[1]
    program = read_program_file(filename)
    cpp_output = assemble_to_memory(program)
    print(cpp_output)

