declare void @std_puts(i8*, i32)
define void @std_put_i32(i32 %0) {
%num = alloca i32
store i32 %0, i32* %num
%buff = alloca [32 x i8]
%2 = getelementptr [32 x i8], [32 x i8]* %buff, i64 0, i32 31
store i8 0, i8* %2
%i = alloca i32
store i32 31, i32* %i
br label %loop1
loop1:
%3 = load i32, i32* %num
%4 = icmp eq i32 0, %3
br i1 %4, label %if2, label %ife2
if2:
br label %loope1
ife2:
%5 = load i32, i32* %num
%6 = srem i32 %5, 10
%7 = add i32 %6, 48
%8 = trunc i32 %7 to i8
%9 = load i32, i32* %i
%10 = getelementptr [32 x i8], [32 x i8]* %buff, i64 0, i32 %9
store i8 %8, i8* %10
%11 = load i32, i32* %i
%12 = sub i32 %11, 1
store i32 %12, i32* %i
%13 = load i32, i32* %num
%14 = sdiv i32 %13, 10
store i32 %14, i32* %num
br label %loop1
loope1:
%15 = load i32, i32* %i
%16 = sub i32 32, %15
%17 = load i32, i32* %i
%18 = getelementptr [32 x i8], [32 x i8]* %buff, i64 0, i32 %17
call void @std_puts(i8* %18, i32 %16)
ret void
}
