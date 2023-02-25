declare void @std_puts(i8*, i32)
define i32 @main() {
%1 = getelementptr [14 x i8],[14 x i8]* @.str1, i64 0, i64 0
call void @std_puts(i8* %1, i32 14)
ret i32 0
}
@.str1 = private unnamed_addr constant [14 x i8] c"Hello world!\0A\00"
