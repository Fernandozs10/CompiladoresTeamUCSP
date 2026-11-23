#include "semantic.h"

// Tipo de una expresión que ya tuvo un error: así no se repite el mismo error en cascada
static const std::string ERROR_TYPE = "<error>";
static const std::string NONE_TYPE = "<None>";
static const std::string EMPTY_LIST_TYPE = "[<Empty>]";
static const std::string LIST_OR_STR = "<lista o str>";   // solo para el parámetro de len

static bool isListType(const std::string& type) {
    return type.size() >= 2 && type.front() == '[';
}

static std::string elementTypeOf(const std::string& listType) {
    return listType.substr(1, listType.size() - 2);
}

static bool isBasicType(const std::string& type) {
    return type == "int" || type == "bool" || type == "str";
}

static Symbol makeSymbol(const std::string& name, const std::string& kind, const std::string& type) {
    Symbol symbol;
    symbol.name = name;
    symbol.kind = kind;
    symbol.type = type;
    return symbol;
}

SemanticChecker::SemanticChecker() {
    globalScope = openScope("global", nullptr);
    currentScope = globalScope;

    // Funciones predefinidas
    Symbol print = makeSymbol("print", "func", NONE_TYPE);
    print.paramTypes = {"object"};
    Symbol len = makeSymbol("len", "func", "int");
    len.paramTypes = {LIST_OR_STR};
    Symbol input = makeSymbol("input", "func", "str");
    globalScope->insert(print);
    globalScope->insert(len);
    globalScope->insert(input);
}

void SemanticChecker::error(int line, const std::string& message) {
    errors.push_back("Error semántico (línea " + std::to_string(line) + "): " + message);
}

Scope* SemanticChecker::openScope(const std::string& name, Scope* parent) {
    allScopes.push_back(std::make_unique<Scope>(name, parent));
    return allScopes.back().get();
}

std::vector<std::string> SemanticChecker::check(const NodePtr& program) {
    declareClasses(program);
    declareNames(program);
    checkBody(program);
    return errors;
}

// ---------------------------------------------------------------------------------------
// Tablas de símbolos
// ---------------------------------------------------------------------------------------

// Primero se registran todas las clases, para poder usarlas como tipo antes de su definición
void SemanticChecker::declareClasses(const NodePtr& program) {
    for (const NodePtr& node : program->children) {
        if (node->kind != "ClassDef") continue;
        Symbol classSymbol = makeSymbol(node->text, "class", node->text);
        classSymbol.parentClass = node->type;
        classSymbol.members = openScope(node->text, nullptr);
        if (!globalScope->insert(classSymbol)) {
            error(node->line, "'" + node->text + "' ya está declarado en este ámbito");
        }
    }
}

// Inserta en la tabla actual los nombres que declara el contenedor (Program o Body)
void SemanticChecker::declareNames(const NodePtr& container) {
    for (const NodePtr& node : container->children) {
        if (node->kind == "VarDef") {
            validateType(node->type, node->line);
            if (!currentScope->insert(makeSymbol(node->text, "var", node->type))) {
                error(node->line, "'" + node->text + "' ya está declarado en este ámbito");
            }
            // el valor inicial es un literal: se comprueba que coincida con el tipo declarado
            const NodePtr& literal = node->children[0];
            literal->type = (literal->kind == "Int") ? "int" : (literal->kind == "Bool") ? "bool"
                          : (literal->kind == "Str") ? "str" : NONE_TYPE;
            if (!isAssignable(literal->type, node->type)) {
                error(node->line, "'" + node->text + "' se declaró como " + node->type +
                                  " pero su valor inicial es " + literal->type);
            }
        } else if (node->kind == "FuncDef") {
            Symbol function = makeSymbol(node->text, "func", node->type);
            validateType(node->type, node->line);
            for (const NodePtr& param : node->children[0]->children) {
                validateType(param->type, param->line);
                function.paramTypes.push_back(param->type);
            }
            if (!currentScope->insert(function)) {
                error(node->line, "'" + node->text + "' ya está declarado en este ámbito");
            }
        } else if (node->kind == "ClassDef") {
            Symbol* classSymbol = globalScope->lookupHere(node->text);
            if (classSymbol == nullptr || classSymbol->members == nullptr) continue;
            if (!node->type.empty() && node->type != "object" &&
                (globalScope->lookupHere(node->type) == nullptr ||
                 globalScope->lookupHere(node->type)->kind != "class")) {
                error(node->line, "la clase padre '" + node->type + "' no existe");
            }
            // los atributos y métodos van a la tabla de la clase
            Scope* outerScope = currentScope;
            currentScope = classSymbol->members;
            declareNames(node);
            currentScope = outerScope;
        }
    }
}

// global x / nonlocal x: la tabla de la función recibe una entrada que apunta a la variable externa
void SemanticChecker::declareOuterNames(const NodePtr& body) {
    for (const NodePtr& node : body->children) {
        if (node->kind != "Global" && node->kind != "Nonlocal") continue;

        Symbol* target = nullptr;
        if (node->kind == "Global") {
            target = globalScope->lookupHere(node->text);
        } else {   // nonlocal: una variable de una función que contiene a esta (no la global)
            for (Scope* scope = currentScope->parent; scope != nullptr && scope != globalScope; scope = scope->parent) {
                target = scope->lookupHere(node->text);
                if (target != nullptr) break;
            }
        }
        if (target != nullptr && target->kind == "ref") target = target->refersTo;
        if (target == nullptr || (target->kind != "var" && target->kind != "param")) {
            std::string where = node->kind == "Global" ? "global" : "de una función externa";
            error(node->line, "'" + node->text + "' no es una variable " + where);
            continue;
        }

        Symbol reference = makeSymbol(node->text, "ref", target->type);
        reference.refersTo = target;
        if (!currentScope->insert(reference)) {
            error(node->line, "'" + node->text + "' ya está declarado en este ámbito");
        }
    }
}

// ---------------------------------------------------------------------------------------
// Recorrido: definiciones y sentencias
// ---------------------------------------------------------------------------------------

// Revisa los cuerpos de las funciones y las sentencias (los nombres ya fueron declarados)
void SemanticChecker::checkBody(const NodePtr& container) {
    for (const NodePtr& node : container->children) {
        if (node->kind == "FuncDef") {
            checkFunction(node);
        } else if (node->kind == "ClassDef") {
            for (const NodePtr& member : node->children) {
                if (member->kind == "FuncDef") checkFunction(member);
            }
        } else if (node->kind != "VarDef" && node->kind != "Global" && node->kind != "Nonlocal") {
            checkStatement(node);
        }
    }
}

void SemanticChecker::checkFunction(const NodePtr& function) {
    const NodePtr& params = function->children[0];
    const NodePtr& body = function->children[1];

    // Se guarda lo del scope de afuera para restaurarlo al terminar
    Scope* outerScope = currentScope;
    const std::string outerReturnType = currentReturnType;
    const bool outerInsideFunction = insideFunction;

    currentScope = openScope(function->text, outerScope);   // tabla nueva: su parent es la actual
    currentReturnType = function->type;
    insideFunction = true;

    for (const NodePtr& param : params->children) {
        if (!currentScope->insert(makeSymbol(param->text, "param", param->type))) {
            error(param->line, "el parámetro '" + param->text + "' está repetido");
        }
    }
    declareOuterNames(body);
    declareNames(body);
    checkBody(body);

    currentScope = outerScope;
    currentReturnType = outerReturnType;
    insideFunction = outerInsideFunction;
}

void SemanticChecker::checkBlock(const NodePtr& block) {
    for (const NodePtr& statement : block->children) checkStatement(statement);
}

void SemanticChecker::checkCondition(const NodePtr& condition, const std::string& where) {
    const std::string type = checkExpr(condition);
    if (type != ERROR_TYPE && type != "bool") {
        error(condition->line, "la condición del " + where + " debe ser bool, pero es " + type);
    }
}

void SemanticChecker::checkStatement(const NodePtr& statement) {
    const std::string& kind = statement->kind;

    if (kind == "Pass") return;

    if (kind == "Return") {
        if (!insideFunction) {
            error(statement->line, "return fuera de una función");
            return;
        }
        const std::string returned = statement->children.empty() ? NONE_TYPE : checkExpr(statement->children[0]);
        if (returned != ERROR_TYPE && !isAssignable(returned, currentReturnType)) {
            error(statement->line, "la función devuelve " + currentReturnType + " pero el return da " + returned);
        }
    } else if (kind == "Assign") {
        // los hijos son los objetivos y, al final, el valor: a = b = 1
        const std::string valueType = checkExpr(statement->children.back());
        for (size_t i = 0; i + 1 < statement->children.size(); ++i) {
            const std::string targetType = checkTarget(statement->children[i]);
            if (valueType != ERROR_TYPE && targetType != ERROR_TYPE && !isAssignable(valueType, targetType)) {
                error(statement->line, "no se puede asignar un valor " + valueType + " a un destino " + targetType);
            }
        }
    } else if (kind == "If") {
        checkCondition(statement->children[0], "if");
        checkBlock(statement->children[1]);
        for (size_t i = 2; i < statement->children.size(); ++i) {
            const NodePtr& part = statement->children[i];
            if (part->kind == "Elif") {
                checkCondition(part->children[0], "elif");
                checkBlock(part->children[1]);
            } else {   // Else
                checkBlock(part->children[0]);
            }
        }
    } else if (kind == "While") {
        checkCondition(statement->children[0], "while");
        checkBlock(statement->children[1]);
    } else if (kind == "For") {
        const std::string iterableType = checkExpr(statement->children[0]);
        Symbol* variable = currentScope->lookup(statement->text);
        if (variable == nullptr) {
            error(statement->line, "la variable '" + statement->text + "' no está declarada");
        } else if (iterableType != ERROR_TYPE) {
            std::string elementType;
            if (isListType(iterableType)) elementType = elementTypeOf(iterableType);
            else if (iterableType == "str") elementType = "str";
            else error(statement->line, "no se puede recorrer un valor de tipo " + iterableType);
            if (!elementType.empty() && !isAssignable(elementType, variable->type)) {
                error(statement->line, "'" + statement->text + "' es " + variable->type +
                                       " pero los elementos son " + elementType);
            }
        }
        checkBlock(statement->children[1]);
    } else {
        checkExpr(statement);   // una expresión usada como sentencia, por ejemplo print(x)
    }
}

// Tipo del lado izquierdo de una asignación
std::string SemanticChecker::checkTarget(const NodePtr& target) {
    if (target->kind == "Id") {
        Symbol* symbol = currentScope->lookup(target->text);
        if (symbol == nullptr) {
            error(target->line, "la variable '" + target->text + "' no está declarada");
            return ERROR_TYPE;
        }
        if (symbol->kind != "var" && symbol->kind != "param") {
            error(target->line, "no se puede asignar a '" + target->text + "'");
            return ERROR_TYPE;
        }
        // Asignar a una variable de un scope externo exige global o nonlocal
        if (currentScope->lookupHere(target->text) == nullptr) {
            error(target->line, "para asignar a '" + target->text + "' hay que declararla con global o nonlocal");
            return ERROR_TYPE;
        }
        target->type = symbol->type;
        return symbol->type;
    }
    if (target->kind == "Attr" || target->kind == "Index") {
        const std::string type = checkExpr(target);
        if (target->kind == "Index" && target->children[0]->type == "str") {
            error(target->line, "las cadenas no se pueden modificar");
            return ERROR_TYPE;
        }
        return type;
    }
    error(target->line, "no se puede asignar a esta expresión");
    return ERROR_TYPE;
}

// ---------------------------------------------------------------------------------------
// Expresiones
// ---------------------------------------------------------------------------------------

std::string SemanticChecker::checkExpr(const NodePtr& node) {
    const std::string& kind = node->kind;
    std::string type = ERROR_TYPE;

    if (kind == "Int") {
        type = "int";
    } else if (kind == "Bool") {
        type = "bool";
    } else if (kind == "Str") {
        type = "str";
    } else if (kind == "None") {
        type = NONE_TYPE;
    } else if (kind == "Id") {
        Symbol* symbol = currentScope->lookup(node->text);
        if (symbol == nullptr) {
            error(node->line, "la variable '" + node->text + "' no está declarada");
        } else if (symbol->kind == "func" || symbol->kind == "class") {
            error(node->line, "'" + node->text + "' es una " + (symbol->kind == "func" ? "función" : "clase") +
                              " y no se puede usar como valor");
        } else {
            type = symbol->type;
        }
    } else if (kind == "BinOp") {
        type = checkBinOp(node);
    } else if (kind == "UnOp") {
        const std::string operand = checkExpr(node->children[0]);
        const std::string needed = (node->text == "not") ? "bool" : "int";
        if (operand == needed) {
            type = needed;
        } else if (operand != ERROR_TYPE) {
            error(node->line, "el operador '" + node->text + "' no se puede aplicar a " + operand);
        }
    } else if (kind == "IfExpr") {
        const std::string whenTrue = checkExpr(node->children[0]);
        checkCondition(node->children[1], "if");
        const std::string whenFalse = checkExpr(node->children[2]);
        if (whenTrue == ERROR_TYPE || whenFalse == ERROR_TYPE) type = ERROR_TYPE;
        else if (isAssignable(whenFalse, whenTrue)) type = whenTrue;
        else if (isAssignable(whenTrue, whenFalse)) type = whenFalse;
        else type = "object";
    } else if (kind == "ListExpr") {
        if (node->children.empty()) {
            type = EMPTY_LIST_TYPE;
        } else {
            std::string elementType = checkExpr(node->children[0]);
            for (size_t i = 1; i < node->children.size(); ++i) {
                const std::string other = checkExpr(node->children[i]);
                if (other == ERROR_TYPE) elementType = ERROR_TYPE;
                else if (other != elementType && elementType != ERROR_TYPE) elementType = "object";
            }
            if (elementType != ERROR_TYPE) type = "[" + elementType + "]";
        }
    } else if (kind == "Index") {
        const std::string objectType = checkExpr(node->children[0]);
        const std::string indexType = checkExpr(node->children[1]);
        if (objectType != ERROR_TYPE && indexType != ERROR_TYPE) {
            if (!isListType(objectType) && objectType != "str") {
                error(node->line, "no se puede indexar un valor de tipo " + objectType);
            } else if (indexType != "int") {
                error(node->line, "el índice debe ser int, pero es " + indexType);
            } else {
                type = (objectType == "str") ? "str" : elementTypeOf(objectType);
            }
        }
    } else if (kind == "Attr") {
        const std::string objectType = checkExpr(node->children[0]);
        if (objectType != ERROR_TYPE) {
            Symbol* member = findMember(objectType, node->text);
            if (member == nullptr || member->kind != "var") {
                error(node->line, "el tipo " + objectType + " no tiene el atributo '" + node->text + "'");
            } else {
                type = member->type;
            }
        }
    } else if (kind == "Call") {
        type = checkCall(node);
    } else {
        error(node->line, "expresión no reconocida: " + kind);
    }

    node->type = type;
    return type;
}

std::string SemanticChecker::checkBinOp(const NodePtr& node) {
    const std::string left = checkExpr(node->children[0]);
    const std::string right = checkExpr(node->children[1]);
    if (left == ERROR_TYPE || right == ERROR_TYPE) return ERROR_TYPE;

    const std::string& op = node->text;
    if (op == "and" || op == "or") {
        if (left == "bool" && right == "bool") return "bool";
    } else if (op == "+") {
        if (left == "int" && right == "int") return "int";
        if (left == "str" && right == "str") return "str";
        if (isListType(left) && isListType(right)) {
            if (left == right) return left;
            if (left == EMPTY_LIST_TYPE) return right;
            if (right == EMPTY_LIST_TYPE) return left;
        }
    } else if (op == "-" || op == "*" || op == "//" || op == "%") {
        if (left == "int" && right == "int") return "int";
    } else if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (left == "int" && right == "int") return "bool";
    } else if (op == "==" || op == "!=") {
        if (left == right && isBasicType(left)) return "bool";
    } else if (op == "is") {
        if (!isBasicType(left) && !isBasicType(right)) return "bool";
    }
    error(node->line, "el operador '" + op + "' no se puede aplicar a " + left + " y " + right);
    return ERROR_TYPE;
}

std::string SemanticChecker::checkCall(const NodePtr& node) {
    const NodePtr& callee = node->children[0];
    const size_t argumentCount = node->children.size() - 1;

    // Primero se revisan los argumentos, para que sus errores se reporten siempre
    std::vector<std::string> argumentTypes;
    for (size_t i = 1; i < node->children.size(); ++i) argumentTypes.push_back(checkExpr(node->children[i]));

    Symbol* function = nullptr;
    size_t skipped = 0;   // en un método el primer parámetro es self y no se pasa
    if (callee->kind == "Id") {
        Symbol* symbol = currentScope->lookup(callee->text);
        if (symbol == nullptr) {
            error(callee->line, "la función '" + callee->text + "' no está declarada");
            return ERROR_TYPE;
        }
        if (symbol->kind == "class") {   // C() crea un objeto de la clase C
            if (argumentCount != 0) {
                error(node->line, "el constructor de '" + callee->text + "' no recibe argumentos");
            }
            return symbol->name;
        }
        if (symbol->kind != "func") {
            error(callee->line, "'" + callee->text + "' no es una función");
            return ERROR_TYPE;
        }
        function = symbol;
    } else if (callee->kind == "Attr") {   // objeto.metodo(...)
        const std::string objectType = checkExpr(callee->children[0]);
        if (objectType == ERROR_TYPE) return ERROR_TYPE;
        Symbol* method = findMember(objectType, callee->text);
        if (method == nullptr || method->kind != "func") {
            error(callee->line, "el tipo " + objectType + " no tiene el método '" + callee->text + "'");
            return ERROR_TYPE;
        }
        function = method;
        skipped = 1;
    } else {
        error(node->line, "esta expresión no se puede llamar");
        return ERROR_TYPE;
    }

    const size_t expectedCount = function->paramTypes.size() - skipped;
    if (argumentCount != expectedCount) {
        error(node->line, "'" + function->name + "' espera " + std::to_string(expectedCount) +
                          " argumento(s) pero recibió " + std::to_string(argumentCount));
    } else {
        for (size_t i = 0; i < argumentCount; ++i) {
            const std::string& expected = function->paramTypes[i + skipped];
            if (argumentTypes[i] != ERROR_TYPE && !isAssignable(argumentTypes[i], expected)) {
                error(node->line, "argumento " + std::to_string(i + 1) + " de '" + function->name +
                                  "': se esperaba " + (expected == LIST_OR_STR ? "una lista o str" : expected) +
                                  " pero es " + argumentTypes[i]);
            }
        }
    }
    return function->type;
}

// ---------------------------------------------------------------------------------------
// Tipos
// ---------------------------------------------------------------------------------------

// ¿Se puede guardar un valor de tipo "from" donde se espera "to"?
bool SemanticChecker::isAssignable(const std::string& from, const std::string& to) {
    if (from == ERROR_TYPE || to == ERROR_TYPE) return true;
    if (from == to || to == "object") return true;
    if (to == LIST_OR_STR) return from == "str" || isListType(from);
    if (from == NONE_TYPE) return isListType(to) || globalScope->lookupHere(to) != nullptr;
    if (from == EMPTY_LIST_TYPE) return isListType(to);
    // una clase hija se puede usar donde se espera su clase padre
    Symbol* classSymbol = globalScope->lookupHere(from);
    while (classSymbol != nullptr && classSymbol->kind == "class") {
        if (classSymbol->parentClass == to) return true;
        classSymbol = globalScope->lookupHere(classSymbol->parentClass);
    }
    return false;
}

// Comprueba que un tipo escrito en el código exista: int, bool, str, object, una clase o [tipo]
void SemanticChecker::validateType(const std::string& type, int line) {
    std::string base = type;
    while (isListType(base)) base = elementTypeOf(base);
    if (isBasicType(base) || base == "object" || base == NONE_TYPE) return;
    Symbol* symbol = globalScope->lookupHere(base);
    if (symbol == nullptr || symbol->kind != "class") error(line, "el tipo '" + base + "' no existe");
}

// Busca un atributo o método en la clase y, si no está, en sus clases padre
Symbol* SemanticChecker::findMember(const std::string& className, const std::string& memberName) {
    std::string current = className;
    while (!current.empty()) {
        Symbol* classSymbol = globalScope->lookupHere(current);
        if (classSymbol == nullptr || classSymbol->kind != "class") return nullptr;
        Symbol* member = classSymbol->members->lookupHere(memberName);
        if (member != nullptr) return member;
        current = classSymbol->parentClass;
    }
    return nullptr;
}
