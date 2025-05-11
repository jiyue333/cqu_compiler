#include"front/semantic.h"

#include<cassert>

using ir::Instruction;
using ir::Function;
using ir::Operand;
using ir::Operator;

#include <iostream>

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

std::string trans2ten(std::string s){
    assert(!s.empty() && "Empty string in strtoint");
    if (s.size() >= 2) {
        // 二进制: 0b或0B开头
        if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
            int res = std::stoi(s.substr(2), nullptr, 2);
            return std::to_string(res);
        }
        else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            int res = std::stoi(s, nullptr, 16);
            return std::to_string(res);
        }
        else if (s[0] == '0') {
            int res = std::stoi(s, nullptr, 8);
            return std::to_string(res);
        }
    }
    int res = std::stoi(s);
    return std::to_string(res);
}

void frontend::SymbolTable::add_scope(Block* node) {
    ScopeInfo scope_info;
    scope_info.name = 'b';
    scope_info.cnt = scope_stack.size() + 1;
    scope_stack.push_back(scope_info);
}

void frontend::SymbolTable::exit_scope() {
    scope_stack.pop_back();
}

string frontend::SymbolTable::get_scoped_name(string id) const {
    return id + '_' + std::to_string(scope_stack.size());
}

Operand frontend::SymbolTable::get_operand(string id) const {
    for (size_t i = scope_stack.size() - 1; i >= 0; i--)
    {
        map_str_ste table = scope_stack[i].table;
        if (table.count(id)) {
            return table[id].operand;
        }
    }
    return Operand(id, Type::Int);
}

frontend::STE frontend::SymbolTable::get_ste(string id) const {
    for (size_t i = scope_stack.size() - 1; i >= 0; i--)
    {
        map_str_ste table = scope_stack[i].table;
        if (table.count(id)) {
            return table[id];
        }
    }
    return STE();
}

frontend::Analyzer::Analyzer(): tmp_cnt(0), symbol_table(), current_func(nullptr) {
    ScopeInfo inittable;
    inittable.cnt = 0;
    inittable.name = 'global';
    inittable.table = map_str_ste();
    symbol_table.scope_stack.push_back(inittable);
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit* root) {
    ir::Program program;
    Function* global_func = new Function("global", Type::null);
    symbol_table.functions.insert({"global", global_func});
    program.addFunction(*global_func);
    auto lib_funcs = *get_lib_funcs();
    for (auto it = lib_funcs.begin(); it != lib_funcs.end(); it++)
    {
        symbol_table.functions[it->first] = it->second;
    }
    analysisCompUnit(root, program);
    program.functions[0].addInst(new ir::Instruction({Operand("null", Type::null), Operand(), Operand(), Operator::_return}));

    std::cout << program.draw() << std::endl;
    return program;
}

// CompUnit -> (FuncDef | Decl) [CompUnit]
void frontend::Analyzer::analysisCompUnit(CompUnit* root, ir::Program& buffer) {
    if (root->children[0]->type == NodeType::DECL) {
        auto node = dynamic_cast<Decl*>(root->children[0]);
        analysisDecl(node, buffer.functions.back().InstVec);
    } else if (root->children[0]->type == NodeType::FUNCDEF) {
        if (buffer.functions.size() == 1) {
            auto global_ir = buffer.functions[0].InstVec;
            for (int i = 0; i < global_ir.size(); i++) {
                buffer.globalVal.push_back(ir::GlobalVal(global_ir[i]->des));
            }
        }
        auto node = dynamic_cast<FuncDef*>(root->children[0]);
        auto func = new ir::Function();
        analysisFuncDef(node, *func);
        buffer.addFunction(*func);
    }
    if (root->children.size() > 1) {
        ANALYSIS(compunit, CompUnit, 1);
    }
}

// Decl -> ConstDecl | VarDecl
void frontend::Analyzer::analysisDecl(Decl* root, vector<ir::Instruction*>& buffer) {
    if (root->children[0]->type == NodeType::CONSTDECL) {
        ANALYSIS(constdecl, ConstDecl, 0);
    } else if (root->children[0]->type == NodeType::VARDECL) {
        ANALYSIS(vardecl, VarDecl, 0);
    }
}

// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
void frontend::Analyzer::analysisConstDecl(ConstDecl* root, vector<ir::Instruction*>& buffer) {
    ANALYSIS(btype, BType, 1);
    root->t = btype->t;
    ANALYSIS(constdef, ConstDef, 2);
    int i = 3;
    while (dynamic_cast<Term*>(root->children[i])->token.type == TokenType::COMMA) {
        ANALYSIS(constdef2, ConstDef, i+1);
        i += 2;
    }
}

// BType -> 'int' | 'float'
void frontend::Analyzer::analysisBType(BType* root, vector<ir::Instruction*>& buffer) {
    auto tk = dynamic_cast<Term*>(root->children[0]);
    if (tk->token.type == TokenType::INTTK) {
        root->t = Type::Int;
    } else if (tk->token.type == TokenType::FLOATTK) {
        root->t = Type::Float;
    }
}

// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
// todo
void frontend::Analyzer::analysisConstDef(ConstDef* root, vector<ir::Instruction*>& buffer) {
    auto root_type = dynamic_cast<ConstDecl*>(root->parent)->t;   // 父节点ConstDecl的类型
    GET_CHILD_PTR(identifier, Term, 0);
    string id = identifier->token.value;    // 变量原名"a"
    string new_name = symbol_table.get_scoped_name(id);     // 符号表里的名字"a_g"
    root->arr_name = new_name;  // 数组的名字

    GET_CHILD_PTR(term, Term, 1);   // 获取第二个节点
    if (term->token.type == TokenType::ASSIGN){   //第二个节点是=,普通的变量定义
        ANALYSIS(constinitval, ConstInitVal, 2);    // 分析ConstInitVal节点
        Operand des = Operand(new_name, root_type);     // 目标操作数
        auto opcode = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
        Operand op1 = Operand(constinitval->v, constinitval->t);    // 第一操作数
        if (root_type == Type::Float){  // 浮点常量定义
            if (constinitval->t == Type::Int){  // 类型转换:Int->Float
                auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Float);
                buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_i2f));
                op1 = tmp;  // 更新第一操作数
            }else if (constinitval->t == Type::IntLiteral){     // 类型转换:IntLiteral->FloatLiteral
                op1.type = Type::FloatLiteral;
            }
        }else{  // 整型常量定义
            assert(root_type == Type::Int);
            if (constinitval->t == Type::Float){    // 类型转换:Float->Int
                auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_f2i));
                op1 = tmp;
            }else if(constinitval->t == Type::FloatLiteral){    // 类型转换:FloatLiteral->IntLiteral
                op1.name = std::to_string((int)std::stof(op1.name));  // string->float->int->string
                op1.type = Type::IntLiteral;
            }
        }
        buffer.push_back(new Instruction(op1, Operand(), des, opcode));     // 常量定义IR
        symbol_table.scope_stack.back().table.insert({id, {op1, {}}});      // 当前作用域符号表插入符号,因为是const常量,所以存入op1

    }else if ((int)root->children.size() == 6){   //一维数组定义
        ANALYSIS(constexp, ConstExp, 2);    // 分析ConstExp节点
        int array_size = std::stoi(constexp->v);    // 数组长度
        STE arr_ste;    // 临时STE
        arr_ste.dimension.push_back(array_size);  
        ir::Type curr_type = root_type;
        if (curr_type == ir::Type::Int){
            curr_type = ir::Type::IntPtr;
        }else if (curr_type == ir::Type::Float){
            curr_type = ir::Type::FloatPtr;
        }
        arr_ste.operand = ir::Operand(new_name, curr_type);
        symbol_table.scope_stack.back().table[id] = arr_ste;    // 插入符号表
        buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(new_name, curr_type), Operator::alloc}));

        // 一维数组的初始化
        GET_CHILD_PTR(constinitval, ConstInitVal, 5);
        if (constinitval->children.size() == 2){    // 只有{}去初始化数组
            for (int i = 0; i<array_size; i++){
                buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
            }
        }else{
            int cnt = 0;    // 数组下标
            for (int i = 1; i< (int)constinitval->children.size()-1; i+=2, cnt++){     // 遍历'{' [ ConstInitVal { ',' ConstInitVal } ] '}'
                ConstInitVal* child = dynamic_cast<ConstInitVal*>(constinitval->children[i]);
                ConstExp* constexp = dynamic_cast<ConstExp*>(child->children[0]);
                analysisConstExp(constexp, buffer); // 分析ConstExp节点
                buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(cnt), Type::IntLiteral), Operand(constexp->v, Type::IntLiteral), Operator::store}));
            }
        }
    // ConstDef -> Ident '[' ConstExp ']' '[' ConstExp ']' '=' ConstInitVal
    }else if ((int)root->children.size() == 9){  // 二维数组定义
        ANALYSIS(constexp1, ConstExp, 2);    // 分析ConstExp节点
        ANALYSIS(constexp2, ConstExp, 5);    // 分析ConstExp节点
        int array_size = std::stoi(constexp1->v) * std::stoi(constexp2->v);    // 数组长度
        STE arr_ste;    // 临时STE
        arr_ste.dimension.push_back(std::stoi(constexp1->v));   // 第一维
        arr_ste.dimension.push_back(std::stoi(constexp2->v));   // 第二维
        ir::Type curr_type = root_type;
        if (curr_type == ir::Type::Int){
            curr_type = ir::Type::IntPtr;
        }else if (curr_type == ir::Type::Float){
            curr_type = ir::Type::FloatPtr;
        }
        arr_ste.operand = ir::Operand(new_name, curr_type);
        symbol_table.scope_stack.back().table[id] = arr_ste;    // 插入符号表
        buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(new_name, curr_type), Operator::alloc}));
        
        // 二维数组的初始化
        GET_CHILD_PTR(constinitval, ConstInitVal, 8);
        if (constinitval->children.size() == 2){    // 只有{}去初始化数组
            for (int i = 0; i<array_size; i++){
                buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
            }
        }else{
            int cnt = 0;    // 数组下标
            for (int i = 1; i< (int)constinitval->children.size()-1; i+=2, cnt++){     // 遍历'{' [ ConstInitVal { ',' ConstInitVal } ] '}'
                ConstInitVal* child = dynamic_cast<ConstInitVal*>(constinitval->children[i]);
                ConstExp* constexp = dynamic_cast<ConstExp*>(child->children[0]);
                analysisConstExp(constexp, buffer); // 分析ConstExp节点
                buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(cnt), Type::IntLiteral), Operand(constexp->v, Type::IntLiteral), Operator::store}));
            }
        }
    }
}

// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
void frontend::Analyzer::analysisConstInitVal(ConstInitVal* root, vector<ir::Instruction*>& buffer) {
    if (root->children[0]->type == NodeType::CONSTEXP){
        ANALYSIS(constexp, ConstExp, 0);
        root->v = constexp->v;
        root->t = constexp->t;
    } else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::LBRACE){
        ANALYSIS(constinitval, ConstInitVal, 1);
        int i = 2;
        while (dynamic_cast<Term*>(root->children[i])->token.type == TokenType::COMMA) {
            ANALYSIS(constinitval2, ConstInitVal, i+1);
            i += 2;
        }
    }
}
// VarDecl -> BType VarDef { ',' VarDef } ';'
void frontend::Analyzer::analysisVarDecl(VarDecl* root, vector<ir::Instruction*>& buffer) {
    ANALYSIS(btype, BType, 0);
    root->t = btype->t;
    ANALYSIS(vardef, VarDef, 1);
    int i = 2;
    while (dynamic_cast<Term*>(root->children[i])->token.type == TokenType::COMMA) {
        ANALYSIS(vardef2, VarDef, i+1);
        i += 2;
    }
}

// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
void frontend::Analyzer::analysisVarDef(VarDef* root, vector<ir::Instruction*>& buffer) {
    
    auto root_type = dynamic_cast<VarDecl*>(root->parent)->t;   // 父节点VarDecl的类型


    GET_CHILD_PTR(identifier, Term, 0);
    string id = identifier->token.value;    // 变量原名"a"


    string new_name = symbol_table.get_scoped_name(id);     // 符号表里的名字"a_g"
    if ((int)root->children.size() == 1){    // 普通变量定义，没有赋值
        Operand des = Operand(new_name, root_type);     // 目标操作数
        auto opcode = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
        if (root_type == Type::Float){  // 变量为Float类型
            buffer.push_back(new Instruction(Operand("0.0", Type::FloatLiteral), Operand(), des, opcode));
        }else{  // 变量为Int类型
            buffer.push_back(new Instruction(Operand("0", Type::IntLiteral), Operand(), des, opcode));
        }
        // symbol_table.scope_stack.back().table.insert({id, {op1, {}}});      // 当前作用域符号表插入符号
        symbol_table.scope_stack.back().table.insert({id, {des, {}}});      // 当前作用域符号表插入符号
    }else{
        GET_CHILD_PTR(term, Term, 1);   // 获取第二个节点
        if (term->token.type == TokenType::ASSIGN){   //普通变量定义,有赋值
            ANALYSIS(initval, InitVal, 2);    // 分析InitVal节点
            Operand des = Operand(new_name, root_type);     // 目标操作数
            auto opcode = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
            Operand op1 = Operand(initval->v, initval->t);    // 第一操作数
            if (root_type == Type::Float){  // 浮点变量定义
                if (initval->t == Type::Int){  // 类型转换:Int->Float
                    auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_i2f));
                    op1 = tmp;  // 更新第一操作数
                }else if (initval->t == Type::IntLiteral){     // 类型转换:IntLiteral->FloatLiteral
                    op1.type = Type::FloatLiteral;
                }
            }else{  // 整型常量定义
                assert(root_type == Type::Int);
                if (initval->t == Type::Float){    // 类型转换:Float->Int
                    auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_f2i));
                    op1 = tmp;
                }else if(initval->t == Type::FloatLiteral){    // 类型转换:FloatLiteral->IntLiteral
                    op1.name = std::to_string((int)std::stof(op1.name));  // string->float->int->string
                    op1.type = Type::IntLiteral;
                }
            }
            buffer.push_back(new Instruction(op1, Operand(), des, opcode));     // 变量定义IR
            symbol_table.scope_stack.back().table.insert({id, {des, {}}});      // 当前作用域符号表插入符号
        
        }else if(root->children.back()->type == NodeType::INITVAL){    // 数组,有赋值
            // VarDef -> Ident '[' ConstExp ']' '=' InitVal
            if ((int)root->children.size() == 6){   //一维数组定义
                ANALYSIS(constexp, ConstExp, 2);    // 分析ConstExp节点
                int array_size = std::stoi(constexp->v);    // 数组长度
                STE arr_ste;    // 临时STE
                arr_ste.dimension.push_back(array_size);  
                ir::Type curr_type = root_type;
                if (curr_type == ir::Type::Int){
                    curr_type = ir::Type::IntPtr;
                }else if (curr_type == ir::Type::Float){
                    curr_type = ir::Type::FloatPtr;
                }
                arr_ste.operand = ir::Operand(new_name, curr_type);
                symbol_table.scope_stack.back().table[id] = arr_ste;    // 插入符号表
                buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(new_name, curr_type), Operator::alloc}));

                // 一维数组的初始化
                GET_CHILD_PTR(initval, InitVal, 5);
                if (initval->children.size() == 2){    // 只有{}去初始化数组
                    for (int i = 0; i<array_size; i++){
                        buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                    }
                }else{
                    int cnt = 0;    // 数组下标
                    for (int i = 1; i< (int)initval->children.size()-1; i+=2, cnt++){     // 遍历'{' [ InitVal { ',' InitVal } ] '}'
                        InitVal* child = dynamic_cast<InitVal*>(initval->children[i]);
                        Exp* exp = dynamic_cast<Exp*>(child->children[0]);
                        analysisExp(exp, buffer); // 分析Exp节点
                        buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(cnt), Type::IntLiteral), Operand(exp->v, Type::IntLiteral), Operator::store}));
                    }
                    // a[20]={1,2},最好写上后续初始化为0
                    for (;cnt<array_size;cnt++){
                        buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(cnt), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                    }
                }
            // VarDef -> Ident '[' ConstExp ']' '[' ConstExp ']' '=' InitVal
            }else if ((int)root->children.size() == 9){  // 二维数组定义
                ANALYSIS(constexp1, ConstExp, 2);    // 分析ConstExp节点
                ANALYSIS(constexp2, ConstExp, 5);    // 分析ConstExp节点
                int array_size = std::stoi(constexp1->v) * std::stoi(constexp2->v);    // 数组长度
                STE arr_ste;    // 临时STE
                arr_ste.dimension.push_back(std::stoi(constexp1->v));   // 第一维
                arr_ste.dimension.push_back(std::stoi(constexp2->v));   // 第二维
                ir::Type curr_type = root_type;
                if (curr_type == ir::Type::Int){
                    curr_type = ir::Type::IntPtr;
                }else if (curr_type == ir::Type::Float){
                    curr_type = ir::Type::FloatPtr;
                }
                arr_ste.operand = ir::Operand(new_name, curr_type);
                symbol_table.scope_stack.back().table[id] = arr_ste;    // 插入符号表
                buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(new_name, curr_type), Operator::alloc}));
                
                // 二维数组的初始化
                GET_CHILD_PTR(initval, InitVal, 8);
                if (initval->children.size() == 2){    // 只有{}去初始化数组
                    for (int i = 0; i<array_size; i++){
                        buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                    }
                }else{
                    int cnt = 0;    // 数组下标
                    for (int i = 1; i< (int)initval->children.size()-1; i+=2, cnt++){     // 遍历'{' [ ConstInitVal { ',' ConstInitVal } ] '}'
                        InitVal* child = dynamic_cast<InitVal*>(initval->children[i]);
                        Exp* exp = dynamic_cast<Exp*>(child->children[0]);
                        analysisExp(exp, buffer); // 分析Exp节点
                        buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(cnt), Type::IntLiteral), Operand(exp->v, Type::IntLiteral), Operator::store}));
                    }
                }
            }
        }else{  // 数组,没有赋值
            // VarDef -> Ident {'[' ConstExp ']'}
            if ((int)root->children.size() == 4){   //一维数组定义
                ANALYSIS(constexp, ConstExp, 2);    // 分析ConstExp节点
                int array_size = std::stoi(constexp->v);    // 数组长度
                STE arr_ste;    // 临时STE
                arr_ste.dimension.push_back(array_size);  
                ir::Type curr_type = root_type;
                if (curr_type == ir::Type::Int){
                    curr_type = ir::Type::IntPtr;
                }else if (curr_type == ir::Type::Float){
                    curr_type = ir::Type::FloatPtr;
                }
                arr_ste.operand = ir::Operand(new_name, curr_type);
                symbol_table.scope_stack.back().table[id] = arr_ste;    // 插入符号表
                buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(new_name, curr_type), Operator::alloc}));

                // 一维数组的初始化
                
                for (int i = 0; i<array_size; i++){
                    buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                }
            // VarDef -> Ident '[' ConstExp ']' '[' ConstExp ']'
            }else if ((int)root->children.size() == 7){
                ANALYSIS(constexp1, ConstExp, 2);    // 分析ConstExp节点
                ANALYSIS(constexp2, ConstExp, 5);    // 分析ConstExp节点
                int array_size = std::stoi(constexp1->v) * std::stoi(constexp2->v);    // 数组长度
                STE arr_ste;    // 临时STE
                arr_ste.dimension.push_back(std::stoi(constexp1->v));   // 第一维
                arr_ste.dimension.push_back(std::stoi(constexp2->v));   // 第二维
                ir::Type curr_type = root_type;
                if (curr_type == ir::Type::Int){
                    curr_type = ir::Type::IntPtr;
                }else if (curr_type == ir::Type::Float){
                    curr_type = ir::Type::FloatPtr;
                }
                arr_ste.operand = ir::Operand(new_name, curr_type);
                symbol_table.scope_stack.back().table[id] = arr_ste;    // 插入符号表
                buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(new_name, curr_type), Operator::alloc}));
                
                // 二维数组的初始化
                for (int i = 0; i<array_size; i++){
                    buffer.push_back(new Instruction({Operand(new_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                }
            }
        }
    }
    
}

// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
void frontend::Analyzer::analysisInitVal(InitVal* root, vector<ir::Instruction*>& buffer) {
    if (root->children[0]->type == NodeType::EXP){
        ANALYSIS(exp, Exp, 0);
        root->v = exp->v;
        root->t = exp->t;
    } else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::LBRACE){
        ANALYSIS(initval, InitVal, 1);
        int i = 2;
        while (dynamic_cast<Term*>(root->children[i])->token.type == TokenType::COMMA) {
            ANALYSIS(initval2, InitVal, i+1);
            i += 2;
        }
    }
}

// FuncDef -> FuncType Ident '(' [ FuncFParams ] ')' Block
void frontend::Analyzer::analysisFuncDef(FuncDef* root, ir::Function& function) {
    auto tk = dynamic_cast<Term*>(root->children[0]->children[0])->token.type;  //函数返回值类型
    root->t = tk == TokenType::VOIDTK ? Type::null : tk == TokenType::INTTK ? Type::Int :Type::Float;
    root->n = dynamic_cast<Term*>(root->children[1])->token.value;
    function.name = root->n;       //函数名
    function.returnType = root->t; //返回值类型

    int cnt = symbol_table.scope_stack.size() + 1;
    symbol_table.scope_stack.push_back({cnt, "fp", map_str_ste()});   //给函数形参增加作用域
    symbol_table.functions.insert({root->n, &function});            //增加函数
    current_func = &function;  // 当前函数指针

    if (function.name == "main"){   // 当前为main函数
        auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::null);
        auto global_callinst = new ir::CallInst(Operand("global", Type::null), vector<Operand>(), tmp);  // 函数调用IR
        current_func->addInst(global_callinst);
    }

    auto paras = dynamic_cast<FuncFParams*>(root->children[3]);     //第三个子节点
    if (paras){     // 如果函数参数列表存在
        analysisFuncFParams(paras, function);
        analysisBlock(dynamic_cast<Block*>(root->children[5]), function.InstVec);
    }else{
        analysisBlock(dynamic_cast<Block*>(root->children[4]), function.InstVec);
    }

    if (function.returnType == Type::null){     // 函数没有返回值，加上return null，防止返回不了
        auto return_inst = new Instruction({Operand("null", Type::null), {}, {}, Operator::_return});
        current_func->addInst(return_inst);
    }

    symbol_table.exit_scope();  //退出函数形参作用域
}


// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
void frontend::Analyzer::analysisFuncFParam(FuncFParam* root, ir::Function& function) {
    auto btype = dynamic_cast<BType*>(root->children[0]);
    analysisBType(btype, function.InstVec);
    auto id = dynamic_cast<Term*>(root->children[1])->token.value;
    if (root->children.size() == 3){
        auto type = (btype->t == Type::Int) ? Type::IntPtr : Type::FloatPtr;
        function.ParameterList.push_back(Operand(id, type));   // 增加参数
        symbol_table.scope_stack.back().table.insert({id, {Operand(id, type), {}}});
    } else {
        function.ParameterList.push_back(Operand(id, btype->t));
        symbol_table.scope_stack.back().table.insert({id, {Operand(id, btype->t), {}}});
    } 
}

// FuncFParams -> FuncFParam { ',' FuncFParam }
void frontend::Analyzer::analysisFuncFParams(FuncFParams* root, ir::Function& function) {
    int i = 0;
    while (i < (int)root->children.size()){
        analysisFuncFParam(dynamic_cast<FuncFParam*>(root->children[i]), function);
        i += 2;
    }
}

// Block -> '{' { BlockItem } '}'
void frontend::Analyzer::analysisBlock(Block* root, vector<ir::Instruction*>& buffer) {
    symbol_table.add_scope(root);
    int i = 1;
    while (i < (int)root->children.size()-1){
        ANALYSIS(blockitem, BlockItem, i);
        i += 1;
    }
    symbol_table.exit_scope();
}

// BlockItem -> Decl | Stmt
void frontend::Analyzer::analysisBlockItem(BlockItem* root, vector<ir::Instruction*>& buffer) {
    if (root->children[0]->type == NodeType::DECL){
        ANALYSIS(decl, Decl, 0);
    }else{
        ANALYSIS(stmt, Stmt, 0);
    }
}

// Stmt -> LVal '=' Exp ';' | Block | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] | 'while' '(' Cond ')' Stmt | 'break' ';' | 'continue' ';' | 'return' [Exp] ';' | [Exp] ';'
// todo
void frontend::Analyzer::analysisStmt(Stmt* root, vector<ir::Instruction*>& buffer) {
     if (root->children[0]->type == NodeType::LVAL){     // 赋值语句块
        ANALYSIS(exp, Exp, 2);  // 分析Exp节点
        ANALYSIS(lval, LVal, 0);    // 分析lval节点

    }else if (root->children[0]->type == NodeType::BLOCK){   // Block块

        ANALYSIS(block, Block, 0);

    }else if (root->children[0]->type == NodeType::EXP){    // Exp块

        ANALYSIS(exp, Exp, 0);

    }else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::IFTK){  // if块
        // Stmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]

        auto tmp1 = vector<Instruction*>();
        GET_CHILD_PTR(cond, Cond, 2);
        analysisCond(cond, tmp1);    // 分析cond节点
        buffer.insert(buffer.end(), tmp1.begin(), tmp1.end());    // 插入cond IR

        // if 成立的跳转
        buffer.push_back(new Instruction(Operand(cond->v,cond->t), Operand(), Operand("2",Type::IntLiteral), Operator::_goto));

        // 分析if的Stmt
        GET_CHILD_PTR(stmt1, Stmt, 4);   // 获取if的stmt
        auto tmp2 = vector<Instruction*>();  // if的stmt IR
        analysisStmt(stmt1, tmp2);   // 分析stmt节点

        if ((int)root->children.size() == 5){    // if 没有else

            // if 不成立的跳转
            buffer.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(tmp2.size()+1), Type::IntLiteral), Operator::_goto}));

            // 插入if stmt的IR
            buffer.insert(buffer.end(), tmp2.begin(), tmp2.end());

            // 增加无用IR,防止if块跳出没有IR了
            buffer.push_back(new Instruction({Operand(), Operand(), Operand(), Operator::__unuse__}));

        }else{      // if 有else
            auto tmp3 = vector<Instruction*>();     // else的stmt IR
            GET_CHILD_PTR(stmt2, Stmt, 6);   // 获取else 的stmt
            analysisStmt(stmt2, tmp3);   // 分析else 的stmt节点

            // if执行完要跳过else
            tmp2.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(tmp3.size()+1), Type::IntLiteral), Operator::_goto}));

            // 执行else要跳过if
            buffer.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(tmp2.size()+1), Type::IntLiteral), Operator::_goto}));

            // 合并if的stmt
            buffer.insert(buffer.end(), tmp2.begin(), tmp2.end());

            // 合并else的stmt
            buffer.insert(buffer.end(), tmp3.begin(), tmp3.end());

            // 增加无用IR,防止if块跳出没有IR了
            buffer.push_back(new Instruction({Operand(), Operand(), Operand(), Operator::__unuse__}));
        }
    }else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::WHILETK){   // while块
        
        // Stmt -> 'while' '(' Cond ')' Stmt 
        
        GET_CHILD_PTR(cond, Cond, 2);
        auto tmp1 = vector<Instruction*>();  // cond IR
        analysisCond(cond, tmp1);

        GET_CHILD_PTR(stmt, Stmt, 4);
        auto tmp2 = vector<Instruction*>();  // while的stmt IR
        analysisStmt(stmt, tmp2);

        // 每一轮while结束都要回到开头
        tmp2.push_back(new Instruction({Operand("continue", Type::null), Operand(), Operand(), Operator::__unuse__}));

        // 遍历WHILE体中的BREAK与CONTINUE标记指令, 修改为_goto
        for (int i=0; i<(int)tmp2.size(); i++){
            if (tmp2[i]->op == Operator::__unuse__ && tmp2[i]->op1.type == Type::null){
                if (tmp2[i]->op1.name == "break"){
                    tmp2[i] = new Instruction({Operand(), Operand(), Operand(std::to_string((int)tmp2.size()-i),Type::IntLiteral), Operator::_goto});
                }
                else if (tmp2[i]->op1.name == "continue"){
                    auto goto_inst = new Instruction({Operand(), Operand(), Operand(std::to_string(-(2+i+(int)tmp1.size())), Type::IntLiteral), Operator::_goto});
                    tmp2[i] = goto_inst;
                }
            }
        }

        // 合并cond IR
        buffer.insert(buffer.end(), tmp1.begin(), tmp1.end());
        
        // 满足条件,执行stmt
        buffer.push_back(new Instruction({Operand(cond->v,cond->t), Operand(), Operand("2",Type::IntLiteral), Operator::_goto}));

        // 不满足,跳出stmt
        buffer.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(tmp2.size()+1), Type::IntLiteral), Operator::_goto}));

        // 合并stmt IR
        buffer.insert(buffer.end(), tmp2.begin(), tmp2.end());

        // 插入unuse
        buffer.push_back(new Instruction(Operand(), Operand(), Operand(), Operator::__unuse__));

    }else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::BREAKTK){   // break块
        
        buffer.push_back(new Instruction({Operand("break", Type::null), Operand(), Operand(), Operator::__unuse__}));

    }else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::CONTINUETK){    // continue块
        
        buffer.push_back(new Instruction({Operand("continue", Type::null), Operand(), Operand(), Operator::__unuse__}));
    
    }else if (dynamic_cast<Term*>(root->children[0])->token.type == TokenType::RETURNTK){  // return块
        
        // stmt -> 'return' [Exp] ';'

        if ((int)root->children.size() == 2){
            Instruction* return_inst = new Instruction({Operand("null", Type::null), Operand(), Operand(), Operator::_return});
            buffer.push_back(return_inst);

        }else{
            // stmt -> 'return' Exp ';'
            auto tmp = vector<Instruction*>();
            GET_CHILD_PTR(exp, Exp, 1);
            analysisExp(exp, tmp);
            buffer.insert(buffer.end(), tmp.begin(), tmp.end());     // 插入exp IR

            std::cout<<curr_function->name<<std::endl;
            std::cout<<toString(curr_function->returnType)<<std::endl;

            // 根据函数返回类型进行返回
            if (curr_function->returnType == Type::Int)
            {
                // Int or IntLiteral
                if (exp->t == Type::Int || exp->t == Type::IntLiteral){
                    Instruction* rerurn_inst = new Instruction({Operand(exp->v, exp->t), Operand(), Operand(), Operator::_return});
                    buffer.push_back(rerurn_inst);  

                    std::cout<<toString(exp->t)<<std::endl;

                }
                // Float or FloatLiteral
                else if (exp->t == Type::FloatLiteral){
                    buffer.push_back(new Instruction({Operand(std::to_string((int)std::stof(exp->v)), Type::IntLiteral), Operand(), Operand(), Operator::_return}));
                    std::cout<<toString(exp->t)<<std::endl;
                }
                else if (exp->t == Type::Float){
                    Operand tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(Operand(exp->v,Type::Float), Operand(), tmp, Operator::cvt_f2i));
                    buffer.push_back(new Instruction(tmp, Operand(), Operand(), Operator::_return));
                    std::cout<<toString(exp->t)<<std::endl;
                }
            }
            else if (curr_function->returnType == Type::Float)
            {
                // Float or FloatLiteral
                if (exp->t == Type::Float || exp->t == Type::FloatLiteral){
                    Instruction* retInst = new Instruction(Operand(exp->v,exp->t), Operand(), Operand(), Operator::_return);
                    buffer.push_back(retInst);
                    std::cout<<toString(exp->t)<<std::endl;
                }
                // Int or IntLiteral
                else if (exp->t == Type::IntLiteral){
                    float val = (float)std::stoi(exp->v);
                    Instruction* retInst = new Instruction(Operand(std::to_string(val),Type::FloatLiteral), Operand(), Operand(), Operator::_return);
                    buffer.push_back(retInst);
                    std::cout<<toString(exp->t)<<std::endl;
                }
                else if (exp->t == Type::Int){
                    Operand tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Float);
                    Instruction* cvtInst = new Instruction(Operand(exp->v, exp->t), Operand(), tmp, Operator::cvt_i2f);
                    Instruction* retInst = new Instruction(tmp, Operand(), Operand(), Operator::_return);
                    buffer.push_back(cvtInst);
                    buffer.push_back(retInst);
                    std::cout<<toString(exp->t)<<std::endl;
                }
            }
        }
    }
}

// Exp -> AddExp
void frontend::Analyzer::analysisExp(Exp* root, vector<ir::Instruction*>& buffer) {
    ANALYSIS(addexp, AddExp, 0);
    COPY_EXP_NODE(addexp, root);
}

// Cond -> LOrExp
void frontend::Analyzer::analysisCond(Cond* root, vector<ir::Instruction*>& buffer) {
    ANALYSIS(lor, LOrExp, 0);
    COPY_EXP_NODE(lor, root);
}

// LVal -> Ident {'[' Exp ']'}
void frontend::Analyzer::analysisLVal(LVal*, vector<ir::Instruction*>&) {
   auto tk = dynamic_cast<Term*>(root->children[0])->token;    // 获取Term节点的token


    auto op = symbol_table.get_operand(tk.value);   // 从符号表
    root->t = op.type;  // 从符号表里拿

    if((int)root->children.size() == 1){     // LVal -> Ident
        // root->v = tk.value;
        
        root->v = op.name;
        root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral) ? true : false;
        root->i = 0;

        if (root->parent->type == NodeType::STMT){   // 这里是lval=exp;
            auto exp_par = dynamic_cast<Exp*>(root->parent->children[2]);   // 的exp节点
            auto op1 = Operand(exp_par->v, exp_par->t);
            auto des = Operand(root->v, root->t);
            if (root->t == Type::Int){
                auto mov_inst = new Instruction({op1, Operand(), des, Operator::mov});
                buffer.push_back(mov_inst);    // 给整型变量赋值
            }else{
                buffer.push_back(new Instruction({op1, Operand(), des, Operator::fmov}));    // 给浮点变量赋值
            }
        }

    }else{      // LVal -> Ident {'[' Exp ']'}

        STE arr = symbol_table.get_ste(tk.value);
        vector<int> dimension = arr.dimension;  // 维度
        int size = dimension.size();    // 数组长度

        // Ident '[' Exp ']'
        if ((int)root->children.size() == 4){     // 一维数组

            ANALYSIS(exp, Exp, 2);
            Type t = (root->t == Type::IntPtr) ? Type::Int : Type::Float;
            root->t = t;
            Operand index = Operand(exp->v, exp->t);    // 取数下标
            if (root->parent->type == NodeType::STMT){   // Stmt->Lval=exp 作为左值，赋值操作
                auto exp_par = dynamic_cast<Exp*>(root->parent->children[2]);   // 取得所赋值节点exp
                Operand des = Operand(exp_par->v, exp_par->t);
                buffer.push_back(new Instruction({arr.operand, index, des, Operator::store}));  // des是存入的值
                root->v = des.name;
            }else{      // 作为右值，取数操作
                Operand des = Operand("t" + std::to_string(tmp_cnt++), t);     // 目的操作数为临时变量
                buffer.push_back(new Instruction({arr.operand, index, des, Operator::load}));  // 用临时变量暂存，再赋值
                root->v = des.name;
            }                   
        }else{      // 二维数组
            // Ident '[' Exp ']' '[' Exp ']'

            ANALYSIS(exp1, Exp, 2);
            ANALYSIS(exp2, Exp, 5);
            Type t = (root->t == Type::IntPtr) ? Type::Int : Type::Float;
            root->t = t;
            if (exp1->is_computable && exp2->is_computable){    // 可简化
                std::string i = std::to_string(std::stoi(exp1->v) * dimension[1] + std::stoi(exp2->v));
                Operand index = Operand(i, Type::IntLiteral);    // 取数下标
                if (root->parent->type == NodeType::STMT){   // Stmt->Lval=exp; 作为左值，赋值操作
                    auto exp_par = dynamic_cast<Exp*>(root->parent->children[2]);   // 取得所赋值节点exp
                    Operand des = Operand(exp_par->v, exp_par->t);
                    buffer.push_back(new Instruction({arr.operand, index, des, Operator::store}));
                    root->v = des.name;
                }else{
                    Operand des = Operand("t" + std::to_string(tmp_cnt++), t);     // 目的操作数为临时变量
                    buffer.push_back(new Instruction({arr.operand, index, des, Operator::load}));
                    root->v = des.name;
                }
            }else{      // 不可简化
                auto op1 = Operand(exp1->v, exp1->t);
                auto op2 = Operand(std::to_string(dimension[1]), Type::IntLiteral);
                auto op3 = Operand(exp2->v, exp2->t);
                type_transform(op1, op2, buffer);
                auto tmp1 = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                auto tmp2 = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction({op1, op2, tmp1, Operator::mul}));
                buffer.push_back(new Instruction({tmp1, op3, tmp2, Operator::add}));
                if (root->parent->type == NodeType::STMT){   // 赋值语句
                    auto exp_par = dynamic_cast<Exp*>(root->parent->children[2]);   // 取得所赋值节点exp
                    Operand des = Operand(exp_par->v, exp_par->t);
                    buffer.push_back(new Instruction({arr.operand, tmp2, des, Operator::store}));
                    root->v = des.name;
                }else{
                    Operand des = Operand("t" + std::to_string(tmp_cnt++), t);
                    buffer.push_back(new Instruction({arr.operand, tmp2, des, Operator::load}));
                    root->v = des.name;
                }
            }
        }
    }
}


// PrimaryExp -> '(' Exp ')' | LVal | Number
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp* root, vector<ir::Instruction*>& buffer) {
    if (root->children[0]->type == NodeType::EXP){
        ANALYSIS(exp, Exp, 1);
        COPY_EXP_NODE(exp, root);
    }else if (root->children[0]->type == NodeType::LVAL){
        ANALYSIS(lval, LVal, 0);
        COPY_EXP_NODE(lval, root);
    }else{
        root->is_computable = true;
        auto number_tk = dynamic_cast<Term*>(root->children[0]->children[0])->token;  //拿到Number节点对应终结符的token
        root->t = (number_tk.type==TokenType::INTLTR) ? Type::IntLiteral : Type::FloatLiteral;      // t属性为终结符的类型
        if (root->t == Type::IntLiteral){
            root->v = trans2ten(number_tk.value);     // v属性为终结符的值
        }else{
            root->v = number_tk.value;
        }
    }
}


// UnaryExp -> PrimaryExp | UnaryOp UnaryExp
void frontend::Analyzer::analysisUnaryExp(UnaryExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisMulExp(MulExp*, vector<ir::Instruction*>&) {
    TODO;
}
void frontend::Analyzer::analysisAddExp(AddExp*, vector<ir::Instruction*>&) {
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
void frontend::Analyzer::analysisConstExp(ConstExp*, vector<ir::Instruction*>&) {
    TODO;
}