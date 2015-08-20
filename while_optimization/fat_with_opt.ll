; ModuleID = 'monga-executable'

@.false = global [6 x i8] c"false\00"
@.true = global [5 x i8] c"true\00"
@.boolean = global [2 x i8*] [i8* getelementptr inbounds ([6 x i8]* @.false, i32 0, i32 0), i8* getelementptr inbounds ([5 x i8]* @.true, i32 0, i32 0)]

declare i32 @printf(i8*, ...)

define i32 @fat(i32 %n) {
entry:
  %0 = icmp sge i32 %n, 1
  br i1 %0, label %loop_in, label %loop_end

loop_in:                                          ; preds = %loop_in, %entry
  %1 = phi i32 [ %n, %entry ], [ %4, %loop_in ]
  %2 = phi i32 [ 1, %entry ], [ %3, %loop_in ]
  %3 = mul i32 %2, %1
  %4 = sub i32 %1, 1
  %5 = icmp sge i32 %4, 1
  br i1 %5, label %loop_in, label %loop_end

loop_end:                                         ; preds = %loop_in, %entry
  %6 = phi i32 [ %n, %entry ], [ %4, %loop_in ]
  %7 = phi i32 [ 1, %entry ], [ %3, %loop_in ]
  ret i32 %7
}

