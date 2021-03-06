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
  DEBUG_PRINT_3("visiting program");

  is_main = 0;

  for (auto iter : node.declarations) {
    iter->accept(*this);
  }

  DEBUG_PRINT_3("leaving program");
}

void CminusBuilder::visit(syntax_num &node) {
  DEBUG_PRINT_3("visiting num");

  value_stack.push(llvm::ConstantInt::get(tInt32, llvm::APInt(32, node.value)));

  DEBUG_PRINT_3("leaving num");
}

void CminusBuilder::visit(syntax_var_declaration &node) {
  DEBUG_PRINT_3("visiting var declaration");

  if (node.array_def) {
    // visiting the `num` in array definition is not needed
    // node.num->accept(*this);
    // value_stack.pop();

    auto theArrayType = llvm::ArrayType::get(
        llvm::IntegerType::get(context, 32), node.num->value);

    if (scope.in_global()) {
      // global array declaration
      auto var = new llvm::GlobalVariable(*(module.get()), theArrayType, false,
                                          llvm::GlobalValue::ExternalLinkage,
                                          nullptr, node.id);

      // set initializer :
      // https://stackoverflow.com/questions/23330018/llvm-global-integer-array-zeroinitializer
      llvm::ConstantAggregateZero *constArray =
          llvm::ConstantAggregateZero::get(theArrayType);

      var->setInitializer(constArray);

      scope.push(node.id, var);
    } else {
      // local array declaration
      // FIXME: what is addrspace?
      auto var = new llvm::AllocaInst(theArrayType, 0, "", bb_now);

      scope.push(node.id, var);
    }
  } else {
    // not a array def
    if (scope.in_global()) {
      // global var declaration
      auto var = new llvm::GlobalVariable(*(module.get()), tInt32, false,
                                          llvm::GlobalVariable::ExternalLinkage,
                                          nullptr, node.id);
      auto zero_initializer = llvm::ConstantAggregateZero::get(tInt32);
      var->setInitializer(zero_initializer);

      scope.push(node.id, var);
    } else {
      // local var declaration
      auto var = builder.CreateAlloca(tInt32);

      scope.push(node.id, var);
    }
  }

  DEBUG_PRINT_3("leaving var declaration");
}

void CminusBuilder::visit(syntax_fun_declaration &node) {
  DEBUG_PRINT_3("visiting fun declaration");

  // param list
  size_t param_size = node.params.size(); // number of params
  std::vector<llvm::Type *> param_table;
  // fill in the table of type of params
  for (size_t i = 0; i < param_size; ++i) {
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
      llvm::GlobalValue::LinkageTypes::ExternalLinkage, node.id, module.get());
  scope.push(node.id, theFunction);
  scope.enter();
  func_now = theFunction;

  // set basic block
  auto theBB =
      llvm::BasicBlock::Create(context, node.id + "_start", theFunction);
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

  // this is important, create a ret if there is not one
  if (node.type == Type_int && bb_now->getTerminator() == nullptr) {
    builder.CreateRet(CONST(0));
  } else if (node.type == Type_void && bb_now->getTerminator() == nullptr) {
    builder.CreateRet(nullptr);
  }

  scope.exit();

  DEBUG_PRINT_3("leaving fun declaration");
}

void CminusBuilder::visit(syntax_param &node) {
  DEBUG_PRINT_3("visiting param");

  static int name_num = 0;
  name_num++;

  llvm::AllocaInst *theParam;
  if (node.isarray) {
    theParam = builder.CreateAlloca(tInt32->getPointerTo());
  } else {
    theParam = builder.CreateAlloca(tInt32);
  }
  builder.CreateStore(value_stack.pop(), theParam);

  theParam->setName("fun_param_" + std::to_string(name_num));

  scope.push(node.id, theParam);

  DEBUG_PRINT_3("leaving param");
}

void CminusBuilder::visit(syntax_compound_stmt &node) {
  DEBUG_PRINT_3("visiting compound stmt");

  scope.enter();

  for (auto iter : node.local_declarations) {
    iter->accept(*this);
  }

  for (auto iter : node.statement_list) {
    iter->accept(*this);
  }

  scope.exit();

  DEBUG_PRINT_3("leaving compound stmt");
}

void CminusBuilder::visit(syntax_expression_stmt &node) {
  DEBUG_PRINT_3("visiting expression stmt");

  if (node.expression != nullptr) {
    node.expression->accept(*this);
  }

  DEBUG_PRINT_3("leaving expression stmt");
}

void CminusBuilder::visit(syntax_selection_stmt &node) {
  // selection_stmt -> if ( expression ) statement | if ( expression )
  // statement else statement
  DEBUG_PRINT_3("visiting selection stmt");

  int all_ret = 0; // this checks terminator status
  static int selection_cnt =
      0; // static value, increase it each time the function is called

  selection_cnt++;

  // create basic block for true entry
  auto if_true_bb = llvm::BasicBlock::Create(
      context, "if_true_" + std::to_string(selection_cnt), func_now);

  // create basic block for false entry
  llvm::BasicBlock *if_else_bb;
  if (node.else_statement != nullptr) {
    if_else_bb = llvm::BasicBlock::Create(
        context, "if_else_" + std::to_string(selection_cnt), func_now);
  } else {
    if_else_bb = nullptr;
  }

  // create basic block for out
  auto if_out_bb = llvm::BasicBlock::Create(
      context, "if_out_" + std::to_string(selection_cnt), func_now);

  // all BBs are created, process
  node.expression->accept(*this);
  auto if_expr = value_stack.pop();

  if (node.else_statement != nullptr) {
    builder.CreateCondBr(if_expr, if_true_bb, if_else_bb);
  } else {
    builder.CreateCondBr(if_expr, if_true_bb, if_out_bb);
  }

  // true block
  builder.SetInsertPoint(if_true_bb);
  bb_now = if_true_bb; // update current basicblock
  node.if_statement->accept(*this);

  if (bb_now->getTerminator() == nullptr) {
    // no terminator in current BB, need a jump
    builder.CreateBr(if_out_bb);
  } else {
    all_ret += 1;
  }

  // false block
  if (node.else_statement != nullptr) {
    builder.SetInsertPoint(if_else_bb);
    bb_now = if_else_bb;
    node.else_statement->accept(*this);

    if (bb_now->getTerminator() == nullptr) {
      // no terminator
      builder.CreateBr(if_out_bb);
    } else {
      all_ret += 1;
    }
  }

  // out block
  if (node.else_statement != nullptr) {
    if (all_ret == 2) {
      if_out_bb->eraseFromParent();
    } else {
      builder.SetInsertPoint(if_out_bb);
      bb_now = if_out_bb;
    }
  } else {
    if (all_ret == 1) {
      if_out_bb->eraseFromParent();
    } else {
      builder.SetInsertPoint(if_out_bb);
      bb_now = if_out_bb;
    }
  }

  DEBUG_PRINT_3("leaving selection stmt");
}

void CminusBuilder::visit(syntax_iteration_stmt &node) {
  // iteration_stmt -> while ( expression ) statement
  DEBUG_PRINT_3("visiting iteration stmt");

  static int while_cnt = 0; // increase it each time a iteration_stmt is visited

  while_cnt++;

  // current function is stored in func_now
  auto while_cmp_bb = llvm::BasicBlock::Create(
      context, "while_cmp_" + std::to_string(while_cnt), func_now);
  auto while_loop_bb = llvm::BasicBlock::Create(
      context, "while_loop_" + std::to_string(while_cnt), func_now);
  auto while_out_bb = llvm::BasicBlock::Create(
      context, "while_out_" + std::to_string(while_cnt), func_now);

  // jump to the cmp basic block
  builder.CreateBr(
      while_cmp_bb); // FIXME: we may need to determine if there is already a
                     // `terminator` in current basicblock

  // first basicblock: compare
  builder.SetInsertPoint(while_cmp_bb);
  bb_now = while_cmp_bb;
  node.expression->accept(*this);
  auto while_expr = value_stack.pop();
  builder.CreateCondBr(while_expr, while_loop_bb, while_out_bb);

  // second basic block: while loop
  builder.SetInsertPoint(while_loop_bb);
  bb_now = while_loop_bb;
  node.statement->accept(*this);

  // we need to check if current bb already has a terminator
  if (bb_now->getTerminator()) {
    // has a terminator, we may not need the `out` bb
    while_out_bb->eraseFromParent();
  } else {
    // does not has a terminator
    builder.CreateBr(while_cmp_bb);
    builder.SetInsertPoint(while_out_bb);
    bb_now = while_out_bb;
  }

  DEBUG_PRINT_3("leaving iteration stmt");
}

void CminusBuilder::visit(syntax_return_stmt &node) {
  // return_stmt -> return; | return expression;
  DEBUG_PRINT_3("visiting return stmt");

  if (node.expression != nullptr) {
    node.expression->accept(*this);
    builder.CreateRet(value_stack.pop());
  } else {
    // return only, no expression
    builder.CreateRet(nullptr);
  }

  DEBUG_PRINT_3("leaving return stmt");
}

void CminusBuilder::visit(syntax_var &node) {
  // the var node gets the value of the variable
  DEBUG_PRINT_3("visiting var");

  auto val = scope.find(node.id);

  auto theType = val->getType()->getPointerElementType();

  if (theType->isArrayTy() || theType->isPointerTy()) {
    // id[ expression ], visit expression first
    DEBUG_PRINT_3("ARRAY!!!");
    llvm::Value *arr_expr;
    if (node.expression != nullptr) {
      node.expression->accept(*this);
      arr_expr = value_stack.pop();
    } else {
      arr_expr = CONST(0);
    }

    // get element, is it a parameter or a local define ?
    llvm::Value *tmp;
    llvm::Value *result;
    if (val->getName().find("fun_param") != std::string::npos) {
      tmp = builder.CreateLoad(val);
      tmp = llvm::GetElementPtrInst::CreateInBounds(tmp, arr_expr, "", bb_now);
    } else {
      tmp = llvm::GetElementPtrInst::CreateInBounds(val, {CONST(0), arr_expr},
                                                    "", bb_now);
    }

    if (node.expression != nullptr) {
      result = builder.CreateLoad(tmp);
    } else {
      result = tmp;
    }

    // use stack to pass the value
    value_stack.push(result);
  } else {
    // the pointer, or the var only
    auto load_val = builder.CreateLoad(val);

    // check if it is array
    value_stack.push(load_val); // FIXME: may be wrong, may be right.
  }

  DEBUG_PRINT_3("leaving var");
}

void CminusBuilder::visit(syntax_assign_expression &node) {
  DEBUG_PRINT_3("visiting assign expression");

  // var = expression
  node.expression->accept(*this);
  auto assign_expr_r = value_stack.pop();

  // visit var
  llvm::Value *var_l;
  if (node.var->expression) {
    llvm::Value *tmp;
    tmp = scope.find(node.var->id);

    // visit expr
    node.var->expression->accept(*this);
    auto var_expression = value_stack.pop();

    if (tmp->getName().find("fun_param") != std::string::npos) {
      DEBUG_PRINT_3("This is an Argument");
      tmp = builder.CreateLoad(tmp);
      var_l = llvm::GetElementPtrInst::CreateInBounds(tmp, var_expression, "",
                                                      bb_now);
    } else {
      var_l = llvm::GetElementPtrInst::CreateInBounds(
          tmp, {CONST(0), var_expression}, "", bb_now);
      DEBUG_PRINT_3("Not an argument");
    }
  } else {
    var_l = scope.find(node.var->id);
  }

  // store
  builder.CreateStore(assign_expr_r, var_l);

  DEBUG_PRINT_3("leaving assign expression");
}

void CminusBuilder::visit(syntax_simple_expression &node) {
  // simple_expression -> additive relop additive | additive
  DEBUG_PRINT_3("visiting simple expression");

  llvm::Value *result;
  if (node.additive_expression_r != nullptr) {
    node.additive_expression_l->accept(*this);
    node.additive_expression_r->accept(*this);

    auto r_expr = value_stack.pop();
    auto l_expr = value_stack.pop();

    // compare
    switch (node.op) {
    case Op_lte: // <=
      result = builder.CreateICmpSLE(l_expr, r_expr);
      break;
    case Op_lt: // <
      result = builder.CreateICmpSLT(l_expr, r_expr);
      break;
    case Op_gt: // >
      result = builder.CreateICmpSGT(l_expr, r_expr);
      break;
    case Op_gte: // >=
      result = builder.CreateICmpSGE(l_expr, r_expr);
      break;
    case Op_eq: // ==
      result = builder.CreateICmpEQ(l_expr, r_expr);
      break;
    case Op_neq: // !=
      result = builder.CreateICmpNE(l_expr, r_expr);
      break;
    default:
      ERROR("wrong op, never here" << std::endl);
      // no default available
    }
  } else {
    node.additive_expression_l->accept(*this);

    result = value_stack.pop();
  }

  value_stack.push(result);

  DEBUG_PRINT_3("leaving simple expression");
}

void CminusBuilder::visit(syntax_additive_expression &node) {
  DEBUG_PRINT_3("visiting additive expression");

  llvm::Value *result;
  if (node.additive_expression != nullptr) {
    node.additive_expression->accept(*this);
    auto add_expr = value_stack.pop();

    node.term->accept(*this);
    auto term = value_stack.pop();

    if (node.op == Op_add) { // +
      result = builder.CreateAdd(add_expr, term);
    } else {
      result = builder.CreateSub(add_expr, term);
    }
  } else {
    node.term->accept(*this);
    auto term = value_stack.pop();

    result = term;
  }

  value_stack.push(result);

  DEBUG_PRINT_3("leaving additive expression");
}

void CminusBuilder::visit(syntax_term &node) {
  // term -> term mulop factor | factor
  DEBUG_PRINT_3("visiting term");

  llvm::Value *result;

  if (node.term == nullptr) {
    node.factor->accept(*this);
    auto factor = value_stack.pop();

    result = factor;
  } else {
    node.factor->accept(*this);
    auto factor = value_stack.pop();

    node.term->accept(*this);
    auto term = value_stack.pop();

    if (node.op == Op_mul) {
      result = builder.CreateMul(term, factor);
    } else {
      result = builder.CreateSDiv(term, factor);
    }
  }

  value_stack.push(result);

  DEBUG_PRINT_3("leaving term");
}

void CminusBuilder::visit(syntax_call &node) {
  DEBUG_PRINT_3("visiting call");

  // setting args
  std::vector<llvm::Value *> call_args;

  for (auto iter : node.args) {
    iter->accept(*this);

    call_args.push_back(value_stack.pop());
  }

  // find function
  auto func_value = scope.find(node.id);
  auto ac_func = llvm::dyn_cast<llvm::Function>(func_value);

  // call
  auto call_ret = builder.CreateCall(llvm::FunctionCallee(ac_func), call_args);

  // pass value through stack
  value_stack.push(call_ret);

  DEBUG_PRINT_3("leaving call");
}

void CminusBuilder::visit(syntax_a_char &node) {}
