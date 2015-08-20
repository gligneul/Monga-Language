; ModuleID = 'monga-executable'

@.false = global [6 x i8] c"false\00"
@.true = global [5 x i8] c"true\00"
@.boolean = global [2 x i8*] [i8* getelementptr inbounds ([6 x i8]* @.false, i32 0, i32 0), i8* getelementptr inbounds ([5 x i8]* @.true, i32 0, i32 0)]

declare i32 @printf(i8*, ...)

define i32 @fat(i32 %n) {
entry:
  br label %while_expression

while_expression:                                 ; preds = %while_statement, %entry
  %0 = phi i32 [ %n, %entry ], [ %4, %while_statement ]
  %1 = phi i32 [ 1, %entry ], [ %3, %while_statement ]
  %2 = icmp sge i32 %0, 1
  br i1 %2, label %while_statement, label %while_out

while_statement:                                  ; preds = %while_expression
  %3 = mul i32 %1, %0
  %4 = sub i32 %0, 1
  br label %while_expression

while_out:                                        ; preds = %while_expression
  ret i32 %1
}

