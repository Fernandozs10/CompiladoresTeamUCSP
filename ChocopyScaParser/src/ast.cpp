#include "ast.h"

#include <map>
#include <set>

NodePtr newNode(const std::string& kind, const std::string& text, int line) {
    NodePtr node = std::make_shared<Node>();
    node->kind = kind;
    node->text = text;
    node->line = line;
    return node;
}

NodePtr newTokenNode(const std::string& tokenType, const std::string& lexeme, int line) {
    return newNode(tokenType, lexeme, line);
}

// Los tokens de operadores llegan con su nombre (PLUS, EQ...); en el AST se guarda el símbolo
static std::string operatorSymbol(const std::string& tokenType) {
    static const std::map<std::string, std::string> symbols = {
        {"PLUS", "+"}, {"MINUS", "-"}, {"MULT", "*"}, {"INTDIV", "//"}, {"MOD", "%"},
        {"EQ", "=="}, {"NE", "!="}, {"LT", "<"}, {"GT", ">"}, {"LE", "<="}, {"GE", ">="},
        {"IS", "is"}, {"AND", "and"}, {"OR", "or"}, {"NOT", "not"}};
    const auto found = symbols.find(tokenType);
    return found == symbols.end() ? tokenType : found->second;
}

// Cuelga de "first" las operaciones pendientes de "tail", de izquierda a derecha.
// Cada operación recibe como primer hijo todo lo construido hasta ese momento:
//   a.b(c)[d]  ->  Index( Call( Attr(a, b), c ), d )
static NodePtr applyTail(const NodePtr& first, const NodePtr& tail) {
    NodePtr result = first;
    for (const NodePtr& operation : tail->children) {
        operation->children.insert(operation->children.begin(), result);
        result = operation;
    }
    return result;
}

// a = b = 1  ->  Assign [a, b, 1]
static NodePtr makeAssign(const NodePtr& target, const NodePtr& rest, int line) {
    NodePtr assign = newNode("Assign", "", line);
    assign->children.push_back(target);
    if (rest->kind == "Assign") {
        for (const NodePtr& child : rest->children) assign->children.push_back(child);
    } else {
        assign->children.push_back(rest);
    }
    return assign;
}

NodePtr buildNode(const std::string& left, const std::vector<std::string>& right,
                  const std::vector<NodePtr>& children) {
    const std::vector<NodePtr>& c = children;
    const size_t count = right.size();

    // Listas: "lista -> lista elemento" o "lista -> elemento" (a veces con una coma en medio)
    static const std::set<std::string> listNames = {
        "def_list", "stmt_list", "stmt_list_plus", "class_body_defs", "func_body_decls",
        "typed_var_list", "elif_list", "expr_list"};
    if (listNames.count(left)) {
        if (count == 0) return newNode("List");
        if (left == "elif_list") {   // elif_list ELIF expr COLON block
            NodePtr elif = newNode("Elif", "", c[1]->line);
            elif->children = {c[2], c[4]};
            c[0]->children.push_back(elif);
            return c[0];
        }
        NodePtr list = (right[0] == left) ? c[0] : newNode("List");
        list->children.push_back(c.back());
        return list;
    }

    // Partes opcionales: sin contenido son una lista vacía (o nada)
    if (left == "typed_var_list_opt" || left == "expr_list_opt") return count == 0 ? newNode("List") : c[0];
    if (left == "return_type_opt" || left == "expr_opt") return count == 0 ? nullptr : c[count - 1];

    // Operaciones pendientes de un postfijo (.x, [i], (args)) o de una comparación
    if (left == "postfix_tail" || left == "comparison_tail") {
        NodePtr tail = (count == 0) ? newNode("List") : c.back();
        if (count == 0) return tail;
        NodePtr operation;
        if (left == "comparison_tail") {
            operation = newNode("BinOp", operatorSymbol(c[0]->text), c[0]->line);
            operation->children = {c[1]};
        } else if (right[0] == "DOT") {
            operation = newNode("Attr", c[1]->text, c[0]->line);
        } else if (right[0] == "LBRACKET") {
            operation = newNode("Index", "", c[0]->line);
            operation->children = {c[1]};
        } else {   // LPAREN
            operation = newNode("Call", "", c[0]->line);
            operation->children = c[1]->children;
        }
        tail->children.insert(tail->children.begin(), operation);
        return tail;
    }
    if (left == "postfix" || left == "comparison") return applyTail(c[0], c[1]);

    // Programa, clases y funciones
    if (left == "program") {
        NodePtr program = newNode("Program", "", 1);
        program->children = c[0]->children;
        for (const NodePtr& statement : c[1]->children) program->children.push_back(statement);
        return program;
    }
    if (left == "class_body") return count == 2 ? newNode("List") : c[0];
    if (left == "class_def") {   // CLASS ID ( ID ) : NEWLINE INDENT class_body DEDENT
        NodePtr classDef = newNode("ClassDef", c[1]->text, c[0]->line);
        classDef->type = c[3]->text;
        classDef->children = c[8]->children;
        return classDef;
    }
    if (left == "func_def") {   // DEF ID ( params ) ret : NEWLINE INDENT func_body DEDENT
        NodePtr funcDef = newNode("FuncDef", c[1]->text, c[0]->line);
        funcDef->type = c[5] ? c[5]->text : "<None>";
        NodePtr params = newNode("Params", "", c[0]->line);
        params->children = c[3]->children;
        funcDef->children = {params, c[9]};
        return funcDef;
    }
    if (left == "func_body") {
        NodePtr body = newNode("Body", "", c[0]->line);
        body->children = c[0]->children;
        for (const NodePtr& statement : c[1]->children) body->children.push_back(statement);
        return body;
    }

    // Declaraciones
    if (left == "typed_var") {   // TYPED_ID COLON type
        NodePtr param = newNode("Param", c[0]->text, c[0]->line);
        param->type = c[2]->text;
        return param;
    }
    if (left == "type") {
        if (count == 3) return newNode("Type", "[" + c[1]->text + "]", c[0]->line);   // [type]
        return newNode("Type", c[0]->text, c[0]->line);
    }
    if (left == "var_def") {   // typed_var ASSIGN literal NEWLINE
        NodePtr varDef = newNode("VarDef", c[0]->text, c[0]->line);
        varDef->type = c[0]->type;
        varDef->children = {c[2]};
        return varDef;
    }
    if (left == "global_decl") return newNode("Global", c[1]->text, c[0]->line);
    if (left == "nonlocal_decl") return newNode("Nonlocal", c[1]->text, c[0]->line);

    // Sentencias
    if (left == "stmt") {
        if (count == 2) return c[0];   // simple_stmt NEWLINE
        if (right[0] == "IF") {        // IF expr COLON block elif_list else_opt
            NodePtr ifNode = newNode("If", "", c[0]->line);
            ifNode->children = {c[1], c[3]};
            for (const NodePtr& elif : c[4]->children) ifNode->children.push_back(elif);
            if (c[5]) ifNode->children.push_back(c[5]);
            return ifNode;
        }
        if (right[0] == "WHILE") {     // WHILE expr COLON block
            NodePtr whileNode = newNode("While", "", c[0]->line);
            whileNode->children = {c[1], c[3]};
            return whileNode;
        }
        NodePtr forNode = newNode("For", c[1]->text, c[0]->line);   // FOR ID IN expr COLON block
        forNode->children = {c[3], c[5]};
        return forNode;
    }
    if (left == "else_opt") {
        if (count == 0) return nullptr;
        NodePtr elseNode = newNode("Else", "", c[0]->line);
        elseNode->children = {c[2]};
        return elseNode;
    }
    if (left == "block") {   // NEWLINE INDENT stmt_list_plus DEDENT
        NodePtr block = newNode("Block", "", c[1]->line);
        block->children = c[2]->children;
        return block;
    }
    if (left == "simple_stmt") {
        if (right[0] == "PASS") return newNode("Pass", "", c[0]->line);
        if (right[0] == "RETURN") {
            NodePtr returnNode = newNode("Return", "", c[0]->line);
            if (c[1]) returnNode->children = {c[1]};
            return returnNode;
        }
        return c[0];
    }
    if ((left == "assignment_stmt" || left == "assignment_tail") && count == 3) {
        return makeAssign(c[0], c[2], c[1]->line);
    }

    // Expresiones
    if (left == "conditional_expr" && count == 5) {   // or_expr IF expr ELSE conditional_expr
        NodePtr ifExpr = newNode("IfExpr", "", c[1]->line);
        ifExpr->children = {c[0], c[2], c[4]};
        return ifExpr;
    }
    if ((left == "or_expr" || left == "and_expr" || left == "arith_expr" || left == "term") && count == 3) {
        NodePtr binOp = newNode("BinOp", operatorSymbol(c[1]->text), c[1]->line);
        binOp->children = {c[0], c[2]};
        return binOp;
    }
    if ((left == "not_expr" || left == "unary") && count == 2) {
        NodePtr unOp = newNode("UnOp", operatorSymbol(c[0]->kind), c[0]->line);
        unOp->children = {c[1]};
        return unOp;
    }
    if (left == "primary") {
        if (count == 1 && right[0] == "ID") return newNode("Id", c[0]->text, c[0]->line);
        if (count == 3 && right[0] == "LBRACKET") {
            NodePtr list = newNode("ListExpr", "", c[0]->line);
            list->children = c[1]->children;
            return list;
        }
        if (count == 3) return c[1];   // ( expr )
        return c[0];                   // literal
    }
    if (left == "literal") {
        const std::string& token = right[0];
        if (token == "INTEGER") return newNode("Int", c[0]->text, c[0]->line);
        if (token == "TRUE" || token == "FALSE") {
            return newNode("Bool", token == "TRUE" ? "True" : "False", c[0]->line);
        }
        if (token == "NONE") return newNode("None", "", c[0]->line);
        return newNode("Str", c[0]->text, c[0]->line);   // STRING o IDSTRING
    }

    // Reglas que solo pasan el nodo de su único símbolo (expr -> conditional_expr, def -> var_def...)
    if (count == 1) return c[0];

    NodePtr generic = newNode(left);
    generic->children = c;
    return generic;
}

void printAst(const NodePtr& node, std::ostream& out, int depth) {
    if (!node) return;
    out << std::string(static_cast<size_t>(depth) * 2, ' ') << node->kind;
    if (!node->text.empty()) out << ' ' << node->text;
    if (!node->type.empty()) out << " : " << node->type;
    if (node->line > 0) out << "  (línea " << node->line << ")";
    out << '\n';
    for (const NodePtr& child : node->children) printAst(child, out, depth + 1);
}
