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

// los tokens de operadores llegan con su nombre (PLUS, EQ...); en el AST se guarda el simbolo
static std::string operatorSymbol(const std::string& tokenType) {
    static const std::map<std::string, std::string> symbols = {
        {"PLUS", "+"}, {"MINUS", "-"}, {"MULT", "*"}, {"INTDIV", "//"}, {"MOD", "%"},
        {"EQ", "=="}, {"NE", "!="}, {"LT", "<"}, {"GT", ">"}, {"LE", "<="}, {"GE", ">="},
        {"IS", "is"}, {"AND", "and"}, {"OR", "or"}, {"NOT", "not"}};
    const auto found = symbols.find(tokenType);
    return found == symbols.end() ? tokenType : found->second;
}

// cuelga de "firstOperand" las operaciones pendientes de "pendingOperations", de izquierda a derecha.
// cada operacion recibe como primer hijo todo lo construido hasta ese momento:
//   a.b(c)[d]  ->  Index( Call( Attr(a, b), c ), d )
static NodePtr applyPendingOperations(const NodePtr& firstOperand, const NodePtr& pendingOperations) {
    NodePtr builtNode = firstOperand;
    for (const NodePtr& pendingOperation : pendingOperations->children) {
        pendingOperation->children.insert(pendingOperation->children.begin(), builtNode);
        builtNode = pendingOperation;
    }
    return builtNode;
}

// a = b = 1  ->  Assign [a, b, 1]
static NodePtr makeAssignNode(const NodePtr& firstTarget, const NodePtr& restOfAssignment, int line) {
    NodePtr assign = newNode("Assign", "", line);
    assign->children.push_back(firstTarget);
    if (restOfAssignment->kind == "Assign") {
        for (const NodePtr& child : restOfAssignment->children) assign->children.push_back(child);
    } else {
        assign->children.push_back(restOfAssignment);
    }
    return assign;
}

NodePtr buildNodeFromProduction(const std::string& lhsSymbol, const std::vector<std::string>& rhsSymbols,
                                const std::vector<NodePtr>& rhsNodes) {
    const size_t rhsSize = rhsSymbols.size();

    // listas: "lista -> lista elemento" o "lista -> elemento" (a veces con una coma en medio)
    static const std::set<std::string> listSymbolNames = {
        "def_list", "stmt_list", "stmt_list_plus", "class_body_defs", "func_body_decls",
        "typed_var_list", "elif_list", "expr_list"};
    if (listSymbolNames.count(lhsSymbol)) {
        if (rhsSize == 0) return newNode("List");
        if (lhsSymbol == "elif_list") {   // elif_list ELIF expr COLON block
            NodePtr elif = newNode("Elif", "", rhsNodes[1]->line);
            elif->children = {rhsNodes[2], rhsNodes[4]};
            rhsNodes[0]->children.push_back(elif);
            return rhsNodes[0];
        }
        NodePtr listNode = (rhsSymbols[0] == lhsSymbol) ? rhsNodes[0] : newNode("List");
        listNode->children.push_back(rhsNodes.back());
        return listNode;
    }

    // partes opcionales: sin contenido son una lista vacia (o nada)
    if (lhsSymbol == "typed_var_list_opt" || lhsSymbol == "expr_list_opt") return rhsSize == 0 ? newNode("List") : rhsNodes[0];
    if (lhsSymbol == "return_type_opt" || lhsSymbol == "expr_opt") return rhsSize == 0 ? nullptr : rhsNodes[rhsSize - 1];

    // operaciones pendientes de un postfijo (.x, [i], (args)) o de una comparacion
    if (lhsSymbol == "postfix_tail" || lhsSymbol == "comparison_tail") {
        NodePtr pendingOperations = (rhsSize == 0) ? newNode("List") : rhsNodes.back();
        if (rhsSize == 0) return pendingOperations;
        NodePtr pendingOperation;
        if (lhsSymbol == "comparison_tail") {
            pendingOperation = newNode("BinOp", operatorSymbol(rhsNodes[0]->text), rhsNodes[0]->line);
            pendingOperation->children = {rhsNodes[1]};
        } else if (rhsSymbols[0] == "DOT") {
            pendingOperation = newNode("Attr", rhsNodes[1]->text, rhsNodes[0]->line);
        } else if (rhsSymbols[0] == "LBRACKET") {
            pendingOperation = newNode("Index", "", rhsNodes[0]->line);
            pendingOperation->children = {rhsNodes[1]};
        } else {   // LPAREN
            pendingOperation = newNode("Call", "", rhsNodes[0]->line);
            pendingOperation->children = rhsNodes[1]->children;
        }
        pendingOperations->children.insert(pendingOperations->children.begin(), pendingOperation);
        return pendingOperations;
    }
    if (lhsSymbol == "postfix" || lhsSymbol == "comparison") return applyPendingOperations(rhsNodes[0], rhsNodes[1]);

    // programa, clases y funciones
    if (lhsSymbol == "program") {
        NodePtr programNode = newNode("Program", "", 1);
        programNode->children = rhsNodes[0]->children;
        for (const NodePtr& statement : rhsNodes[1]->children) programNode->children.push_back(statement);
        return programNode;
    }
    if (lhsSymbol == "class_body") return rhsSize == 2 ? newNode("List") : rhsNodes[0];
    if (lhsSymbol == "class_def") {   // CLASS ID ( ID ) : NEWLINE INDENT class_body DEDENT
        NodePtr classDefNode = newNode("ClassDef", rhsNodes[1]->text, rhsNodes[0]->line);
        classDefNode->type = rhsNodes[3]->text;
        classDefNode->children = rhsNodes[8]->children;
        return classDefNode;
    }
    if (lhsSymbol == "func_def") {   // DEF ID ( params ) ret : NEWLINE INDENT func_body DEDENT
        NodePtr funcDefNode = newNode("FuncDef", rhsNodes[1]->text, rhsNodes[0]->line);
        funcDefNode->type = rhsNodes[5] ? rhsNodes[5]->text : "<None>";
        NodePtr paramsNode = newNode("Params", "", rhsNodes[0]->line);
        paramsNode->children = rhsNodes[3]->children;
        funcDefNode->children = {paramsNode, rhsNodes[9]};
        return funcDefNode;
    }
    if (lhsSymbol == "func_body") {
        NodePtr bodyNode = newNode("Body", "", rhsNodes[0]->line);
        bodyNode->children = rhsNodes[0]->children;
        for (const NodePtr& statement : rhsNodes[1]->children) bodyNode->children.push_back(statement);
        return bodyNode;
    }

    // declaraciones
    if (lhsSymbol == "typed_var") {   // TYPED_ID COLON type
        NodePtr paramNode = newNode("Param", rhsNodes[0]->text, rhsNodes[0]->line);
        paramNode->type = rhsNodes[2]->text;
        return paramNode;
    }
    if (lhsSymbol == "type") {
        if (rhsSize == 3) return newNode("Type", "[" + rhsNodes[1]->text + "]", rhsNodes[0]->line);   // [type]
        return newNode("Type", rhsNodes[0]->text, rhsNodes[0]->line);
    }
    if (lhsSymbol == "var_def") {   // typed_var ASSIGN literal NEWLINE
        NodePtr varDefNode = newNode("VarDef", rhsNodes[0]->text, rhsNodes[0]->line);
        varDefNode->type = rhsNodes[0]->type;
        varDefNode->children = {rhsNodes[2]};
        return varDefNode;
    }
    if (lhsSymbol == "global_decl") return newNode("Global", rhsNodes[1]->text, rhsNodes[0]->line);
    if (lhsSymbol == "nonlocal_decl") return newNode("Nonlocal", rhsNodes[1]->text, rhsNodes[0]->line);

    // sentencias
    if (lhsSymbol == "stmt") {
        if (rhsSize == 2) return rhsNodes[0];   // simple_stmt NEWLINE
        if (rhsSymbols[0] == "IF") {        // IF expr COLON block elif_list else_opt
            NodePtr ifNode = newNode("If", "", rhsNodes[0]->line);
            ifNode->children = {rhsNodes[1], rhsNodes[3]};
            for (const NodePtr& elif : rhsNodes[4]->children) ifNode->children.push_back(elif);
            if (rhsNodes[5]) ifNode->children.push_back(rhsNodes[5]);
            return ifNode;
        }
        if (rhsSymbols[0] == "WHILE") {     // WHILE expr COLON block
            NodePtr whileNode = newNode("While", "", rhsNodes[0]->line);
            whileNode->children = {rhsNodes[1], rhsNodes[3]};
            return whileNode;
        }
        NodePtr forNode = newNode("For", rhsNodes[1]->text, rhsNodes[0]->line);   // FOR ID IN expr COLON block
        forNode->children = {rhsNodes[3], rhsNodes[5]};
        return forNode;
    }
    if (lhsSymbol == "else_opt") {
        if (rhsSize == 0) return nullptr;
        NodePtr elseNode = newNode("Else", "", rhsNodes[0]->line);
        elseNode->children = {rhsNodes[2]};
        return elseNode;
    }
    if (lhsSymbol == "block") {   // NEWLINE INDENT stmt_list_plus DEDENT
        NodePtr blockNode = newNode("Block", "", rhsNodes[1]->line);
        blockNode->children = rhsNodes[2]->children;
        return blockNode;
    }
    if (lhsSymbol == "simple_stmt") {
        if (rhsSymbols[0] == "PASS") return newNode("Pass", "", rhsNodes[0]->line);
        if (rhsSymbols[0] == "RETURN") {
            NodePtr returnNode = newNode("Return", "", rhsNodes[0]->line);
            if (rhsNodes[1]) returnNode->children = {rhsNodes[1]};
            return returnNode;
        }
        return rhsNodes[0];
    }
    if ((lhsSymbol == "assignment_stmt" || lhsSymbol == "assignment_tail") && rhsSize == 3) {
        return makeAssignNode(rhsNodes[0], rhsNodes[2], rhsNodes[1]->line);
    }

    // expresiones
    if (lhsSymbol == "conditional_expr" && rhsSize == 5) {   // or_expr IF expr ELSE conditional_expr
        NodePtr ifExprNode = newNode("IfExpr", "", rhsNodes[1]->line);
        ifExprNode->children = {rhsNodes[0], rhsNodes[2], rhsNodes[4]};
        return ifExprNode;
    }
    if ((lhsSymbol == "or_expr" || lhsSymbol == "and_expr" || lhsSymbol == "arith_expr" || lhsSymbol == "term") && rhsSize == 3) {
        NodePtr binaryOpNode = newNode("BinOp", operatorSymbol(rhsNodes[1]->text), rhsNodes[1]->line);
        binaryOpNode->children = {rhsNodes[0], rhsNodes[2]};
        return binaryOpNode;
    }
    if ((lhsSymbol == "not_expr" || lhsSymbol == "unary") && rhsSize == 2) {
        NodePtr unaryOpNode = newNode("UnOp", operatorSymbol(rhsNodes[0]->kind), rhsNodes[0]->line);
        unaryOpNode->children = {rhsNodes[1]};
        return unaryOpNode;
    }
    if (lhsSymbol == "primary") {
        if (rhsSize == 1 && rhsSymbols[0] == "ID") return newNode("Id", rhsNodes[0]->text, rhsNodes[0]->line);
        if (rhsSize == 3 && rhsSymbols[0] == "LBRACKET") {
            NodePtr listNode = newNode("ListExpr", "", rhsNodes[0]->line);
            listNode->children = rhsNodes[1]->children;
            return listNode;
        }
        if (rhsSize == 3) return rhsNodes[1];   // ( expr )
        return rhsNodes[0];                   // literal
    }
    if (lhsSymbol == "literal") {
        const std::string& literalToken = rhsSymbols[0];
        if (literalToken == "INTEGER") return newNode("Int", rhsNodes[0]->text, rhsNodes[0]->line);
        if (literalToken == "TRUE" || literalToken == "FALSE") {
            return newNode("Bool", literalToken == "TRUE" ? "True" : "False", rhsNodes[0]->line);
        }
        if (literalToken == "NONE") return newNode("None", "", rhsNodes[0]->line);
        return newNode("Str", rhsNodes[0]->text, rhsNodes[0]->line);   // STRING o IDSTRING
    }

    // reglas que solo pasan el nodo de su unico simbolo (expr -> conditional_expr, def -> var_def...)
    if (rhsSize == 1) return rhsNodes[0];

    NodePtr genericNode = newNode(lhsSymbol);
    genericNode->children = rhsNodes;
    return genericNode;
}

void printAst(const NodePtr& node, std::ostream& out, int depth) {
    if (!node) return;
    out << std::string(static_cast<size_t>(depth) * 2, ' ') << node->kind;
    if (!node->text.empty()) out << ' ' << node->text;
    if (!node->type.empty()) out << " : " << node->type;
    if (node->line > 0) out << "  (linea " << node->line << ")";
    out << '\n';
    for (const NodePtr& child : node->children) printAst(child, out, depth + 1);
}
