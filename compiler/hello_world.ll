define i32 @main() {
%a = alloca i32
store i32 10, i32* %a
%1 = load i32, i32* %a
ret i32 %1
}
