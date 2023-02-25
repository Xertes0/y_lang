define i32 @main() {
%a = alloca [32 x i8]
%1 = getelementptr [32 x i8], [32 x i8]* %a, i64 0, i32 3
store i8 120, i8* %1
%2 = getelementptr [32 x i8], [32 x i8]* %a, i64 0, i32 3
%3 = load i8, i8* %2
%4 = sext i8 %3 to i32
ret i32 %4
}
