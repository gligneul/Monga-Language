/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * monga.c
 */

#include <stdbool.h>
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

/* Argument options */
bool generate_bytecode = false;
bool dump_module = false;
bool execute_module = true;

/* Parses then main arguments */
static void parseArguments(int argc, char* argv[]);

/* Prints the help message */
static void printHelpMessage();

/* Generates the LLVM bitcode */
static void exportModule(LLVMModuleRef module);

/* Executes a LLVM module */
static int executeModule(LLVMModuleRef module);

int main(int argc, char* argv[])
{
    parseArguments(argc, argv);

    yyparse();
    SemanticAnalyseTree(parser_ast);
    LLVMModuleRef module = IRCompileModule(parser_ast);

    if (generate_bytecode)
        exportModule(module);

    if (dump_module)
       LLVMDumpModule(module);

    int return_value = 0;
    if (execute_module)
        return_value = executeModule(module);

    return return_value;
}

static void parseArguments(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0)
            printHelpMessage();
        else if (strcmp(argv[i], "-bc") == 0)
            generate_bytecode = true;
        else if (strcmp(argv[i], "-dump") == 0)
            dump_module = true;
        else if (strcmp(argv[i], "-no-execution") == 0)
            execute_module = false;
        else
            Error("Unknown option: %s", argv[i]);
	}
}

static void printHelpMessage()
{
    printf(
    "Usage:\n"
    "    monga [options] < [input]\n"
    "\n"
    "Options:\n"
    "    -h             Shows this message\n"
    "    -bc            Exports the llvm bytecode file\n"
    "    -dump          Dumps the llvm module\n"
    "    -no-execution  Doesn't execute the monga program\n");
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

