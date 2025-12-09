lexer grammar SysYLexer;

// Keywords
CONST: 'const';
INT: 'int';
VOID: 'void';
IF: 'if';
ELSE: 'else';
WHILE: 'while';
BREAK: 'break';
CONTINUE: 'continue';
RETURN: 'return';

// Operators
PLUS: '+';
MINUS: '-';
MUL: '*';
DIV: '/';
MOD: '%';

// Relational operators
LT: '<';
GT: '>';
LE: '<=';
GE: '>=';
EQ: '==';
NE: '!=';

// Logical operators
NOT: '!';
AND: '&&';
OR: '||';

// Assignment
ASSIGN: '=';

// Delimiters
SEMICOLON: ';';
COMMA: ',';
LPAREN: '(';
RPAREN: ')';
LBRACKET: '[';
RBRACKET: ']';
LBRACE: '{';
RBRACE: '}';

// Identifiers
IDENT: [a-zA-Z_][a-zA-Z0-9_]*;

// Integer constants
INTEGER_CONST: DECIMAL_CONST | OCTAL_CONST | HEXADECIMAL_CONST;

fragment DECIMAL_CONST: [1-9][0-9]*;
fragment OCTAL_CONST: '0' [0-7]*;
fragment HEXADECIMAL_CONST: ('0x' | '0X') [0-9a-fA-F]+;

// Whitespace
WS: [ \t\r\n]+ -> skip;

// Comments
LINE_COMMENT: '//' ~[\r\n]* -> skip;
BLOCK_COMMENT: '/*' .*? '*/' -> skip;