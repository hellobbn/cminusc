// main entry for Cminus C
// provided by TAs of Compiler Principle

// Modified by bbn <clfbbn@gmail.com>

#include "builder.hpp"
#include "cxx_helper.hpp"
#include "syntax_tree.hpp"

#include <cstdlib>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/CodeGen/CommandFlags.h>
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
#include <llvm/InitializePasses.h>
#include <llvm/Pass.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include <iostream>
#include <llvm/Target/TargetOptions.h>

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

  DEBUG_PRINT_3("Exec name is " << exec_name);

  // scan for flags
  for (int i = 1; i < argc; ++i) {
    DEBUG_PRINT_3("opt: \"" << argv[i] << "\"");
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      DEBUG_PRINT_3("-h or --help found");
      print_help(exec_name);
      return 0;
    } else if (strcmp(argv[i], "-o") == 0) {
      DEBUG_PRINT_3("-o found, checking output file name");

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
        DEBUG_PRINT_3("output obj name is " << output_path);
      }
    } else if (strcmp(argv[i], "--emit-llvm") == 0) {
      DEBUG_PRINT_3("--emit-llvm found, setting flag.");

      emit = true;
    } else if (strcmp(argv[i], "--analyse") == 0) {
      DEBUG_PRINT_3("--analyse found, setting flag.");

      analyse = true;
    } else {
      if (input_path.empty()) {
        // no input path yet
        // TODO: multi files
        DEBUG_PRINT_3("found input path: " << argv[i]);

        input_path = argv[i];
        // This is for Linux convention only
        if (input_path[0] != '.' && input_path[1] != '/') {
          input_path = "./" + input_path; // Linux ONLY
        }
      } else {
        print_help(exec_name);
        return 0;
      }
    }
  }

  // check if all done
  if (input_path.empty()) {
    DEBUG_PRINT("no input file found, possible parsing command line error");
    DEBUG_PRINT("printing help info...");

    print_help(exec_name);
    return 0;
  }

  // check target path
  if (output_path.empty()) {
    DEBUG_PRINT("no output path found found");
    DEBUG_PRINT("checking input file to see if it is parsed right");
    auto pos = input_path.rfind('.');

    if ((int)pos == -1) {
      // not found '.'
      ERROR("the input file parsed is \"" << input_path << "\". Possible wrong."
                                          << std::endl);
    } else {
      // found '.', check file extension
      if (input_path.substr(pos) != ".cminus") {
        ERROR("the input file \"" << input_path << "\" has wrong extension!"
                                  << std::endl);
      }

      // right extension
      if (emit) {
        output_path = input_path.substr(0, pos) + ".ll";
      } else {
        pos = input_path.rfind('/');
        output_path = input_path.substr(0, pos + 1) + "a.out";
      }

      DEBUG_PRINT_3("setting the output file to \"" << output_path << "\"");
    }
  }

  // print debug info
  DEBUG_PRINT("input file:  \"" << input_path << "\".");
  DEBUG_PRINT("output file: \"" << output_path << "\".");

  auto s = syn_parser(input_path.c_str());

  // if you need to see the syntax tree...
  // printSynTree(stdout, s);

  // convert it to a tree
  auto tree = syntax_tree(s);

  CminusBuilder builder;

  DEBUG_PRINT_1("");
  DEBUG_PRINT_1("-------------------------------------------------------");
  DEBUG_PRINT_1("visiting the transformed tree.");
  DEBUG_PRINT_1("-------------------------------------------------------");
  DEBUG_PRINT_1("");
  tree.run_visitor(builder);
  DEBUG_PRINT_1("");
  DEBUG_PRINT_1("-------------------------------------------------------");
  DEBUG_PRINT_1("Done visiting");
  DEBUG_PRINT_1("-------------------------------------------------------");
  DEBUG_PRINT_1("");

  auto mod = builder.build();
  DEBUG_PRINT_1("Builder build completed.");

  mod->setSourceFileName(input_path);
  DEBUG_PRINT("Done setting up source file name");

  // check
  // https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl08.html
  // TODO: check later
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    llvm::errs() << Error;
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";

  auto CPUName = llvm::sys::getHostCPUName();
  DEBUG_PRINT_2("CPU name: " << CPUName.str());
  llvm::StringMap<bool> CPUFeatures;
  if (!llvm::sys::getHostCPUFeatures(CPUFeatures)) {
    ERROR("CPU FEATURES QURTY FAILED.");
    exit(-1);
  }

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  auto TargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  mod->setDataLayout(TargetMachine->createDataLayout());
  mod->setTargetTriple(TargetTriple);

  std::error_code EC;

  llvm::legacy::PassManager pass;

  if (analyse) {
    llvm::errs() << "analyse is not supported yet\n";
    return 1;
  } else if (emit) {
    auto output_file = std::make_unique<llvm::ToolOutputFile>(
        output_path, EC, llvm::sys::fs::F_None);
    DEBUG_PRINT_3("file output to " << output_path);
    if (EC.value()) {
      llvm::errs() << EC.message() << "\n";
      return -1;
    }
    auto output_ostream = &output_file->os();
    mod->print(*output_ostream, nullptr);
    output_file->keep();

    return 0;
  } else {
    DEBUG_PRINT_1("Print to fie...\n");
    auto obj_file_name = output_path + ".o";
    llvm::raw_fd_ostream dest(obj_file_name, EC, llvm::sys::fs::OF_None);

    if (EC) {
      llvm::errs() << "cannot open file";
      return 1;
    }

    auto file_type = llvm::CGFT_ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
      llvm::errs() << "Target Machine can't emit a file of this type.";
      return 1;
    }

    pass.run(*mod);
    dest.flush();

    DEBUG_PRINT("object path for cminus-c library is set default to "
                "\"build/libcminusc.a\"");
    std::string stdlib_dir = "build";
    std::string stdlib_name = "libcminusc.a";
    std::string stdlib = "cminus";
    auto command = std::string("ld ") + output_path + ".o " +
                   "-L build -l cminusc -o " + output_path;

    DEBUG_PRINT("using command: " << command);

    std::system(command.c_str());
    // auto command_string =
    //     std::string("clang -w ") + output_path + ".o -o " + output_path;
    // std::system(command_string.c_str());

    // make it use ld for linking:
    // https://stackoverflow.com/questions/51677440/linking-llvm-produced-object-code-with-ld
  }

  return 0;
}