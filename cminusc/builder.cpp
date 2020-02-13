#include "builder.hpp"
#include "cxx_helper.hpp"

// global definitions here
#define tInt32 llvm::Type::getInt32Ty(context)
#define tVoid llvm::Type::getVoidTy(context)
llvm::Function *func_now; // current function scope
llvm::BasicBlock *bb_now; // current basic block

int is_main; // in main function or not

// stack definition here
class ValStack {
  public:
    void push(llvm::Value *val) { theStack.push_back(val); }

    llvm::Value *pop(void) {
        auto tmp = theStack.back();
        theStack.pop_back();
        return tmp;
    }

    llvm::Value *getTop(void) { return theStack.back(); }

    size_t getSize(void) { return theStack.size(); }

  private:
    std::vector<llvm::Value *> theStack;
};

ValStack value_stack;

// Some macro to make life easier
#define CONST(num) llvm::ConstantInt::get(context, llvm::APInt(32, num))
#define ArrayType(num) llvm::ArrayType::get(TyInt32, num)

void CminusBuilder::visit(syntax_program &node) {
    DEBUG_PRINT_2("visiting program");

    is_main = 0;

    for (auto iter : node.declarations) {
        iter->accept(*this);
    }

    DEBUG_PRINT_2("leaving program");
}

void CminusBuilder::visit(syntax_num &node) {
    DEBUG_PRINT_2("visiting num");

    value_stack.push(
        llvm::ConstantInt::get(tInt32, llvm::APInt(32, node.value)));

    DEBUG_PRINT_2("leaving num");
}

void CminusBuilder::visit(syntax_var_declaration &node) {
    DEBUG_PRINT_2("visiting var declaration");

    if (node.array_def) {
        // visiting the `num` in array definition is not needed
        // node.num->accept(*this);
        // value_stack.pop();

        auto theArrayType = llvm::ArrayType::get(
            llvm::IntegerType::get(context, 32), node.num->value);

        if (scope.in_global()) {
            // global array declaration
            auto var = new llvm::GlobalVariable(
                *(module.get()), theArrayType, false,
                llvm::GlobalValue::ExternalLinkage, nullptr, node.id);

            // set initializer :
            // https://stackoverflow.com/questions/23330018/llvm-global-integer-array-zeroinitializer
            llvm::ConstantAggregateZero *constArray =
                llvm::ConstantAggregateZero::get(theArrayType);

            var->setInitializer(constArray);

            scope.push(node.id, var);
        } else {
            // local array declaration
            // FIXME: what is addrspace?
            auto var = new llvm::AllocaInst(theArrayType, 0, node.id, bb_now);

            scope.push(node.id, var);
        }
    } else {
        // not a array def
        if (scope.in_global()) {
            // global var declaration
            auto var = new llvm::GlobalVariable(
                *(module.get()), tInt32, false,
                llvm::GlobalVariable::ExternalLinkage, nullptr, node.id);
            auto zero_initializer = llvm::ConstantAggregateZero::get(tInt32);
            var->setInitializer(zero_initializer);

            scope.push(node.id, var);
        } else {
            // local var declaration
            auto var = builder.CreateAlloca(tInt32);

            scope.push(node.id, var);
        }
    }

    DEBUG_PRINT("leaving var declaration");
}

void CminusBuilder::visit(syntax_fun_declaration &node) {
    DEBUG_PRINT_2("visiting fun declaration");

    // param list
    size_t param_size = node.params.size(); // number of params
    std::vector<llvm::Type *> param_table;
    // fill in the table of type of params
    for (int i = 0; i < param_size; ++i) {
        auto tmp = node.params[i];
        if (tmp->isarray) {
            param_table.push_back(tInt32->getPointerTo());
        } else {
            param_table.push_back(tInt32);
        }
    }

    // function return type
    llvm::Type *fun_type;
    if (node.type == Type_int) { // return int
        fun_type = llvm::Type::getInt32Ty(context);
    } else { // return void
        fun_type = llvm::Type::getVoidTy(context);
    }

    // create function
    auto theFunction = llvm::Function::Create(
        llvm::FunctionType::get(fun_type, param_table, false),
        llvm::GlobalValue::LinkageTypes::ExternalLinkage, node.id,
        module.get());
    scope.push(node.id, theFunction);
    scope.enter();
    func_now = theFunction;

    // set basic block
    auto theBB = llvm::BasicBlock::Create(context, node.id + "_start", theFunction);
    builder.SetInsertPoint(theBB);
    bb_now = theBB;

    // set up arg
    std::vector<llvm::Value *> args;
    int i = 0;
    for (auto arg = theFunction->arg_begin(); arg != theFunction->arg_end();
         arg++) {
        args.push_back(arg);

        value_stack.push(arg);
        node.params[i]->accept(*this); // this makes the param into scope
        i++;
    }

    node.compound_stmt->accept(*this);

    scope.exit();

    DEBUG_PRINT_2("leaving fun declaration");
}

void CminusBuilder::visit(syntax_param &node) {
    DEBUG_PRINT_2("visiting param");

    llvm::AllocaInst* theParam;
    if(node.isarray) {
        theParam = builder.CreateAlloca(tInt32->getPointerTo());
    } else {
        theParam = builder.CreateAlloca(tInt32);
    }
    builder.CreateStore(value_stack.pop(), theParam);
    scope.push(node.id, theParam);

    DEBUG_PRINT_2("leaving param");
}

void CminusBuilder::visit(syntax_compound_stmt &node) {
    DEBUG_PRINT_2("visiting compound stmt");

    scope.enter();

    for(auto iter : node.local_declarations) {
        iter->accept(*this);
    }

    for(auto iter : node.statement_list) {
        iter->accept(*this);
    }

    DEBUG_PRINT_2("leaving compound stmt");
}

void CminusBuilder::visit(syntax_expression_stmt &node) {}

void CminusBuilder::visit(syntax_selection_stmt &node) {}

void CminusBuilder::visit(syntax_iteration_stmt &node) {}

void CminusBuilder::visit(syntax_return_stmt &node) {}

void CminusBuilder::visit(syntax_var &node) {}

void CminusBuilder::visit(syntax_assign_expression &node) {}

void CminusBuilder::visit(syntax_simple_expression &node) {}

void CminusBuilder::visit(syntax_additive_expression &node) {}

void CminusBuilder::visit(syntax_term &node) {}

void CminusBuilder::visit(syntax_call &node) {}