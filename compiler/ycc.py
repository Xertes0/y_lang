#!/usr/bin/env python3

import sys

class Compilator:
    def __init__(self, input):
        self.input = input
        self.output = ""
        self.var_i = 0
        self.history = []
        self.string_literals = []
        self.string_i = 0
        self.declared = []

    def get_output(self):
        return self.output

    def parse_exists(self, split):
        return_type = next(split)
        name = next(split)
        params = []
        if not name.endswith(")"):
            while (next_token := next(split, None)) != None:
                name += " " + next_token
                if next_token.endswith(")"):
                    splited = name.split("(") # )
                    name = splited[0]
                    params = [item.strip() for item in splited[1][:-1].split(",")]
                    break

        declare = {}
        declare["return_type"] = return_type
        declare["name"] = name
        declare["params"] = params
        self.declared.append(declare)

        self.output += f"declare {return_type} @{name}({', '.join(params)})\n"

    def parse_proc(self, split):
        return_type = next(split)
        name = next(split)

        self.output += f"define {return_type} @{name} {{\n" # }}

    def parse_string_literal(self, split, token):
        if token.endswith("\""):
            token = token[1:-1]
            str_i = self.next_string()
            str_len = len(token) - token.count('\\')
            token = token \
                .replace('\\0', '\\00') \
                .replace('\\n', '\\0A')
            self.string_literals.append((str_i, token, str_len))
            self.history.append(("-str-", str_i, str_len))
        else:
            while (next_token := next(split, None)) != None:
                token += " "
                token += next_token
                if(next_token.endswith("\"")):
                    token = token[1:-1]
                    str_i = self.next_string()
                    str_len = len(token) - token.count('\\')
                    token = token \
                        .replace('\\0', '\\00') \
                        .replace('\\n', '\\0A')
                    self.string_literals.append((str_i, token, str_len))
                    self.history.append(("-str-", str_i, str_len))

                    return

    def make_call(self, func):
        buffer = ""
        if func["return_type"] == "void":
            if len(func["params"]) == 0:
                self.output += f"call void @{func['name']}()"
            else:
                buffer += f"call void @{func['name']}(" #)
                first = True
                for param in func["params"]:
                    if not first:
                        buffer += ", "

                    poped = self.history.pop()
                    if poped[0] == "-str-":
                        new_var = self.next_var()
                        size = poped[2]
                        self.output += f"%{new_var} = getelementptr [{size} x i8],[{size} x i8]* @.str{poped[1]}, i64 0, i64 0\n"
                        buffer += f"i8* %{new_var}"
                    elif param != poped[0]:
                        print(f"mismatch {param} != {poped[0]}")
                        exit(1)
                    else:
                        buffer += f"{poped[0]} {poped[1]}"

                    first = False
                self.output += buffer + ")\n"
        else:
            print("Not implemented return type")
            exit()

    def next_var(self):
        self.var_i += 1
        return self.var_i

    def next_string(self):
        self.string_i += 1
        return self.string_i

    def compile(self):
        split = iter(self.input.split())
        for token in split:
            if token == "_exists":
                self.parse_exists(split)
            elif token == "_proc":
                self.parse_proc(split)
            elif token == "_end":
                self.output += "}\n"
            elif token == "_ret":
                last = self.history.pop()
                self.output += f"ret {last[0]} {last[1]}\n"
            elif token.startswith("_"):
                print(f"Unknown keyword '{token}'")
                return
            elif token.startswith("\""):
                self.parse_string_literal(split, token)
            elif token.isdigit():
                #self.history.append(f"i32 {token}")
                self.history.append(("i32", token))
            elif token == "+":
                first  = self.history.pop()
                second = self.history.pop()
                self.output += f"%{self.next_var()} = add {first[0]} {first[1]}, {second[1]}\n"
                #self.history.append(f"%{self.var_i}")
                self.history.append((first[0], f"%{self.var_i}"))
            elif token == "!":
                self.history = []
            else:
                found = False
                for declare in self.declared:
                    if declare["name"] == token:
                        self.make_call(declare)
                        found = True
                        break
                if not found:
                    print(f"Unknown token '{token}'")
                    return

        for lit in self.string_literals:
            self.output += f"@.str{lit[0]} = private unnamed_addr constant [{lit[2]} x i8] c\"{lit[1]}\"\n"
        print("End")

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

