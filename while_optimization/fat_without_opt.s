	.text
	.file	"fat_without_opt.ll"
	.globl	fat
	.align	16, 0x90
	.type	fat,@function
fat:                                    # @fat
	.cfi_startproc
# BB#0:                                 # %entry
	movl	$1, %eax
	jmp	.LBB0_1
	.align	16, 0x90
.LBB0_2:                                # %while_statement
                                        #   in Loop: Header=BB0_1 Depth=1
	imull	%edi, %eax
	decl	%edi
.LBB0_1:                                # %while_expression
                                        # =>This Inner Loop Header: Depth=1
	testl	%edi, %edi
	jg	.LBB0_2
# BB#3:                                 # %while_out
	retq
.Ltmp0:
	.size	fat, .Ltmp0-fat
	.cfi_endproc

	.type	.false,@object          # @.false
	.data
	.globl	.false
.false:
	.asciz	"false"
	.size	.false, 6

	.type	.true,@object           # @.true
	.globl	.true
.true:
	.asciz	"true"
	.size	.true, 5

	.type	.boolean,@object        # @.boolean
	.globl	.boolean
	.align	8
.boolean:
	.quad	.false
	.quad	.true
	.size	.boolean, 16


	.section	".note.GNU-stack","",@progbits
