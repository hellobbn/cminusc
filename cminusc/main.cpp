// main entry for Cminus C
// provided by TAs of Compiler Principle

// Modified by bbn <clfbbn@gmail.com>

#include "builder.hpp"
#include "syntax_tree.hpp"
#include "cxx_helper.hpp"

#include <llvm/ADT/Triple.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/CodeGen/CommandFlags.inc>
#include <llvm/CodeGen/LinkAllCodegenComponents.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/IR/AutoUpgrade.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetMachine.h>

#include <iostream>

void print_help(std::string exe_name) {
    std::cout << "Usage: " << exe_name
              << "[ -h | --help ] [ -o <target-file> ] [ -emit-llvm ] "
                 "[-analyze] <input-file>"
              << std::endl;
}

int main(int argc, char** argv) {
    std::string output_path;    // output file path
    std::string input_path;     // input file path

    bool emit = false;  // emit-llvm
    bool analyse = false;   // analyse

    // save exec name, if print_help is needed, use this exec_name
    std::string exec_name = argv[0];

    DEBUG_PRINT_3("Exec name is " << exec_name << std::endl);

    // scan for flags
    for(int i = 1; i < argc; ++ i) {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            DEBUG_PRINT_3("-h or --help found" << std::endl);
            print_help(exec_name);
        }
    }

    return 0;
}