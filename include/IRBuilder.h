#ifndef IRBUILDER_H
#define IRBUILDER_H

#include "SysYParserBaseVisitor.h"
#include "SymbolTable.h"
#include "IRGenerator.h"
#include <any>

struct Value {
    std::string reg;  // Register name or constant value
    std::shared_ptr<Type> type;
    bool isConst;
    int constValue;
    std::vector<int> constArrayValues;
    
    Value() : isConst(false), constValue(0) {}
    Value(const std::string& r, std::shared_ptr<Type> t) 
        : reg(r), type(t), isConst(false), constValue(0) {}
};

class IRBuilder : public SysYParserBaseVisitor {
private:
    SymbolTable symbolTable;
    IRGenerator ir;
    std::string currentFuncReturnType;
    std::map<std::string, std::string> varMap;  // Variable name to IR register/global name
    
public:
    IRBuilder() {
        // Add built-in functions from sylib
        addBuiltInFunctions();
    }
    
    void addBuiltInFunctions();
    
    std::string getIR() {
        return ir.getOutput();
    }
    
    // Override visitor methods
    std::any visitCompUnit(SysYParser::CompUnitContext *ctx) override;
    std::any visitDecl(SysYParser::DeclContext *ctx) override;
    std::any visitConstDecl(SysYParser::ConstDeclContext *ctx) override;
    std::any visitConstDef(SysYParser::ConstDefContext *ctx) override;
    std::any visitVarDecl(SysYParser::VarDeclContext *ctx) override;
    std::any visitVarDef(SysYParser::VarDefContext *ctx) override;
    std::any visitFuncDef(SysYParser::FuncDefContext *ctx) override;
    std::any visitBlock(SysYParser::BlockContext *ctx) override;
    std::any visitAssignStmt(SysYParser::AssignStmtContext *ctx) override;
    std::any visitExpStmt(SysYParser::ExpStmtContext *ctx) override;
    std::any visitBlockStmt(SysYParser::BlockStmtContext *ctx) override;
    std::any visitIfStmt(SysYParser::IfStmtContext *ctx) override;
    std::any visitWhileStmt(SysYParser::WhileStmtContext *ctx) override;
    std::any visitBreakStmt(SysYParser::BreakStmtContext *ctx) override;
    std::any visitContinueStmt(SysYParser::ContinueStmtContext *ctx) override;
    std::any visitReturnStmt(SysYParser::ReturnStmtContext *ctx) override;
    std::any visitExp(SysYParser::ExpContext *ctx) override;
    std::any visitCond(SysYParser::CondContext *ctx) override;
    std::any visitLVal(SysYParser::LValContext *ctx) override;
    std::any visitPrimaryExp(SysYParser::PrimaryExpContext *ctx) override;
    std::any visitNumber(SysYParser::NumberContext *ctx) override;
    std::any visitPrimaryUnaryExp(SysYParser::PrimaryUnaryExpContext *ctx) override;
    std::any visitFuncCallExp(SysYParser::FuncCallExpContext *ctx) override;
    std::any visitUnaryOpExp(SysYParser::UnaryOpExpContext *ctx) override;
    std::any visitUnaryMulExp(SysYParser::UnaryMulExpContext *ctx) override;
    std::any visitMulDivModExp(SysYParser::MulDivModExpContext *ctx) override;
    std::any visitMulAddExp(SysYParser::MulAddExpContext *ctx) override;
    std::any visitAddSubExp(SysYParser::AddSubExpContext *ctx) override;
    std::any visitAddRelExp(SysYParser::AddRelExpContext *ctx) override;
    std::any visitRelOpExp(SysYParser::RelOpExpContext *ctx) override;
    std::any visitRelEqExp(SysYParser::RelEqExpContext *ctx) override;
    std::any visitEqNeExp(SysYParser::EqNeExpContext *ctx) override;
    std::any visitEqLAndExp(SysYParser::EqLAndExpContext *ctx) override;
    std::any visitAndExp(SysYParser::AndExpContext *ctx) override;
    std::any visitLAndLOrExp(SysYParser::LAndLOrExpContext *ctx) override;
    std::any visitOrExp(SysYParser::OrExpContext *ctx) override;
    std::any visitConstExp(SysYParser::ConstExpContext *ctx) override;
    std::any visitConstInitVal(SysYParser::ConstInitValContext *ctx) override;
    std::any visitInitVal(SysYParser::InitValContext *ctx) override;
    
private:
    int evaluateConstExp(SysYParser::ConstExpContext *ctx);
    std::vector<int> evaluateConstInitVal(SysYParser::ConstInitValContext *ctx, 
                                          const std::vector<int>& dimensions, int depth);
    std::vector<int> evaluateInitVal(SysYParser::InitValContext *ctx,
                                     const std::vector<int>& dimensions, int depth);
};

#endif // IRBUILDER_H
