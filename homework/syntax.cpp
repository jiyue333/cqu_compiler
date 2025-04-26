/*
// 编译单元 
CompUnit -> (Decl | FuncDef) [CompUnit]

// 声明 
Decl -> ConstDecl | VarDecl

// 常量声明 
ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    ConstDecl.t  // 类型

// 基本类型 
BType -> 'int' | 'float'
    BType.t      // 类型

// 常量定义 
ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    ConstDef.arr_name // 数组名 (如果存在)

// 常量初始值 
ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    ConstInitVal.v // 值
    ConstInitVal.t // 类型

// 变量声明 
VarDecl -> BType VarDef { ',' VarDef } ';'
    VarDecl.t    // 类型

// 变量定义 
VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
    VarDef.arr_name // 数组名 (如果存在)

// 初始值 
InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    InitVal.is_computable // 是否在编译时可计算
    InitVal.v           // 值
    InitVal.t           // 类型

// 函数定义 
FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    FuncDef.t // 返回类型
    FuncDef.n // 函数名

// 函数类型 
FuncType -> 'void' | 'int' | 'float'

// 函数形式参数 
FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]

// 函数形式参数列表 
FuncFParams -> FuncFParam { ',' FuncFParam }

// 语句块 
Block -> '{' { BlockItem } '}'

// 块内项 
BlockItem -> Decl | Stmt

// 语句 
Stmt -> LVal '=' Exp ';'
      | Block
      | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
      | 'while' '(' Cond ')' Stmt
      | 'break' ';'
      | 'continue' ';'
      | ';'
      | 'return' [Exp] ';'
      | [Exp] ';'

// 表达式 
Exp -> AddExp
    Exp.is_computable // 是否在编译时可计算
    Exp.v           // 值
    Exp.t           // 类型

// 条件表达式 
Cond -> LOrExp
    Cond.is_computable // 是否在编译时可计算
    Cond.v           // 值
    Cond.t           // 类型

// 左值 
LVal -> Ident {'[' Exp ']'}
    LVal.is_computable // 是否在编译时可计算 (通常为 false)
    LVal.v           // 值
    LVal.t           // 类型
    LVal.i           // 标识符 (变量名或数组名)

// 数字 
Number -> IntConst | floatConst

// 初等表达式 
PrimaryExp -> '(' Exp ')'
            | LVal
            | Number
    PrimaryExp.is_computable // 是否在编译时可计算
    PrimaryExp.v           // 值
    PrimaryExp.t           // 类型

// 一元表达式 
UnaryExp -> PrimaryExp
            | Ident '(' [FuncRParams] ')' // 函数调用
            | UnaryOp UnaryExp
    UnaryExp.is_computable // 是否在编译时可计算
    UnaryExp.v           // 值
    UnaryExp.t           // 类型

// 一元运算符 
UnaryOp -> '+' | '-' | '!'

// 函数实参列表 
FuncRParams -> Exp { ',' Exp }

// 乘法表达式 
MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    MulExp.is_computable // 是否在编译时可计算
    MulExp.v           // 值
    MulExp.t           // 类型

// 加法表达式 
AddExp -> MulExp { ('+' | '-') MulExp }
    AddExp.is_computable // 是否在编译时可计算
    AddExp.v           // 值
    AddExp.t           // 类型

// 关系表达式 
RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    RelExp.is_computable // 是否在编译时可计算
    RelExp.v           // 值
    RelExp.t           // 类型

// 相等性表达式 
EqExp -> RelExp { ('==' | '!=') RelExp }
    EqExp.is_computable // 是否在编译时可计算
    EqExp.v           // 值
    EqExp.t           // 类型

// 逻辑与表达式 
LAndExp -> EqExp [ '&&' LAndExp ]
    LAndExp.is_computable // 是否在编译时可计算
    LAndExp.v           // 值
    LAndExp.t           // 类型

// 逻辑或表达式 
LOrExp -> LAndExp [ '||' LOrExp ]
    LOrExp.is_computable // 是否在编译时可计算
    LOrExp.v           // 值
    LOrExp.t           // 类型

// 常量表达式 
ConstExp -> AddExp
    ConstExp.is_computable: true // 必须在编译时可计算
    ConstExp.v           // 值
    ConstExp.t           // 类型

*/