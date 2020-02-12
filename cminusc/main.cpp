// main entry for Cminus C
// provided by TAs of Compiler Principle

// Modified by bbn <clfbbn@gmail.com>

#include "builder.hpp"
#include "cxx_helper.hpp"
#include "syntax_tree.hpp"

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
                 "[--analyze] <input-file>"
              << std::endl;
}

int main(int argc, char **argv) {
    std::string output_path; // output file path
    std::string input_path;  // input file path

    bool emit = false;    // emit-llvm
    bool analyse = false; // analyse

    // save exec name, if print_help is needed, use this exec_name
    std::string exec_name = argv[0];

    DEBUG_PRINT_3("Exec name is " << exec_name << std::endl);

    // scan for flags
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            DEBUG_PRINT_3("-h or --help found" << std::endl);
            print_help(exec_name);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            DEBUG_PRINT_3("-o found, checking output file name" << std::endl);

            if (!(output_path.empty())) {
                // 2 -o directive
                print_help(exec_name);
                // TODO: should panic here
                return 0;
            }

            // get output file, should be at index `i + 1`
            if (i + 1 == argc) {
                // wrong
                print_help(exec_name);
                return 0;
            } else {
                // get output path
                output_path = argv[i + 1];
                i++; // skip the output path
                DEBUG_PRINT_3("output obj name is " << output_path
                                                    << std::endl);
            }
        } else if (strcmp(argv[i], "--emit-llvm") == 0) {
            DEBUG_PRINT_3("--emit-llvm found, setting flag." << std::endl);

            emit = true;
        } else if (strcmp(argv[i], "--analyse") == 0) {
            DEBUG_PRINT_3("--analyse found, setting flag." << std::endl);
        } else {
            if (input_path.empty()) {
                // no input path yet
                // TODO: multi files
                DEBUG_PRINT_3("found input path: " << argv[i] << std::endl);
                input_path = argv[i];
            } else {
                print_help(exec_name);
                return 0;
            }
        }

        // check if all done
        if (input_path.empty()) {
            DEBUG_PRINT(
                "no input file found, possible parsing command line error"
                << std::endl);
            DEBUG_PRINT("printing help info..." << std::endl);

            print_help(exec_name);
            return 0;
        }
    }

    return 0;
}