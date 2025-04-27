#include"front/semantic.h"

#include<cassert>

using ir::Instruction;
using ir::Function;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");

#define GET_CHILD_PTR(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node); 
#define ANALYSIS(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node); analysis##type(node, buffer);
#define COPY_EXP_NODE(from, to) to->is_computable = from->is_computable; to->v = from->v; to->t = from->t;

map<std::string,ir::Function*>* frontend::get_lib_funcs() {
    static map<std::string,ir::Function*> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray", new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)}, Type::null)},
        {"putfarray", new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)}, Type::null)},
    };
    return &lib_funcs;
}

void frontend::SymbolTable::add_scope(Block* node) {
    ScopeInfo scope_info;
    scope_info.name = 'b';
    scope_info.cnt = ++block_cnt;
    scope_stack.push_back(scope_info);
}
void frontend::SymbolTable::exit_scope() {
    --block_cnt;
    scope_stack.pop_back();
}

string frontend::SymbolTable::get_scoped_name(string id) const {
    return id + '_' + std::to_string(block_cnt);
}

Operand frontend::SymbolTable::get_operand(string id) const {
    for (size_t i = scope_stack.size() - 1; i >= 0; i--)
    {
        map<string, STE> table = scope_stack[i].table;
        if (table.count(id)) {
            return table[id].operand;
        }
    }
    return Operand(id, Type::Int);
}

frontend::STE frontend::SymbolTable::get_ste(string id) const {
    for (size_t i = scope_stack.size() - 1; i >= 0; i--)
    {
        map<string, STE> table = scope_stack[i].table;
        if (table.count(id)) {
            return table[id];
        }
    }
    return STE();
}

frontend::Analyzer::Analyzer(): tmp_cnt(0), symbol_table() {
    ScopeInfo inittable;
    inittable.cnt = 0;
    inittable.name = 'global';
    inittable.table = map_str_ste();
    symbol_table.scope_stack.push_back(inittable);
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit* root) {
    TODO;
}

void frontend::Analyzer::analysisCompUnit(CompUnit*, ir::Program&) {
    TODO;
}
void frontend::Analyzer::analysisFuncDef(FuncDef*, ir::Function&) {
    TODO;
}
void frontend::Analyzer::analysisDecl(Decl*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisConstDecl(ConstDecl*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisBType(BType*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisVarDecl(VarDecl*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisVarDef(VarDef*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisInitVal(InitVal*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisFuncType(FuncType*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisFuncFParam(FuncFParam*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisBlockItem(BlockItem*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisStmt(Stmt*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisExp(Exp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisCond(Cond*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisUnaryExp(UnaryExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisUnaryOp(UnaryOp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisFuncRParams(FuncRParams*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisRelExp(RelExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisEqExp(EqExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisLAndExp(LAndExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisLOrExp(LOrExp*, vector<ir::Instruction*>&) {
    TODO;
}
