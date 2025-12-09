parser grammar SysYParser;

options {
    tokenVocab = SysYLexer;
}

// Starting symbol
compUnit: (decl | funcDef)* EOF;

// Declarations
decl: constDecl | varDecl;

// Constant declaration
constDecl: CONST bType constDef (COMMA constDef)* SEMICOLON;

bType: INT;

constDef: IDENT (LBRACKET constExp RBRACKET)* ASSIGN constInitVal;

constInitVal: constExp
            | LBRACE (constInitVal (COMMA constInitVal)*)? RBRACE;

// Variable declaration
varDecl: bType varDef (COMMA varDef)* SEMICOLON;

varDef: IDENT (LBRACKET constExp RBRACKET)*
      | IDENT (LBRACKET constExp RBRACKET)* ASSIGN initVal;

initVal: exp
       | LBRACE (initVal (COMMA initVal)*)? RBRACE;

// Function definition
funcDef: funcType IDENT LPAREN (funcFParams)? RPAREN block;

funcType: VOID | INT;

funcFParams: funcFParam (COMMA funcFParam)*;

funcFParam: bType IDENT (LBRACKET RBRACKET (LBRACKET exp RBRACKET)*)?;

// Block
block: LBRACE blockItem* RBRACE;

blockItem: decl | stmt;

// Statements
stmt: lVal ASSIGN exp SEMICOLON                           # assignStmt
    | exp? SEMICOLON                                       # expStmt
    | block                                                # blockStmt
    | IF LPAREN cond RPAREN stmt (ELSE stmt)?              # ifStmt
    | WHILE LPAREN cond RPAREN stmt                        # whileStmt
    | BREAK SEMICOLON                                      # breakStmt
    | CONTINUE SEMICOLON                                   # continueStmt
    | RETURN exp? SEMICOLON                                # returnStmt
    ;

// Expressions
exp: addExp;

cond: lOrExp;

lVal: IDENT (LBRACKET exp RBRACKET)*;

primaryExp: LPAREN exp RPAREN
          | lVal
          | number;

number: INTEGER_CONST;

unaryExp: primaryExp                                       # primaryUnaryExp
        | IDENT LPAREN (funcRParams)? RPAREN               # funcCallExp
        | unaryOp unaryExp                                 # unaryOpExp
        ;

unaryOp: PLUS | MINUS | NOT;

funcRParams: exp (COMMA exp)*;

mulExp: unaryExp                                           # unaryMulExp
      | mulExp (MUL | DIV | MOD) unaryExp                  # mulDivModExp
      ;

addExp: mulExp                                             # mulAddExp
      | addExp (PLUS | MINUS) mulExp                       # addSubExp
      ;

relExp: addExp                                             # addRelExp
      | relExp (LT | GT | LE | GE) addExp                  # relOpExp
      ;

eqExp: relExp                                              # relEqExp
     | eqExp (EQ | NE) relExp                              # eqNeExp
     ;

lAndExp: eqExp                                             # eqLAndExp
       | lAndExp AND eqExp                                 # andExp
       ;

lOrExp: lAndExp                                            # lAndLOrExp
      | lOrExp OR lAndExp                                  # orExp
      ;

constExp: addExp;