#include "IRBuilder.h"
#include <iostream>
#include <algorithm>

// Forward declarations for helper functions
std::string getArrayIndicesFromFlat(int flatIndex, const std::vector<int>& dimensions);
void emitArrayInitializerHelper(IRGenerator& ir, const std::vector<int>& values,
                                const std::vector<int>& dimensions, int depth, int& index);

void IRBuilder::addBuiltInFunctions() {
    // Declare sylib functions in IR
    ir.emitHeader("declare i32 @getint()\n");
    ir.emitHeader("declare i32 @getch()\n");
    ir.emitHeader("declare i32 @getarray(i32*)\n");
    ir.emitHeader("declare void @putint(i32)\n");
    ir.emitHeader("declare void @putch(i32)\n");
    ir.emitHeader("declare void @putarray(i32, i32*)\n");
    ir.emitHeader("declare void @putf(i8*, ...)\n");
    ir.emitHeader("declare void @starttime()\n");
    ir.emitHeader("declare void @stoptime()\n");
    ir.emitHeader("\n");
    
    // Add to symbol table
    auto intType = std::make_shared<IntType>();
    auto voidType = std::make_shared<VoidType>();
    auto intPtrType = std::make_shared<PointerType>(intType);
    
    symbolTable.addSymbol("getint", std::make_shared<Symbol>("getint", 
        std::make_shared<FunctionType>(intType, std::vector<std::shared_ptr<Type>>{})));
    symbolTable.addSymbol("getch", std::make_shared<Symbol>("getch",
        std::make_shared<FunctionType>(intType, std::vector<std::shared_ptr<Type>>{})));
    symbolTable.addSymbol("getarray", std::make_shared<Symbol>("getarray",
        std::make_shared<FunctionType>(intType, std::vector<std::shared_ptr<Type>>{intPtrType})));
    symbolTable.addSymbol("putint", std::make_shared<Symbol>("putint",
        std::make_shared<FunctionType>(voidType, std::vector<std::shared_ptr<Type>>{intType})));
    symbolTable.addSymbol("putch", std::make_shared<Symbol>("putch",
        std::make_shared<FunctionType>(voidType, std::vector<std::shared_ptr<Type>>{intType})));
    symbolTable.addSymbol("putarray", std::make_shared<Symbol>("putarray",
        std::make_shared<FunctionType>(voidType, std::vector<std::shared_ptr<Type>>{intType, intPtrType})));
    symbolTable.addSymbol("starttime", std::make_shared<Symbol>("starttime",
        std::make_shared<FunctionType>(voidType, std::vector<std::shared_ptr<Type>>{})));
    symbolTable.addSymbol("stoptime", std::make_shared<Symbol>("stoptime",
        std::make_shared<FunctionType>(voidType, std::vector<std::shared_ptr<Type>>{})));
}

std::any IRBuilder::visitCompUnit(SysYParser::CompUnitContext *ctx) {
    for (auto item : ctx->children) {
        if (auto decl = dynamic_cast<SysYParser::DeclContext*>(item)) {
            visit(decl);
        } else if (auto funcDef = dynamic_cast<SysYParser::FuncDefContext*>(item)) {
            visit(funcDef);
        }
    }
    return nullptr;
}

std::any IRBuilder::visitDecl(SysYParser::DeclContext *ctx) {
    if (ctx->constDecl()) {
        return visit(ctx->constDecl());
    } else {
        return visit(ctx->varDecl());
    }
}

std::any IRBuilder::visitConstDecl(SysYParser::ConstDeclContext *ctx) {
    for (auto constDef : ctx->constDef()) {
        visit(constDef);
    }
    return nullptr;
}

int IRBuilder::evaluateConstExp(SysYParser::ConstExpContext *ctx) {
    auto result = visit(ctx->addExp());
    Value val = std::any_cast<Value>(result);
    if (val.isConst) {
        return val.constValue;
    }
    return 0;
}

std::vector<int> IRBuilder::evaluateConstInitVal(SysYParser::ConstInitValContext *ctx,
                                                  const std::vector<int>& dimensions, int depth) {
    std::vector<int> result;
    
    if (ctx->constExp()) {
        result.push_back(evaluateConstExp(ctx->constExp()));
    } else {
        if (ctx->constInitVal().empty()) {
            int size = 1;
            for (size_t i = depth; i < dimensions.size(); ++i) {
                size *= dimensions[i];
            }
            result.resize(size, 0);
        } else {
            for (auto initVal : ctx->constInitVal()) {
                auto subResult = evaluateConstInitVal(initVal, dimensions, depth + 1);
                result.insert(result.end(), subResult.begin(), subResult.end());
            }
            int expectedSize = 1;
            for (size_t i = depth; i < dimensions.size(); ++i) {
                expectedSize *= dimensions[i];
            }
            if (result.size() < expectedSize) {
                result.resize(expectedSize, 0);
            }
        }
    }
    
    return result;
}

void emitArrayInitializerHelper(IRGenerator& ir, const std::vector<int>& values,
                                const std::vector<int>& dimensions, int depth, int& index) {
    if (depth == dimensions.size() - 1) {
        ir.emitHeader("[");
        for (int i = 0; i < dimensions[depth]; ++i) {
            if (i > 0) ir.emitHeader(", ");
            ir.emitHeader("i32 " + std::to_string(values[index++]));
        }
        ir.emitHeader("]");
    } else {
        ir.emitHeader("[");
        for (int i = 0; i < dimensions[depth]; ++i) {
            if (i > 0) ir.emitHeader(", ");
            std::string subType = "i32";
            for (int j = dimensions.size() - 1; j > depth; --j) {
                subType = "[" + std::to_string(dimensions[j]) + " x " + subType + "]";
            }
            ir.emitHeader(subType + " ");
            emitArrayInitializerHelper(ir, values, dimensions, depth + 1, index);
        }
        ir.emitHeader("]");
    }
}

std::any IRBuilder::visitConstDef(SysYParser::ConstDefContext *ctx) {
    std::string name = ctx->IDENT()->getText();
    std::vector<int> dimensions;
    
    for (auto constExp : ctx->constExp()) {
        int dim = evaluateConstExp(constExp);
        dimensions.push_back(dim);
    }
    
    std::shared_ptr<Type> type;
    if (dimensions.empty()) {
        type = std::make_shared<IntType>();
    } else {
        type = std::make_shared<ArrayType>(std::make_shared<IntType>(), dimensions);
    }
    
    auto symbol = std::make_shared<Symbol>(name, type, true);
    
    if (dimensions.empty()) {
        symbol->intValue = evaluateConstExp(ctx->constInitVal()->constExp());
    } else {
        symbol->arrayValues = evaluateConstInitVal(ctx->constInitVal(), dimensions, 0);
    }
    
    symbolTable.addSymbol(name, symbol);
    
    if (symbolTable.isGlobalScope()) {
        std::string globalName = "@" + name;
        varMap[name] = globalName;
        
        if (dimensions.empty()) {
            ir.emitHeader(globalName + " = dso_local constant i32 " + 
                         std::to_string(symbol->intValue) + "\n");
        } else {
            ir.emitHeader(globalName + " = dso_local constant " + type->toString() + " ");
            int index = 0;
            emitArrayInitializerHelper(ir, symbol->arrayValues, dimensions, 0, index);
            ir.emitHeader("\n");
        }
    }
    
    return nullptr;
}

std::any IRBuilder::visitVarDecl(SysYParser::VarDeclContext *ctx) {
    for (auto varDef : ctx->varDef()) {
        visit(varDef);
    }
    return nullptr;
}

std::string getArrayIndicesFromFlat(int flatIndex, const std::vector<int>& dimensions) {
    std::string result;
    for (size_t i = 0; i < dimensions.size(); ++i) {
        int stride = 1;
        for (size_t j = i + 1; j < dimensions.size(); ++j) {
            stride *= dimensions[j];
        }
        int index = flatIndex / stride;
        flatIndex %= stride;
        result += ", i32 " + std::to_string(index);
    }
    return result;
}

std::vector<int> IRBuilder::evaluateInitVal(SysYParser::InitValContext *ctx,
                                            const std::vector<int>& dimensions, int depth) {
    std::vector<int> result;
    
    if (ctx->exp()) {
        auto val = std::any_cast<Value>(visit(ctx->exp()));
        if (val.isConst) {
            result.push_back(val.constValue);
        } else {
            result.push_back(0);
        }
    } else {
        if (ctx->initVal().empty()) {
            int size = 1;
            for (size_t i = depth; i < dimensions.size(); ++i) {
                size *= dimensions[i];
            }
            result.resize(size, 0);
        } else {
            for (auto initVal : ctx->initVal()) {
                auto subResult = evaluateInitVal(initVal, dimensions, depth + 1);
                result.insert(result.end(), subResult.begin(), subResult.end());
            }
            int expectedSize = 1;
            for (size_t i = depth; i < dimensions.size(); ++i) {
                expectedSize *= dimensions[i];
            }
            if (result.size() < expectedSize) {
                result.resize(expectedSize, 0);
            }
        }
    }
    
    return result;
}

std::any IRBuilder::visitVarDef(SysYParser::VarDefContext *ctx) {
    std::string name = ctx->IDENT()->getText();
    std::vector<int> dimensions;
    
    for (auto constExp : ctx->constExp()) {
        int dim = evaluateConstExp(constExp);
        dimensions.push_back(dim);
    }
    
    std::shared_ptr<Type> type;
    if (dimensions.empty()) {
        type = std::make_shared<IntType>();
    } else {
        type = std::make_shared<ArrayType>(std::make_shared<IntType>(), dimensions);
    }
    
    auto symbol = std::make_shared<Symbol>(name, type, false);
    symbolTable.addSymbol(name, symbol);
    
    if (symbolTable.isGlobalScope()) {
        std::string globalName = "@" + name;
        varMap[name] = globalName;
        
        if (ctx->initVal()) {
            if (dimensions.empty()) {
                auto val = std::any_cast<Value>(visit(ctx->initVal()));
                if (val.isConst) {
                    ir.emitHeader(globalName + " = dso_local global i32 " + 
                                 std::to_string(val.constValue) + "\n");
                } else {
                    ir.emitHeader(globalName + " = dso_local global i32 0\n");
                }
            } else {
                auto values = evaluateInitVal(ctx->initVal(), dimensions, 0);
                ir.emitHeader(globalName + " = dso_local global " + type->toString() + " ");
                int index = 0;
                emitArrayInitializerHelper(ir, values, dimensions, 0, index);
                ir.emitHeader("\n");
            }
        } else {
            if (dimensions.empty()) {
                ir.emitHeader(globalName + " = dso_local global i32 0\n");
            } else {
                ir.emitHeader(globalName + " = dso_local global " + type->toString() + " zeroinitializer\n");
            }
        }
    } else {
        std::string reg = ir.getNewTemp();
        varMap[name] = reg;
        
        if (dimensions.empty()) {
            ir.emit("  " + reg + " = alloca i32\n");
            if (ctx->initVal()) {
                auto val = std::any_cast<Value>(visit(ctx->initVal()));
                ir.emit("  store i32 " + val.reg + ", i32* " + reg + "\n");
            }
        } else {
            ir.emit("  " + reg + " = alloca " + type->toString() + "\n");
            if (ctx->initVal()) {
                auto values = evaluateInitVal(ctx->initVal(), dimensions, 0);
                for (size_t i = 0; i < values.size(); ++i) {
                    std::string ptrReg = ir.getNewTemp();
                    std::string indices = getArrayIndicesFromFlat(i, dimensions);
                    ir.emit("  " + ptrReg + " = getelementptr " + type->toString() + 
                           ", " + type->toString() + "* " + reg + ", i32 0" + indices + "\n");
                    ir.emit("  store i32 " + std::to_string(values[i]) + ", i32* " + ptrReg + "\n");
                }
            }
        }
    }
    
    return nullptr;
}

std::any IRBuilder::visitConstInitVal(SysYParser::ConstInitValContext *ctx) {
    return nullptr;
}

std::any IRBuilder::visitInitVal(SysYParser::InitValContext *ctx) {
    if (ctx->exp()) {
        return visit(ctx->exp());
    }
    return nullptr;
}

std::any IRBuilder::visitFuncDef(SysYParser::FuncDefContext *ctx) {
    std::string funcName = ctx->IDENT()->getText();
    std::string retTypeStr = ctx->funcType()->INT() ? "i32" : "void";
    currentFuncReturnType = retTypeStr;
    
    std::vector<std::pair<std::string, std::shared_ptr<Type>>> params;
    if (ctx->funcFParams()) {
        for (auto param : ctx->funcFParams()->funcFParam()) {
            std::string paramName = param->IDENT()->getText();
            std::shared_ptr<Type> paramType = std::make_shared<IntType>();
            params.push_back({paramName, paramType});
        }
    }
    
    ir.emit("define dso_local " + retTypeStr + " @" + funcName + "(");
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) ir.emit(", ");
        ir.emit("i32 %" + params[i].first + ".param");
    }
    ir.emit(") {\n");
    ir.emit("entry:\n");
    
    symbolTable.enterScope();
    
    for (const auto& param : params) {
        std::string reg = ir.getNewTemp();
        varMap[param.first] = reg;
        ir.emit("  " + reg + " = alloca i32\n");
        ir.emit("  store i32 %" + param.first + ".param, i32* " + reg + "\n");
        
        auto symbol = std::make_shared<Symbol>(param.first, param.second);
        symbolTable.addSymbol(param.first, symbol);
    }
    
    visit(ctx->block());
    
    if (retTypeStr == "void") {
        ir.emit("  ret void\n");
    } else {
        ir.emit("  ret i32 0\n");
    }
    
    ir.emit("}\n\n");
    
    symbolTable.exitScope();
    return nullptr;
}

std::any IRBuilder::visitBlock(SysYParser::BlockContext *ctx) {
    symbolTable.enterScope();
    for (auto item : ctx->blockItem()) {
        visit(item);
    }
    symbolTable.exitScope();
    return nullptr;
}

std::any IRBuilder::visitAssignStmt(SysYParser::AssignStmtContext *ctx) {
    auto lval = ctx->lVal();
    std::string varName = lval->IDENT()->getText();
    
    auto symbol = symbolTable.lookup(varName);
    if (!symbol) {
        std::cerr << "Undefined variable: " << varName << std::endl;
        return nullptr;
    }
    
    auto expVal = std::any_cast<Value>(visit(ctx->exp()));
    std::string varReg = varMap[varName];
    ir.emit("  store i32 " + expVal.reg + ", i32* " + varReg + "\n");
    
    return nullptr;
}

std::any IRBuilder::visitExpStmt(SysYParser::ExpStmtContext *ctx) {
    if (ctx->exp()) {
        visit(ctx->exp());
    }
    return nullptr;
}

std::any IRBuilder::visitBlockStmt(SysYParser::BlockStmtContext *ctx) {
    return visit(ctx->block());
}

std::any IRBuilder::visitIfStmt(SysYParser::IfStmtContext *ctx) {
    int thenLabel = ir.getNewLabel();
    int elseLabel = ctx->ELSE() ? ir.getNewLabel() : -1;
    int endLabel = ir.getNewLabel();
    
    auto condVal = std::any_cast<Value>(visit(ctx->cond()));
    std::string condBool = ir.getNewTemp();
    ir.emit("  " + condBool + " = icmp ne i32 " + condVal.reg + ", 0\n");
    
    if (elseLabel != -1) {
        ir.emit("  br i1 " + condBool + ", label %label" + std::to_string(thenLabel) +
               ", label %label" + std::to_string(elseLabel) + "\n");
    } else {
        ir.emit("  br i1 " + condBool + ", label %label" + std::to_string(thenLabel) +
               ", label %label" + std::to_string(endLabel) + "\n");
    }
    
    ir.emitLabel(thenLabel);
    visit(ctx->stmt(0));
    ir.emit("  br label %label" + std::to_string(endLabel) + "\n");
    
    if (elseLabel != -1) {
        ir.emitLabel(elseLabel);
        visit(ctx->stmt(1));
        ir.emit("  br label %label" + std::to_string(endLabel) + "\n");
    }
    
    ir.emitLabel(endLabel);
    return nullptr;
}

std::any IRBuilder::visitWhileStmt(SysYParser::WhileStmtContext *ctx) {
    int condLabel = ir.getNewLabel();
    int bodyLabel = ir.getNewLabel();
    int endLabel = ir.getNewLabel();
    
    ir.emit("  br label %label" + std::to_string(condLabel) + "\n");
    ir.emitLabel(condLabel);
    
    auto condVal = std::any_cast<Value>(visit(ctx->cond()));
    std::string condBool = ir.getNewTemp();
    ir.emit("  " + condBool + " = icmp ne i32 " + condVal.reg + ", 0\n");
    ir.emit("  br i1 " + condBool + ", label %label" + std::to_string(bodyLabel) +
           ", label %label" + std::to_string(endLabel) + "\n");
    
    ir.emitLabel(bodyLabel);
    ir.pushBreakLabel(endLabel);
    ir.pushContinueLabel(condLabel);
    
    visit(ctx->stmt());
    
    ir.popBreakLabel();
    ir.popContinueLabel();
    ir.emit("  br label %label" + std::to_string(condLabel) + "\n");
    
    ir.emitLabel(endLabel);
    return nullptr;
}

std::any IRBuilder::visitBreakStmt(SysYParser::BreakStmtContext *ctx) {
    int breakLabel = ir.getBreakLabel();
    if (breakLabel != -1) {
        ir.emit("  br label %label" + std::to_string(breakLabel) + "\n");
    }
    return nullptr;
}

std::any IRBuilder::visitContinueStmt(SysYParser::ContinueStmtContext *ctx) {
    int continueLabel = ir.getContinueLabel();
    if (continueLabel != -1) {
        ir.emit("  br label %label" + std::to_string(continueLabel) + "\n");
    }
    return nullptr;
}

std::any IRBuilder::visitReturnStmt(SysYParser::ReturnStmtContext *ctx) {
    if (ctx->exp()) {
        auto val = std::any_cast<Value>(visit(ctx->exp()));
        ir.emit("  ret i32 " + val.reg + "\n");
    } else {
        ir.emit("  ret void\n");
    }
    return nullptr;
}

std::any IRBuilder::visitExp(SysYParser::ExpContext *ctx) {
    return visit(ctx->addExp());
}

std::any IRBuilder::visitCond(SysYParser::CondContext *ctx) {
    return visit(ctx->lOrExp());
}

std::any IRBuilder::visitLVal(SysYParser::LValContext *ctx) {
    std::string varName = ctx->IDENT()->getText();
    auto symbol = symbolTable.lookup(varName);
    
    if (!symbol) {
        return Value();
    }
    
    if (symbol->isConst && ctx->exp().empty()) {
        Value val;
        val.isConst = true;
        val.constValue = symbol->intValue;
        val.reg = std::to_string(symbol->intValue);
        val.type = symbol->type;
        return val;
    }
    
    std::string varReg = varMap[varName];
    std::string loadReg = ir.getNewTemp();
    ir.emit("  " + loadReg + " = load i32, i32* " + varReg + "\n");
    
    Value val(loadReg, std::make_shared<IntType>());
    return val;
}

std::any IRBuilder::visitPrimaryExp(SysYParser::PrimaryExpContext *ctx) {
    if (ctx->exp()) {
        return visit(ctx->exp());
    } else if (ctx->lVal()) {
        return visit(ctx->lVal());
    } else {
        return visit(ctx->number());
    }
}

std::any IRBuilder::visitNumber(SysYParser::NumberContext *ctx) {
    std::string text = ctx->INTEGER_CONST()->getText();
    int value;
    
    if (text.size() > 2 && (text[1] == 'x' || text[1] == 'X')) {
        value = std::stoi(text, nullptr, 16);
    } else if (text[0] == '0' && text.size() > 1) {
        value = std::stoi(text, nullptr, 8);
    } else {
        value = std::stoi(text);
    }
    
    Value val;
    val.reg = std::to_string(value);
    val.type = std::make_shared<IntType>();
    val.isConst = true;
    val.constValue = value;
    return val;
}

std::any IRBuilder::visitPrimaryUnaryExp(SysYParser::PrimaryUnaryExpContext *ctx) {
    return visit(ctx->primaryExp());
}

std::any IRBuilder::visitFuncCallExp(SysYParser::FuncCallExpContext *ctx) {
    std::string funcName = ctx->IDENT()->getText();
    auto symbol = symbolTable.lookup(funcName);
    
    if (!symbol || !symbol->type->isFunction()) {
        return Value();
    }
    
    auto funcType = std::static_pointer_cast<FunctionType>(symbol->type);
    
    std::vector<Value> args;
    if (ctx->funcRParams()) {
        for (auto exp : ctx->funcRParams()->exp()) {
            args.push_back(std::any_cast<Value>(visit(exp)));
        }
    }
    
    std::string callInstr = "call " + funcType->returnType->toString() + " @" + funcName + "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) callInstr += ", ";
        callInstr += "i32 " + args[i].reg;
    }
    callInstr += ")";
    
    if (funcType->returnType->isVoid()) {
        ir.emit("  " + callInstr + "\n");
        return Value();
    } else {
        std::string resultReg = ir.getNewTemp();
        ir.emit("  " + resultReg + " = " + callInstr + "\n");
        return Value(resultReg, funcType->returnType);
    }
}

std::any IRBuilder::visitUnaryOpExp(SysYParser::UnaryOpExpContext *ctx) {
    auto operand = std::any_cast<Value>(visit(ctx->unaryExp()));
    std::string op = ctx->unaryOp()->getText();
    
    if (op == "+") {
        return operand;
    } else if (op == "-") {
        if (operand.isConst) {
            Value result;
            result.isConst = true;
            result.constValue = -operand.constValue;
            result.reg = std::to_string(result.constValue);
            result.type = std::make_shared<IntType>();
            return result;
        }
        
        std::string resultReg = ir.getNewTemp();
        ir.emit("  " + resultReg + " = sub i32 0, " + operand.reg + "\n");
        return Value(resultReg, std::make_shared<IntType>());
    } else if (op == "!") {
        if (operand.isConst) {
            Value result;
            result.isConst = true;
            result.constValue = !operand.constValue;
            result.reg = std::to_string(result.constValue);
            result.type = std::make_shared<IntType>();
            return result;
        }
        
        std::string cmpReg = ir.getNewTemp();
        ir.emit("  " + cmpReg + " = icmp eq i32 " + operand.reg + ", 0\n");
        std::string resultReg = ir.getNewTemp();
        ir.emit("  " + resultReg + " = zext i1 " + cmpReg + " to i32\n");
        return Value(resultReg, std::make_shared<IntType>());
    }
    
    return Value();
}

std::any IRBuilder::visitUnaryMulExp(SysYParser::UnaryMulExpContext *ctx) {
    return visit(ctx->unaryExp());
}

std::any IRBuilder::visitMulDivModExp(SysYParser::MulDivModExpContext *ctx) {
    auto left = std::any_cast<Value>(visit(ctx->mulExp()));
    auto right = std::any_cast<Value>(visit(ctx->unaryExp()));
    
    std::string op = ctx->children[1]->getText();
    
    if (left.isConst && right.isConst) {
        Value result;
        result.isConst = true;
        result.type = std::make_shared<IntType>();
        if (op == "*") {
            result.constValue = left.constValue * right.constValue;
        } else if (op == "/") {
            result.constValue = left.constValue / right.constValue;
        } else {
            result.constValue = left.constValue % right.constValue;
        }
        result.reg = std::to_string(result.constValue);
        return result;
    }
    
    std::string resultReg = ir.getNewTemp();
    std::string irOp;
    if (op == "*") {
        irOp = "mul";
    } else if (op == "/") {
        irOp = "sdiv";
    } else {
        irOp = "srem";
    }
    
    ir.emit("  " + resultReg + " = " + irOp + " i32 " + left.reg + ", " + right.reg + "\n");
    return Value(resultReg, std::make_shared<IntType>());
}

std::any IRBuilder::visitMulAddExp(SysYParser::MulAddExpContext *ctx) {
    return visit(ctx->mulExp());
}

std::any IRBuilder::visitAddSubExp(SysYParser::AddSubExpContext *ctx) {
    auto left = std::any_cast<Value>(visit(ctx->addExp()));
    auto right = std::any_cast<Value>(visit(ctx->mulExp()));
    
    std::string op = ctx->children[1]->getText();
    
    if (left.isConst && right.isConst) {
        Value result;
        result.isConst = true;
        result.type = std::make_shared<IntType>();
        if (op == "+") {
            result.constValue = left.constValue + right.constValue;
        } else {
            result.constValue = left.constValue - right.constValue;
        }
        result.reg = std::to_string(result.constValue);
        return result;
    }
    
    std::string resultReg = ir.getNewTemp();
    std::string irOp = (op == "+") ? "add" : "sub";
    ir.emit("  " + resultReg + " = " + irOp + " i32 " + left.reg + ", " + right.reg + "\n");
    return Value(resultReg, std::make_shared<IntType>());
}

std::any IRBuilder::visitAddRelExp(SysYParser::AddRelExpContext *ctx) {
    return visit(ctx->addExp());
}

std::any IRBuilder::visitRelOpExp(SysYParser::RelOpExpContext *ctx) {
    auto left = std::any_cast<Value>(visit(ctx->relExp()));
    auto right = std::any_cast<Value>(visit(ctx->addExp()));
    
    std::string op = ctx->children[1]->getText();
    
    std::string cmpOp;
    if (op == "<") cmpOp = "slt";
    else if (op == ">") cmpOp = "sgt";
    else if (op == "<=") cmpOp = "sle";
    else cmpOp = "sge";
    
    std::string cmpReg = ir.getNewTemp();
    ir.emit("  " + cmpReg + " = icmp " + cmpOp + " i32 " + left.reg + ", " + right.reg + "\n");
    
    std::string resultReg = ir.getNewTemp();
    ir.emit("  " + resultReg + " = zext i1 " + cmpReg + " to i32\n");
    
    return Value(resultReg, std::make_shared<IntType>());
}

std::any IRBuilder::visitRelEqExp(SysYParser::RelEqExpContext *ctx) {
    return visit(ctx->relExp());
}

std::any IRBuilder::visitEqNeExp(SysYParser::EqNeExpContext *ctx) {
    auto left = std::any_cast<Value>(visit(ctx->eqExp()));
    auto right = std::any_cast<Value>(visit(ctx->relExp()));
    
    std::string op = ctx->children[1]->getText();
    std::string cmpOp = (op == "==") ? "eq" : "ne";
    
    std::string cmpReg = ir.getNewTemp();
    ir.emit("  " + cmpReg + " = icmp " + cmpOp + " i32 " + left.reg + ", " + right.reg + "\n");
    
    std::string resultReg = ir.getNewTemp();
    ir.emit("  " + resultReg + " = zext i1 " + cmpReg + " to i32\n");
    
    return Value(resultReg, std::make_shared<IntType>());
}

std::any IRBuilder::visitEqLAndExp(SysYParser::EqLAndExpContext *ctx) {
    return visit(ctx->eqExp());
}

std::any IRBuilder::visitAndExp(SysYParser::AndExpContext *ctx) {
    auto left = std::any_cast<Value>(visit(ctx->lAndExp()));
    auto right = std::any_cast<Value>(visit(ctx->eqExp()));
    
    std::string leftBool = ir.getNewTemp();
    ir.emit("  " + leftBool + " = icmp ne i32 " + left.reg + ", 0\n");
    std::string rightBool = ir.getNewTemp();
    ir.emit("  " + rightBool + " = icmp ne i32 " + right.reg + ", 0\n");
    
    std::string andReg = ir.getNewTemp();
    ir.emit("  " + andReg + " = and i1 " + leftBool + ", " + rightBool + "\n");
    
    std::string resultReg = ir.getNewTemp();
    ir.emit("  " + resultReg + " = zext i1 " + andReg + " to i32\n");
    
    return Value(resultReg, std::make_shared<IntType>());
}

std::any IRBuilder::visitLAndLOrExp(SysYParser::LAndLOrExpContext *ctx) {
    return visit(ctx->lAndExp());
}

std::any IRBuilder::visitOrExp(SysYParser::OrExpContext *ctx) {
    auto left = std::any_cast<Value>(visit(ctx->lOrExp()));
    auto right = std::any_cast<Value>(visit(ctx->lAndExp()));
    
    std::string leftBool = ir.getNewTemp();
    ir.emit("  " + leftBool + " = icmp ne i32 " + left.reg + ", 0\n");
    std::string rightBool = ir.getNewTemp();
    ir.emit("  " + rightBool + " = icmp ne i32 " + right.reg + ", 0\n");
    
    std::string orReg = ir.getNewTemp();
    ir.emit("  " + orReg + " = or i1 " + leftBool + ", " + rightBool + "\n");
    
    std::string resultReg = ir.getNewTemp();
    ir.emit("  " + resultReg + " = zext i1 " + orReg + " to i32\n");
    
    return Value(resultReg, std::make_shared<IntType>());
}

std::any IRBuilder::visitConstExp(SysYParser::ConstExpContext *ctx) {
    return visit(ctx->addExp());
}

