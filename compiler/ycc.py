#!/usr/bin/env python3

import sys

class Compilator:
    def __init__(self, input):
        self.input = input
        self.output = ""
        self.var_i = 0
        self.history = []

    def get_output(self):
        return self.output

    def parse_proc(self, split):
        return_type = next(split)
        name = next(split)

        self.output += (f"define {return_type} @{name} {{\n")

    def next_var(self):
        self.var_i += 1
        return self.var_i

    def compile(self):
        split = iter(self.input.split())
        for token in split:
            if token == "_proc":
                self.parse_proc(split)
            elif token == "_end":
                self.output += "}\n"
            elif token == "_ret":
                last = self.history.pop()
                self.output += f"ret {last[0]} {last[1]}\n"
            elif token.startswith("_"):
                print(f"Unknown keyword '{token}'")
                return
            elif token.isdigit():
                #self.history.append(f"i32 {token}")
                self.history.append(("i32", token))
            elif token == "+":
                first  = self.history.pop()
                second = self.history.pop()
                self.output += f"%{self.next_var()} = add {first[0]} {first[1]}, {second[1]}\n"
                #self.history.append(f"%{self.var_i}")
                self.history.append((first[0], f"%{self.var_i}"))
            else:
                print(f"Unknown token '{token}'")
                return

def print_usage():
    print("Usage: ")
    print("    ./ycc.py <input> <output>")

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        print_usage()
        exit(1)

    input = open(sys.argv[1], "r").read()
    comp = Compilator(input)
    comp.compile()
    with open(sys.argv[2], "w") as output:
        output.write(comp.get_output())

