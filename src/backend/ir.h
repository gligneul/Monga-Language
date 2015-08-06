/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ir.h
 */

#ifndef IR_H 
#define IR_H

#include <llvm-c/Core.h>

#include "ast/ast.h"

/* Compiles the LLVM IR module from the AST */
LLVMModuleRef IRCompileModule(AstDeclaration* tree);

#endif

