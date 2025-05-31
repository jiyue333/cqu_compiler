/**
 * @file semantic.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-06
 * 
 * a Analyzer should 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include"ir/ir.h"
#include"front/abstract_syntax_tree.h"

#include<map>
#include<string>
#include<vector>
using std::map;
using std::string;
using std::vector;

namespace frontend
{

// definition of symbol table entry
struct STE {
    ir::Operand operand;
    vector<int> dimension;
};

using map_str_ste = map<string, STE>;
// definition of scope infomation
struct ScopeInfo {
    int cnt;
    string name;
    map_str_ste table;
};

// surpport lib functions
map<std::string,ir::Function*>* get_lib_funcs();

// definition of symbol table
struct SymbolTable{
    vector<ScopeInfo> scope_stack;
    map<std::string,ir::Function*> functions;

    /**
     * @brief enter a new scope, record the infomation in scope stacks
     * @param node: a Block node, entering a new Block means a new name scope
     */
    void add_scope(Block*);

    /**
     * @brief exit a scope, pop out infomations
     */
    void exit_scope();

    /**
     * @brief Get the scoped name, to deal the same name in different scopes, we change origin id to a new one with scope infomation,
     * for example, we have these code:
     * "     
     * int a;
     * {
     *      int a; ....
     * }
     * "
     * in this case, we have two variable both name 'a', after change they will be 'a' and 'a_block'
     * @param id: origin id 
     * @return string: new name with scope infomations
     */
    string get_scoped_name(string id) const;

    /**
     * @brief get the right operand with the input name
     * @param id identifier name
     * @return Operand 
     */
    ir::Operand get_operand(string id) const;

    /**
     * @brief get the right ste with the input name
     * @param id identifier name
     * @return STE 
     */
    STE get_ste(string id) const;
};


// singleton class
struct Analyzer {
    int tmp_cnt;
    vector<ir::Instruction*> g_init_inst;
    SymbolTable symbol_table;
    ir::Function* current_func;

    /**
     * @brief constructor
     */
    Analyzer();

    // analysis functions
    ir::Program get_ir_program(CompUnit*);

    // reject copy & assignment
    Analyzer(const Analyzer&) = delete;
    Analyzer& operator=(const Analyzer&) = delete;

    // analysis functions
    void type_transform(ir::Operand& , ir::Operand& , vector<ir::Instruction*>& );
    void analysisCompUnit(CompUnit*, ir::Program&);
    void analysisDecl(Decl*, vector<ir::Instruction*>&);
    void analysisConstDecl(ConstDecl*, vector<ir::Instruction*>&);
    void analysisBType(BType*, vector<ir::Instruction*>&);
    void analysisConstDef(ConstDef*, vector<ir::Instruction*>&);
    void analysisConstInitVal(ConstInitVal*, vector<ir::Instruction*>&);
    void analysisVarDecl(VarDecl*, vector<ir::Instruction*>&);
    void analysisVarDef(VarDef*, vector<ir::Instruction*>&);
    void analysisInitVal(InitVal*, vector<ir::Instruction*>&);
    void analysisFuncDef(FuncDef*, ir::Function&);
    void analysisFuncType(FuncType*, vector<ir::Instruction*>&);
    void analysisFuncFParam(FuncFParam*, ir::Function&);
    void analysisFuncFParams(FuncFParams*, ir::Function&);
    void analysisBlock(Block*, vector<ir::Instruction*>&);
    void analysisBlockItem(BlockItem*, vector<ir::Instruction*>&);
    void analysisStmt(Stmt*, vector<ir::Instruction*>&);
    void analysisExp(Exp*, vector<ir::Instruction*>&);
    void analysisCond(Cond*, vector<ir::Instruction*>&);
    void analysisLVal(LVal*, vector<ir::Instruction*>&);
    void analysisNumber(Number*, vector<ir::Instruction*>&);
    void analysisPrimaryExp(PrimaryExp*, vector<ir::Instruction*>&);
    void analysisUnaryExp(UnaryExp*, vector<ir::Instruction*>&);
    void analysisUnaryOp(UnaryOp*, vector<ir::Instruction*>&);
    void analysisFuncRParams(FuncRParams*, vector<ir::Instruction*>&, ir::CallInst&);
    void analysisMulExp(MulExp*, vector<ir::Instruction*>&);
    void analysisAddExp(AddExp*, vector<ir::Instruction*>&);
    void analysisRelExp(RelExp*, vector<ir::Instruction*>&);
    void analysisEqExp(EqExp*, vector<ir::Instruction*>&);
    void analysisLAndExp(LAndExp*, vector<ir::Instruction*>&);
    void analysisLOrExp(LOrExp*, vector<ir::Instruction*>&);
    void analysisConstExp(ConstExp*, vector<ir::Instruction*>&);
};

} // namespace frontend

#endif


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