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
        self.variables = []
        self.label_i = 0
        self.if_history = []
        self.loop_history = []
        self.end_history = []
        self.last_token = ""

    def get_output(self):
        return self.output

    def name_and_params_from_split(self, split):
        name = next(split)
        params = []
        if name.endswith("()"):
            name = name[:-2]
        elif name.endswith(")"):
            splited = name.split("(") # )
            name = splited[0]
            params.append(splited[1][:-1])
        else:
            while (next_token := next(split, None)) != None:
                name += " " + next_token
                if next_token.endswith(")"):
                    splited = name.split("(") # )
                    name = splited[0]
                    params = [item.strip() for item in splited[1][:-1].split(",")]
                    break
        return (name, params)

    def parse_exists(self, split):
        return_type = next(split)
        (name, params) = self.name_and_params_from_split(split)

        declare = {}
        declare["return_type"] = return_type
        declare["name"] = name
        declare["params"] = params
        self.declared.append(declare)

        self.output += f"declare {return_type} @{name}({', '.join(params)})\n"

    def parse_proc(self, split):
        self.var_i = -1
        return_type = next(split)
        (name, params) = self.name_and_params_from_split(split)

        declare = {}
        declare["return_type"] = return_type
        declare["name"] = name
        declare["params"] = [param.split()[0] for param in params]
        self.declared.append(declare)

        parm_str = ""
        first = True
        for param in params:
            param = param.split(" ")
            next_var = self.next_var()
            self.variables.append((param[0], param[1][1:], next_var))
            if not first:
                parm_str += ", "
            parm_str += f"{param[0]} %{next_var}"
            first = False

        self.output += f"define {return_type} @{name}({parm_str}) {{\n" # }}
        self.var_i += 1
        self.end_history.append("proc")

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
        if len(func["params"]) == 0:
            if func["return_type"] != "void":
                next_var = self.next_var()
                self.output += f"%{next_var} = "
                self.history.append((func["return_type"], f"%{next_var}"))
            self.output += f"call {func['return_type']} @{func['name']}()\n"
        else:
            next_var = 0
            if func["return_type"] != "void":
                next_var = self.next_var()
                buffer += f"%{next_var} = "
            buffer += f"call {func['return_type']} @{func['name']}(" #)
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
            if func["return_type"] != "void":
                self.history.append((func["return_type"], f"%{next_var}"))
            self.output += buffer + ")\n"

    def parse_variable_declare(self):
        name = self.history.pop()[1]
        tail = self.history.pop()
        if tail[0] == "-type-":
            var_type = tail[1][1:]
            self.variables.append((f"{var_type}*", name, name))
            self.output += f"%{name} = alloca {var_type}\n"
        else:
            size = tail[1]
            arr_type = self.history.pop()[1][1:]

            self.variables.append((f"[{size} x {arr_type}]", name, name))
            self.output += f"%{name} = alloca [{size} x {arr_type}]\n"

    def parse_array_at(self):
        name  = self.history.pop()[1][1:]
        index = self.history.pop()

        found = False
        for var in self.variables:
            if str(var[2]) == name:
                found = True
                next_var = self.next_var()
                if "x" in var[0]:
                    self.output += f"%{next_var} = getelementptr {var[0]}, {var[0]}* %{var[2]}, i64 0, {index[0]} {index[1]}\n"
                    self.history.append((f"{var[0].split('x')[1][1:-1]}*", f"%{next_var}"))
                else:
                    self.output += f"%{next_var} = getelementptr {var[0][:-1]}, {var[0]} %{var[2]}, {index[0]} {index[1]}\n"
                    self.history.append((f"{var[0]}", f"%{next_var}"))
        if not found:
            print(f"@ Variable {name} not found")
            exit(1)

    def parse_assign(self):
        dest = self.history.pop()
        src  = self.history.pop()

        self.output += f"store {dest[0][:-1]} {src[1]}, {dest[0]} {dest[1]}\n"

    def parse_deref(self):
        ptr = self.history.pop()

        next_var = self.next_var()
        self.output += f"%{next_var} = load {ptr[0][:-1]}, {ptr[0]} {ptr[1]}\n"
        self.history.append((ptr[0][:-1], f"%{next_var}"))

    def parse_as(self):
        target = self.history.pop()
        src = self.history.pop()

        next_var = self.next_var()
        self.output += f"%{next_var} = "
        if int(target[1][2:]) > int(src[0][1:]):
            self.output += "sext "
        else:
            self.output += "trunc "

        self.output += f"{src[0]} {src[1]} to {target[1][1:]}\n"
        self.history.append((target[1][1:], f"%{next_var}"))

    def parse_eq(self):
        b = self.history.pop()
        a = self.history.pop()
        next_var = self.next_var()
        self.output += f"%{next_var} = icmp eq {a[0]} {a[1]}, {b[1]}\n"
        self.history.append(("i1", f"%{next_var}"))

    def parse_if(self):
        cond = self.history.pop()
        next_label = self.next_label()
        self.output += f"br {cond[0]} {cond[1]}, label %if{next_label}, label %ife{next_label}\n"
        self.if_history.append((False, next_label))
        self.output += f"if{next_label}:\n"
        self.end_history.append("if")

    def parse_ifelse(self):
        poped = self.if_history.pop()
        if not self.last_token in ["_break", "_end", "_ret"]:
            self.output += f"br label %ifend{poped[1]}\n"
        self.output += f"ife{poped[1]}:\n"
        self.if_history.append((True, poped[1]))

    def parse_endif(self):
        suffix = "e"
        poped = self.if_history.pop()
        if poped[0]:
            suffix = "end"
        if not self.last_token in ["_break", "_end"]:
            self.output += f"br label %if{suffix}{poped[1]}\n"
        self.output += f"if{suffix}{poped[1]}:\n"

    def parse_loop(self):
        next_label = self.next_label()
        self.output += f"br label %loop{next_label}\n"
        self.output += f"loop{next_label}:\n"
        self.end_history.append("loop")
        self.loop_history.append(next_label)

    def parse_endloop(self):
        poped = self.loop_history.pop()
        self.output += f"br label %loop{poped}\n"
        self.output += f"loope{poped}:\n"

    def parse_break(self):
        self.output += f"br label %loope{self.loop_history[-1]}\n"

    def parse_arth_generic(self, inst):
        first  = self.history.pop()
        second = self.history.pop()
        self.output += f"%{self.next_var()} = {inst} {first[0]} {first[1]}, {second[1]}\n"
        self.history.append((first[0], f"%{self.var_i}"))

    def parse_add(self):
        self.parse_arth_generic("add")

    def parse_sub(self):
        self.parse_arth_generic("sub")

    def parse_mod(self):
        self.parse_arth_generic("srem")

    def parse_div(self):
        self.parse_arth_generic("sdiv")

    def next_var(self):
        self.var_i += 1
        return self.var_i

    def next_string(self):
        self.string_i += 1
        return self.string_i

    def next_label(self):
        self.label_i += 1
        return self.label_i

    def compile(self):
        split = iter(self.input.split())
        in_commment = False
        for token in split:
            if in_commment:
                if token.endswith(";"):
                    in_commment = False
                continue

            if token == "_exists":
                self.parse_exists(split)
            elif token == "_proc":
                self.parse_proc(split)
            elif token == "_end":
                last = self.end_history.pop()
                if last == "proc":
                    self.output += "}\n"
                elif last == "if":
                    self.parse_endif()
                elif last == "loop":
                    self.parse_endloop()
                else:
                    print(f"Unknown end history {last}")
                    return
            elif token == "_ret":
                last = self.history.pop()
                if last[0] == "-type-":
                    self.output += f"ret {last[1][1:]}\n"
                else:
                    self.output += f"ret {last[0]} {last[1]}\n"
            elif token == "_as":
                self.parse_as()
            elif token == "_eq":
                self.parse_eq()
            elif token == "_add":
                self.parse_add()
            elif token == "_sub":
                self.parse_sub()
            elif token == "_mod":
                self.parse_mod()
            elif token == "_div":
                self.parse_div()
            elif token == "_if":
                self.parse_if()
            elif token == "_else":
                self.parse_ifelse()
            elif token == "_loop":
                self.parse_loop()
            elif token == "_break":
                self.parse_break()
            elif token.startswith("_"):
                print(f"Unknown keyword '{token}'")
                return
            elif token.startswith(";"):
                if not (token.endswith(";") and len(token) > 1):
                    in_commment = True
                continue
            elif token == ":":
                self.parse_variable_declare()
            elif token.startswith(":"):
                self.history.append(("-type-", token))
            elif token == "@":
                self.parse_array_at()
            elif token == "=":
                self.parse_assign()
            elif token == "*":
                self.parse_deref()
            elif token.startswith("\""):
                self.parse_string_literal(split, token)
            elif token.isdigit():
                #self.history.append(f"i32 {token}")
                self.history.append(("i32", token))
            elif token.startswith("$"):
                found = False
                for var in self.variables:
                    if var[1] == token[1:]:
                        self.history.append((var[0], f"%{var[2]}"))
                        found = True
                        break
                if not found:
                    self.history.append(("-var_name-", token[1:]))
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

            self.last_token = token

        for lit in self.string_literals:
            self.output += f"@.str{lit[0]} = private unnamed_addr constant [{lit[2]} x i8] c\"{lit[1]}\"\n"

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

