
declare void @std_puts(i8* nocapture, i32 noundef)

@.str = private unnamed_addr constant [15 x i8] c"Hello, world!\0A\00"

define i32 @main() {
entry:
    ; Convert [13 x i8]* to i8  *...
    %cast210 = getelementptr [15 x i8],[15 x i8]* @.str, i64 0, i64 0
    call void @std_puts(i8* %cast210, i32 15)
    ret i32 0
}

