
// Generated from SysYParser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "SysYParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SysYParser.
 */
class  SysYParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SysYParser.
   */
    virtual std::any visitCompUnit(SysYParser::CompUnitContext *context) = 0;

    virtual std::any visitDecl(SysYParser::DeclContext *context) = 0;

    virtual std::any visitConstDecl(SysYParser::ConstDeclContext *context) = 0;

    virtual std::any visitBType(SysYParser::BTypeContext *context) = 0;

    virtual std::any visitConstDef(SysYParser::ConstDefContext *context) = 0;

    virtual std::any visitConstInitVal(SysYParser::ConstInitValContext *context) = 0;

    virtual std::any visitVarDecl(SysYParser::VarDeclContext *context) = 0;

    virtual std::any visitVarDef(SysYParser::VarDefContext *context) = 0;

    virtual std::any visitInitVal(SysYParser::InitValContext *context) = 0;

    virtual std::any visitFuncDef(SysYParser::FuncDefContext *context) = 0;

    virtual std::any visitFuncType(SysYParser::FuncTypeContext *context) = 0;

    virtual std::any visitFuncFParams(SysYParser::FuncFParamsContext *context) = 0;

    virtual std::any visitFuncFParam(SysYParser::FuncFParamContext *context) = 0;

    virtual std::any visitBlock(SysYParser::BlockContext *context) = 0;

    virtual std::any visitBlockItem(SysYParser::BlockItemContext *context) = 0;

    virtual std::any visitAssignStmt(SysYParser::AssignStmtContext *context) = 0;

    virtual std::any visitExpStmt(SysYParser::ExpStmtContext *context) = 0;

    virtual std::any visitBlockStmt(SysYParser::BlockStmtContext *context) = 0;

    virtual std::any visitIfStmt(SysYParser::IfStmtContext *context) = 0;

    virtual std::any visitWhileStmt(SysYParser::WhileStmtContext *context) = 0;

    virtual std::any visitBreakStmt(SysYParser::BreakStmtContext *context) = 0;

    virtual std::any visitContinueStmt(SysYParser::ContinueStmtContext *context) = 0;

    virtual std::any visitReturnStmt(SysYParser::ReturnStmtContext *context) = 0;

    virtual std::any visitExp(SysYParser::ExpContext *context) = 0;

    virtual std::any visitCond(SysYParser::CondContext *context) = 0;

    virtual std::any visitLVal(SysYParser::LValContext *context) = 0;

    virtual std::any visitPrimaryExp(SysYParser::PrimaryExpContext *context) = 0;

    virtual std::any visitNumber(SysYParser::NumberContext *context) = 0;

    virtual std::any visitPrimaryUnaryExp(SysYParser::PrimaryUnaryExpContext *context) = 0;

    virtual std::any visitFuncCallExp(SysYParser::FuncCallExpContext *context) = 0;

    virtual std::any visitUnaryOpExp(SysYParser::UnaryOpExpContext *context) = 0;

    virtual std::any visitUnaryOp(SysYParser::UnaryOpContext *context) = 0;

    virtual std::any visitFuncRParams(SysYParser::FuncRParamsContext *context) = 0;

    virtual std::any visitMulDivModExp(SysYParser::MulDivModExpContext *context) = 0;

    virtual std::any visitUnaryMulExp(SysYParser::UnaryMulExpContext *context) = 0;

    virtual std::any visitMulAddExp(SysYParser::MulAddExpContext *context) = 0;

    virtual std::any visitAddSubExp(SysYParser::AddSubExpContext *context) = 0;

    virtual std::any visitRelOpExp(SysYParser::RelOpExpContext *context) = 0;

    virtual std::any visitAddRelExp(SysYParser::AddRelExpContext *context) = 0;

    virtual std::any visitEqNeExp(SysYParser::EqNeExpContext *context) = 0;

    virtual std::any visitRelEqExp(SysYParser::RelEqExpContext *context) = 0;

    virtual std::any visitEqLAndExp(SysYParser::EqLAndExpContext *context) = 0;

    virtual std::any visitAndExp(SysYParser::AndExpContext *context) = 0;

    virtual std::any visitLAndLOrExp(SysYParser::LAndLOrExpContext *context) = 0;

    virtual std::any visitOrExp(SysYParser::OrExpContext *context) = 0;

    virtual std::any visitConstExp(SysYParser::ConstExpContext *context) = 0;


};

