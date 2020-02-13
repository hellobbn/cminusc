#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "syntax_tree.hpp"

#include <map>

// Scope is a class for saving and searching for symbols
class Scope {
  public:
    void enter() { inner.push_back({}); }

    void exit() { inner.pop_back(); }

    bool in_global() { return inner.size() == 1; }

    bool push(std::string name, llvm::Value *val) {
        auto result = inner[inner.size() - 1].insert({name, val});
        return result.second;
    }

    llvm::Value *find(std::string name) {
        for (auto s = inner.rbegin(); s != inner.rend(); s++) {
            auto iter = s->find(name);
            if (iter != s->end()) { // found
                return iter->second;
            }
        }
        return nullptr;
    }

  private:
    std::vector<std::map<std::string, llvm::Value *>> inner;
};

// tree visitor
class CminusBuilder : public syntax_tree_visitor {
  public:
    CminusBuilder() : builder(context) {
        module = std::make_unique<llvm::Module>("cminus", context);
        // auto TyVoid = llvm::Type::getVoidTy(context);
        // auto TyInt32 = llvm::Type::getInt32Ty(context);

        // built-in function: input
        // auto input_type = llvm::FunctionType::get(TyInt32, false);
        // auto input_fun = llvm::Function::Create(
        //     input_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        //     "input", module.get());

        // built-in function: output
        // std::vector<llvm::Type *> output_params;
        // output_params.push_back(TyInt32);
        // auto output_type = llvm::FunctionType::get(TyInt32, false);
        // auto output_fun = llvm::Function::Create(
        //     output_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        //     "output", module.get());

        // built-in function: neg_idx_except_fun(disabled)
        // empty

        // scope.enter();
        // scope.push("input", input_fun);
        // scope.push("output", output_fun);
        scope.enter();  // go global
    }

    std::unique_ptr<llvm::Module> build() { return std::move(module); }

  private:
    virtual void visit(syntax_program &) override final;
    virtual void visit(syntax_num &) override final;
    virtual void visit(syntax_var_declaration &) override final;
    virtual void visit(syntax_fun_declaration &) override final;
    virtual void visit(syntax_param &) override final;
    virtual void visit(syntax_compound_stmt &) override final;
    virtual void visit(syntax_expression_stmt &) override final;
    virtual void visit(syntax_selection_stmt &) override final;
    virtual void visit(syntax_iteration_stmt &) override final;
    virtual void visit(syntax_return_stmt &) override final;
    virtual void visit(syntax_assign_expression &) override final;
    virtual void visit(syntax_simple_expression &) override final;
    virtual void visit(syntax_additive_expression &) override final;
    virtual void visit(syntax_var &) override final;
    virtual void visit(syntax_term &) override final;
    virtual void visit(syntax_call &) override final;

    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    Scope scope;
    std::unique_ptr<llvm::Module> module;
};

#endif /* BUILDER_HPP */
