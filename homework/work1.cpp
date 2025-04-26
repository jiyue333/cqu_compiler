/*
Exp -> AddExp

    Exp.v

Number -> IntConst | floatConst

PrimaryExp -> '(' Exp ')' | Number
    PrimaryExp.v

UnaryExp -> PrimaryExp | UnaryOp UnaryExp
    UnaryExp.v

UnaryOp -> '+' | '-'

MulExp -> UnaryExp { ('*' | '/') UnaryExp }
    MulExp.v

AddExp -> MulExp { ('+' | '-') MulExp }
    AddExp.v
*/
#include <map>
#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <queue>

#define TODO assert(0 && "TODO")
// #define DEBUG_DFA
// #define DEBUG_PARSER

// enumerate for Status
enum class State
{
    Empty,      // space, \n, \r ...
    IntLiteral, // int literal, like '1' '01900', '0xAB', '0b11001'
    op          // operators and '(', ')'
};
std::string toString(State s)
{
    switch (s)
    {
    case State::Empty:
        return "Empty";
    case State::IntLiteral:
        return "IntLiteral";
    case State::op:
        return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

// enumerate for Token type
enum class TokenType
{
    INTLTR,  // int literal
    PLUS,    // +
    MINU,    // -
    MULT,    // *
    DIV,     // /
    LPARENT, // (
    RPARENT, // )
};
std::string toString(TokenType type)
{
    switch (type)
    {
    case TokenType::INTLTR:
        return "INTLTR";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINU:
        return "MINU";
    case TokenType::MULT:
        return "MULT";
    case TokenType::DIV:
        return "DIV";
    case TokenType::LPARENT:
        return "LPARENT";
    case TokenType::RPARENT:
        return "RPARENT";
    default:
        assert(0 && "invalid token type");
        break;
    }
    return "";
}

// definition of Token
struct Token
{
    TokenType type;
    std::string value;
};

// definition of DFA
struct DFA
{
    /**
     * @brief constructor, set the init state to State::Empty
     */
    DFA();

    /**
     * @brief destructor
     */
    ~DFA();

    // the meaning of copy and assignment for a DFA is not clear, so we do not allow them
    DFA(const DFA &) = delete;            // copy constructor
    DFA &operator=(const DFA &) = delete; // assignment

    /**
     * @brief take a char as input, change state to next state, and output a Token if necessary
     * @param[in] input: the input character
     * @param[out] buf: the output Token buffer
     * @return  return true if a Token is produced, the buf is valid then
     */
    bool next(char input, Token &buf);

    /**
     * @brief reset the DFA state to begin
     */
    void reset();

private:
    State cur_state;     // record current state of the DFA
    std::string cur_str; // record input characters
};

DFA::DFA() : cur_state(State::Empty), cur_str() {}

DFA::~DFA() {}

// helper function, you are not require to implement these, but they may be helpful
bool isoperator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')';
}

TokenType get_op_type(std::string s)
{
    if (s == "+")
        return TokenType::PLUS;
    else if (s == "-")
        return TokenType::MINU;
    else if (s == "*")
        return TokenType::MULT;
    else if (s == "/")
        return TokenType::DIV;
    else if (s == "(")
        return TokenType::LPARENT;
    else if (s == ")")
        return TokenType::RPARENT;
    else
        // assert(0 && "invalid operator");
        std::cout << "invalid operator" << s << std::endl;
    return TokenType::PLUS;
}

bool DFA::next(char input, Token &buf)
{
    // std::cout << "[DEBUG]input: " << input << " [DEBUG]cur_state: " << toString(cur_state) << " [DEBUG]cur_str: " << cur_str << std::endl;
    if (cur_state == State::Empty)
    {
        if (isdigit(input))
        {
            cur_state = State::IntLiteral;
            cur_str += input;
        }
        else if (isoperator(input))
        {
            buf.type = get_op_type(std::string(1, input));
            buf.value = std::string(1, input);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (cur_state == State::IntLiteral)
    {
        if (isdigit(input) || (input >= 'a' && input <= 'z' || input >= 'A' && input <= 'Z'))
        {
            cur_str += input;
        }
        else if (isoperator(input))
        {
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;
            reset();
            cur_state = State::op;
            cur_str += input;
            // std::cout << "[DEBUG]IntLiteral: " << cur_str << std::endl;
            return true;
        }
        else
        {
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;
            reset();
            return true;
        }
    }
    else if (cur_state == State::op)
    {
        buf.type = get_op_type(cur_str);
        buf.value = cur_str;
        reset();
        if (isdigit(input))
        {
            cur_state = State::IntLiteral;
            cur_str += input;
        }
        else if (isoperator(input))
        {
            cur_state = State::op;
            cur_str += input;
        }
        return true;
    }
    return false;
}

void DFA::reset()
{
    cur_state = State::Empty;
    cur_str = "";
}

// hw2
enum class NodeType
{
    TERMINAL, // terminal lexical unit
    EXP,
    NUMBER,
    PRIMARYEXP,
    UNARYEXP,
    UNARYOP,
    MULEXP,
    ADDEXP,
    NONE
};
std::string toString(NodeType nt)
{
    switch (nt)
    {
    case NodeType::TERMINAL:
        return "Terminal";
    case NodeType::EXP:
        return "Exp";
    case NodeType::NUMBER:
        return "Number";
    case NodeType::PRIMARYEXP:
        return "PrimaryExp";
    case NodeType::UNARYEXP:
        return "UnaryExp";
    case NodeType::UNARYOP:
        return "UnaryOp";
    case NodeType::MULEXP:
        return "MulExp";
    case NodeType::ADDEXP:
        return "AddExp";
    case NodeType::NONE:
        return "NONE";
    default:
        assert(0 && "invalid node type");
        break;
    }
    return "";
}

// tree node basic class
struct AstNode
{
    int value;
    NodeType type;                   // the node type
    AstNode *parent;                 // the parent node
    std::vector<AstNode *> children; // children of node

    /**
     * @brief constructor
     */
    AstNode(NodeType t = NodeType::NONE, AstNode *p = nullptr) : type(t), parent(p), value(0) {}

    /**
     * @brief destructor
     */
    virtual ~AstNode()
    {
        for (auto child : children)
        {
            delete child;
        }
    }

    // rejcet copy and assignment
    AstNode(const AstNode &) = delete;
    AstNode &operator=(const AstNode &) = delete;
};

// definition of Parser
// a parser should take a token stream as input, then parsing it, output a AST
struct Parser
{
    uint32_t index; // current token index
    const std::vector<Token> &token_stream;

    /**
     * @brief constructor
     * @param tokens: the input token_stream
     */
    Parser(const std::vector<Token> &tokens) : index(0), token_stream(tokens) {}

    /**
     * @brief destructor
     */
    ~Parser() {}

    /**
     * @brief creat the abstract syntax tree
     * @return the root of abstract syntax tree
     */
    AstNode *get_abstract_syntax_tree() {
        for(auto token : token_stream) {
            //std::cout << "[DEBUG]token_type: " << toString(token.type) << " token_value: " << token.value << std::endl;
        }
        return parse_Exp();
    }

    AstNode *parse_Exp() {
    #ifdef DEBUG_PARSER
        std::cout << "[DEBUG]parse_Exp" << "index: " << index << " token_type: " << toString(token_stream[index].type) << " token_value: " << token_stream[index].value << std::endl;
    #endif
        AstNode* node = new AstNode(NodeType::EXP);
        AstNode* addExp = parse_AddExp(); 
        if (addExp) {
            node->children.push_back(addExp);
            node->value = addExp->value; 
            return node;
        }
        return nullptr;
    }

    // AddExp -> MulExp { ('+' | '-') MulExp }
    AstNode *parse_AddExp() {
    #ifdef DEBUG_PARSER
        std::cout << "[DEBUG]parse_AddExp" << "index: " << index << " token_type: " << toString(token_stream[index].type) << " token_value: " << token_stream[index].value << std::endl;
    #endif
        AstNode* node = new AstNode(NodeType::ADDEXP);
        AstNode* left = parse_MulExp();
        if (left) {
            node->children.push_back(left);
            node->value = left->value;
            left = node;
        }
        while (true) {
            Token token = peek(*this);
            if (token.type == TokenType::PLUS || token.type == TokenType::MINU) {
                AstNode* newNode = new AstNode(NodeType::ADDEXP);
                advance(*this);
                AstNode* right = parse_MulExp();
                if (right) {
                    newNode->children.push_back(right);
                    newNode->value = token.type == TokenType::PLUS ? left->value + right->value : left->value - right->value;
                    left = newNode;
                } else {
                    break;
                }
            }
            else {
                break;
            }
        }
        return left;
    }

    // MulExp -> UnaryExp { ('*' | '/') UnaryExp }
    AstNode *parse_MulExp() {
     #ifdef DEBUG_PARSER
        std::cout << "[DEBUG]parse_AddExp" << "index: " << index << " token_type: " << toString(token_stream[index].type) << " token_value: " << token_stream[index].value << std::endl;
    #endif
        AstNode* node = new AstNode(NodeType::MULEXP);
        AstNode* left = parse_UnaryExp();
        if (left) {
            node->children.push_back(left);
            node->value = left->value;
            left = node;
        }
        // 左结合，迭代实现
        while (true) {
            Token token = peek(*this);
            if (token.type == TokenType::MULT || token.type == TokenType::DIV) {
                AstNode* newNode = new AstNode(NodeType::MULEXP);
                advance(*this);
                AstNode* right = parse_UnaryExp();
                if (right) {
                    newNode->children.push_back(right);
                    newNode->value = token.type == TokenType::MULT ? left->value * right->value : left->value / right->value;
                    //std::cout << "[DEBUG]newNode->value: " << newNode->value << std::endl;
                    left = newNode;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        return left;
    }

    // UnaryExp -> PrimaryExp | UnaryOp UnaryExp
    AstNode* parse_UnaryExp() {
    #ifdef DEBUG_PARSER
        std::cout << "[DEBUG]parse_UnaryExp" << "index: " << index << " token_type: " << toString(token_stream[index].type) << " token_value: " << token_stream[index].value << std::endl;
    #endif
        AstNode* node = new AstNode(NodeType::UNARYEXP);
        Token token = peek(*this);
        if (token.type == TokenType::PLUS || token.type == TokenType::MINU) {
            advance(*this);
            AstNode* unaryOp = new AstNode(NodeType::UNARYOP);
            unaryOp->value = token.type == TokenType::PLUS ? 1 : -1;
            node->children.push_back(unaryOp);
            AstNode* unaryExp = parse_UnaryExp();
            if (unaryExp) {
                node->children.push_back(unaryExp);
                node->value = unaryOp->value * unaryExp->value;
                return node;
            }
        } else {
            AstNode* primaryExp = parse_PrimaryExp();
            if (primaryExp) {
                node->children.push_back(primaryExp);
                node->value = primaryExp->value;
                return node;
            }
        }
        return node;
    }

    // PrimaryExp -> '(' Exp ')' | Number
    AstNode* parse_PrimaryExp() {
    #ifdef DEBUG_PARSER
        std::cout << "[DEBUG]parse_PrimaryExp" << "index: " << index << " token_type: " << toString(token_stream[index].type) << " token_value: " << token_stream[index].value << std::endl;
    #endif
        Token token = peek(*this);
        if (token.type == TokenType::LPARENT) {
            advance(*this);
            AstNode* exp = parse_Exp();
            if (exp) {
                token = peek(*this);
                if (token.type == TokenType::RPARENT) {
                    advance(*this);
                    return exp;
                }
            }
        } else if (token.type == TokenType::INTLTR) {
            return parse_Number();
        }
        return nullptr;
    }

    // Number -> IntConst | floatConst
    AstNode* parse_Number() { 
    #ifdef DEBUG_PARSER
        std::cout << "[DEBUG]parse_Number" << "index: " << index << " token_type: " << toString(token_stream[index].type) << " token_value: " << token_stream[index].value << std::endl;
    #endif
        Token token = peek(*this);
        if (token.type == TokenType::INTLTR) {
            AstNode* node = new AstNode(NodeType::NUMBER);
            node->value = strtoint(token.value); // convert string to int
            advance(*this);
            return node;
        }
        return nullptr;

    }


    Token peek(Parser &parser)
    {
        if (parser.index >= parser.token_stream.size())
        {
            return {TokenType::RPARENT, ""};
        }
        return parser.token_stream[parser.index];
    }

    void advance(Parser &parser)
    {
        parser.index++;
    }

    int strtoint(std::string s)
    {
        //std::cout << "[DEBUG]strtoint: " << s << " " << std::stoi(s.substr(2), nullptr, 2) << std::endl;
        if (s.size() >= 2) {
            // 二进制: 0b或0B开头
            if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
                return std::stoi(s.substr(2), nullptr, 2);
            }
            else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
                return std::stoi(s, nullptr, 16);
            }
            else if (s[0] == '0') {
                return std::stoi(s, nullptr, 8);
            }
        }
        return std::stoi(s);
    }

    // u can define member funcition of Parser here
    // for debug, u r not required to use this
    // how to use this: in ur local enviroment, defines the macro DEBUG_PARSER and add this function in every parse fuction
    void log(AstNode *node)
    {
#ifdef DEBUG_PARSER
        std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << 'n';
#endif
    }

};

// u can define funcition here


int main()
{
    std::string stdin_str;
    std::getline(std::cin, stdin_str);
    stdin_str += "\n";
    DFA dfa;
    Token tk;
    std::vector<Token> tokens;
    for (size_t i = 0; i < stdin_str.size(); i++)
    {
        if (dfa.next(stdin_str[i], tk))
        {
            tokens.push_back(tk);
        }
    }

    // hw2
    Parser parser(tokens);
    auto root = parser.get_abstract_syntax_tree();
    // u may add function here to analysis the AST, or do this in parsing
    // like get_value(root);
    if(!root) {
        std::cout << "Error: invalid expression" << std::endl;
        return 1;
    }
    std::cout << root->value;

    return 0;
}