/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * monga.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h>

#include "ast/ast.h"
#include "backend/ir.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "util/error.h"

/* Generates the LLVM bitcode */
static void exportModule(LLVMModuleRef module);

/* Executes a LLVM module */
static int executeModule(LLVMModuleRef module);

int main(int argc, char* argv[])
{
    yyparse();
    SemanticAnalyseTree(parser_ast);
    LLVMModuleRef module = IRCompileModule(parser_ast);

    if (argc > 1) {
        if (strcmp(argv[1], "-bc") == 0)
            exportModule(module);
        else if (strcmp(argv[1], "-d") == 0)
            LLVMDumpModule(module);
        else
            Error("Unknown option: %s", argv[1]);
    }

    return executeModule(module);
}

static void exportModule(LLVMModuleRef module)
{
    if (LLVMWriteBitcodeToFile(module, "monga.bc") != 0) {
        Error("Error writing bitcode to file, skipping");
    }
}

static int executeModule(LLVMModuleRef module)
{
    LLVMExecutionEngineRef engine;
    char* error_msg = NULL;
    LLVMLinkInJIT();
    LLVMInitializeNativeTarget();
    if (LLVMCreateExecutionEngineForModule(&engine, module, &error_msg) != 0) {
        Error("failed to create execution engine");
    }
    if (error_msg != NULL) {
        LLVMDisposeMessage(error_msg);
        Error("error: %s", error_msg);
    }
    LLVMValueRef main_function = LLVMGetNamedFunction(module, "main");
    if (main_function == NULL) {
        Error("main function not found");
    }
    LLVMGenericValueRef result = LLVMRunFunction(engine, main_function, 0, NULL);
    LLVMDisposeExecutionEngine(engine);
    return (int)LLVMGenericValueToInt(result, 0);
}
