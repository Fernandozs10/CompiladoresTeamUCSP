#include "semantic.h"

// tipo de una expresion que ya tuvo un error: asi no se repite el mismo error en cascada
static const std::string ERROR_TYPE = "<error>";
static const std::string NONE_TYPE = "<None>";
static const std::string EMPTY_LIST_TYPE = "[<Empty>]";
static const std::string LIST_OR_STRING_PARAM = "<lista o str>";   // solo para el parametro de len

static bool isListType(const std::string& typeName) {
    return typeName.size() >= 2 && typeName.front() == '[';
}

static std::string listElementType(const std::string& listType) {
    return listType.substr(1, listType.size() - 2);
}

static bool isPrimitiveType(const std::string& typeName) {
    return typeName == "int" || typeName == "bool" || typeName == "str";
}

static Symbol createSymbol(const std::string& name, const std::string& kind, const std::string& typeName) {
    Symbol symbol;
    symbol.name = name;
    symbol.kind = kind;
    symbol.type = typeName;
    return symbol;
}

// tipo del nombre de una funcion: "(int, str) -> bool". en un metodo no se cuenta self
static std::string functionSignature(const Symbol& function, size_t selfParameterCount) {
    std::string signature = "(";
    for (size_t i = selfParameterCount; i < function.parameterTypes.size(); ++i) {
        if (i > selfParameterCount) signature += ", ";
        signature += function.parameterTypes[i];
    }
    return signature + ") -> " + function.type;
}

SemanticChecker::SemanticChecker() {
    globalScope = createScope("global", nullptr);
    currentScope = globalScope;

    // funciones predefinidas
    Symbol printFunction = createSymbol("print", "func", NONE_TYPE);
    printFunction.parameterTypes = {"object"};
    Symbol lenFunction = createSymbol("len", "func", "int");
    lenFunction.parameterTypes = {LIST_OR_STRING_PARAM};
    Symbol inputFunction = createSymbol("input", "func", "str");
    globalScope->insertSymbol(printFunction);
    globalScope->insertSymbol(lenFunction);
    globalScope->insertSymbol(inputFunction);
}

void SemanticChecker::reportError(int line, const std::string& message) {
    collectedErrors.push_back("error semantico (linea " + std::to_string(line) + "): " + message);
}

Scope* SemanticChecker::createScope(const std::string& scopeName, Scope* parentScope) {
    ownedScopes.push_back(std::make_unique<Scope>(scopeName, parentScope));
    return ownedScopes.back().get();
}

std::vector<std::string> SemanticChecker::checkProgram(const NodePtr& program) {
    declareClassNames(program);
    declareNamesInScope(program);
    checkBodyContents(program);
    return collectedErrors;
}

// ---------------------------------------------------------------------------------------
// tablas de simbolos
// ---------------------------------------------------------------------------------------

// primero se registran todas las clases, para poder usarlas como tipo antes de su definicion
void SemanticChecker::declareClassNames(const NodePtr& program) {
    for (const NodePtr& node : program->children) {
        if (node->kind != "ClassDef") continue;
        Symbol classSymbol = createSymbol(node->text, "class", node->text);
        classSymbol.parentClassName = node->type;
        classSymbol.memberScope = createScope(node->text, nullptr);
        if (!globalScope->insertSymbol(classSymbol)) {
            reportError(node->line, "'" + node->text + "' ya esta declarado en este ambito");
        }
    }
}

// inserta en la tabla actual los nombres que declara el contenedor (Program o Body)
void SemanticChecker::declareNamesInScope(const NodePtr& container) {
    for (const NodePtr& node : container->children) {
        if (node->kind == "VarDef") {
            checkTypeExists(node->type, node->line);
            if (!currentScope->insertSymbol(createSymbol(node->text, "var", node->type))) {
                reportError(node->line, "'" + node->text + "' ya esta declarado en este ambito");
            }
            // el valor inicial es un literal: se comprueba que coincida con el tipo declarado
            const NodePtr& literal = node->children[0];
            literal->type = (literal->kind == "Int") ? "int" : (literal->kind == "Bool") ? "bool"
                          : (literal->kind == "Str") ? "str" : NONE_TYPE;
            if (!isAssignable(literal->type, node->type)) {
                reportError(node->line, "'" + node->text + "' se declaro como " + node->type +
                                  " pero su valor inicial es " + literal->type);
            }
        } else if (node->kind == "FuncDef") {
            Symbol function = createSymbol(node->text, "func", node->type);
            checkTypeExists(node->type, node->line);
            for (const NodePtr& param : node->children[0]->children) {
                checkTypeExists(param->type, param->line);
                function.parameterTypes.push_back(param->type);
            }
            if (!currentScope->insertSymbol(function)) {
                reportError(node->line, "'" + node->text + "' ya esta declarado en este ambito");
            }
        } else if (node->kind == "ClassDef") {
            Symbol* classSymbol = globalScope->lookupInThisScope(node->text);
            if (classSymbol == nullptr || classSymbol->memberScope == nullptr) continue;
            if (!node->type.empty() && node->type != "object" &&
                (globalScope->lookupInThisScope(node->type) == nullptr ||
                 globalScope->lookupInThisScope(node->type)->kind != "class")) {
                reportError(node->line, "la clase padre '" + node->type + "' no existe");
            }
            // los atributos y metodos van a la tabla de la clase
            Scope* outerScope = currentScope;
            currentScope = classSymbol->memberScope;
            declareNamesInScope(node);
            currentScope = outerScope;
        }
    }
}

// global x / nonlocal x: la tabla de la funcion recibe una entrada que apunta a la variable externa
void SemanticChecker::declareGlobalAndNonlocalNames(const NodePtr& body) {
    for (const NodePtr& node : body->children) {
        if (node->kind != "Global" && node->kind != "Nonlocal") continue;

        Symbol* target = nullptr;
        if (node->kind == "Global") {
            target = globalScope->lookupInThisScope(node->text);
        } else {   // nonlocal: una variable de una funcion que contiene a esta (no la global)
            for (Scope* scope = currentScope->parentScope; scope != nullptr && scope != globalScope; scope = scope->parentScope) {
                target = scope->lookupInThisScope(node->text);
                if (target != nullptr) break;
            }
        }
        if (target != nullptr && target->kind == "ref") target = target->referencedSymbol;
        if (target == nullptr || (target->kind != "var" && target->kind != "param")) {
            std::string where = node->kind == "Global" ? "global" : "de una funcion externa";
            reportError(node->line, "'" + node->text + "' no es una variable " + where);
            continue;
        }

        Symbol outerReference = createSymbol(node->text, "ref", target->type);
        outerReference.referencedSymbol = target;
        if (!currentScope->insertSymbol(outerReference)) {
            reportError(node->line, "'" + node->text + "' ya esta declarado en este ambito");
        }
    }
}

// ---------------------------------------------------------------------------------------
// recorrido: definiciones y sentencias
// ---------------------------------------------------------------------------------------

// revisa los cuerpos de las funciones y las sentencias (los nombres ya fueron declarados)
void SemanticChecker::checkBodyContents(const NodePtr& container) {
    for (const NodePtr& node : container->children) {
        if (node->kind == "FuncDef") {
            checkFunctionDefinition(node);
        } else if (node->kind == "ClassDef") {
            for (const NodePtr& member : node->children) {
                if (member->kind == "FuncDef") checkFunctionDefinition(member);
            }
        } else if (node->kind != "VarDef" && node->kind != "Global" && node->kind != "Nonlocal") {
            checkStatement(node);
        }
    }
}

void SemanticChecker::checkFunctionDefinition(const NodePtr& function) {
    const NodePtr& params = function->children[0];
    const NodePtr& body = function->children[1];

    // se guarda lo del scope de afuera para restaurarlo al terminar
    Scope* outerScope = currentScope;
    const std::string outerExpectedReturnType = expectedReturnType;
    const bool outerIsInsideFunction = isInsideFunction;

    currentScope = createScope(function->text, outerScope);   // tabla nueva: su parent es la actual
    expectedReturnType = function->type;
    isInsideFunction = true;

    for (const NodePtr& param : params->children) {
        if (!currentScope->insertSymbol(createSymbol(param->text, "param", param->type))) {
            reportError(param->line, "el parametro '" + param->text + "' esta repetido");
        }
    }
    declareGlobalAndNonlocalNames(body);
    declareNamesInScope(body);
    checkBodyContents(body);

    currentScope = outerScope;
    expectedReturnType = outerExpectedReturnType;
    isInsideFunction = outerIsInsideFunction;
}

void SemanticChecker::checkBlockStatements(const NodePtr& block) {
    for (const NodePtr& statement : block->children) checkStatement(statement);
}

void SemanticChecker::checkBooleanCondition(const NodePtr& conditionNode, const std::string& statementName) {
    const std::string type = checkExpression(conditionNode);
    if (type != ERROR_TYPE && type != "bool") {
        reportError(conditionNode->line, "la condicion del " + statementName + " debe ser bool, pero es " + type);
    }
}

void SemanticChecker::checkStatement(const NodePtr& statement) {
    const std::string& kind = statement->kind;

    if (kind == "Pass") return;

    if (kind == "Return") {
        if (!isInsideFunction) {
            reportError(statement->line, "return fuera de una funcion");
            return;
        }
        const std::string returnedType = statement->children.empty() ? NONE_TYPE : checkExpression(statement->children[0]);
        if (returnedType != ERROR_TYPE && !isAssignable(returnedType, expectedReturnType)) {
            reportError(statement->line, "la funcion devuelve " + expectedReturnType + " pero el return da " + returnedType);
        }
    } else if (kind == "Assign") {
        // los hijos son los objetivos y, al final, el valor: a = b = 1
        const std::string valueType = checkExpression(statement->children.back());
        for (size_t i = 0; i + 1 < statement->children.size(); ++i) {
            const std::string targetType = checkAssignmentTarget(statement->children[i]);
            if (valueType != ERROR_TYPE && targetType != ERROR_TYPE && !isAssignable(valueType, targetType)) {
                reportError(statement->line, "no se puede asignar un valor " + valueType + " a un destino " + targetType);
            }
        }
    } else if (kind == "If") {
        checkBooleanCondition(statement->children[0], "if");
        checkBlockStatements(statement->children[1]);
        for (size_t i = 2; i < statement->children.size(); ++i) {
            const NodePtr& part = statement->children[i];
            if (part->kind == "Elif") {
                checkBooleanCondition(part->children[0], "elif");
                checkBlockStatements(part->children[1]);
            } else {   // Else
                checkBlockStatements(part->children[0]);
            }
        }
    } else if (kind == "While") {
        checkBooleanCondition(statement->children[0], "while");
        checkBlockStatements(statement->children[1]);
    } else if (kind == "For") {
        const std::string iterableType = checkExpression(statement->children[0]);
        Symbol* variable = currentScope->lookupThroughParents(statement->text);
        if (variable == nullptr) {
            reportError(statement->line, "la variable '" + statement->text + "' no esta declarada");
        } else if (iterableType != ERROR_TYPE) {
            std::string elementType;
            if (isListType(iterableType)) elementType = listElementType(iterableType);
            else if (iterableType == "str") elementType = "str";
            else reportError(statement->line, "no se puede recorrer un valor de tipo " + iterableType);
            if (!elementType.empty() && !isAssignable(elementType, variable->type)) {
                reportError(statement->line, "'" + statement->text + "' es " + variable->type +
                                       " pero los elementos son " + elementType);
            }
        }
        checkBlockStatements(statement->children[1]);
    } else {
        checkExpression(statement);   // una expresion usada como sentencia, por ejemplo print(x)
    }
}

// tipo del lado izquierdo de una asignacion
std::string SemanticChecker::checkAssignmentTarget(const NodePtr& target) {
    if (target->kind == "Id") {
        Symbol* symbol = currentScope->lookupThroughParents(target->text);
        if (symbol == nullptr) {
            reportError(target->line, "la variable '" + target->text + "' no esta declarada");
            return ERROR_TYPE;
        }
        if (symbol->kind != "var" && symbol->kind != "param") {
            reportError(target->line, "no se puede asignar a '" + target->text + "'");
            return ERROR_TYPE;
        }
        // asignar a una variable de un scope externo exige global o nonlocal
        if (currentScope->lookupInThisScope(target->text) == nullptr) {
            reportError(target->line, "para asignar a '" + target->text + "' hay que declararla con global o nonlocal");
            return ERROR_TYPE;
        }
        target->type = symbol->type;
        return symbol->type;
    }
    if (target->kind == "Attr" || target->kind == "Index") {
        const std::string type = checkExpression(target);
        if (target->kind == "Index" && target->children[0]->type == "str") {
            reportError(target->line, "las cadenas no se pueden modificar");
            return ERROR_TYPE;
        }
        return type;
    }
    reportError(target->line, "no se puede asignar a esta expresion");
    return ERROR_TYPE;
}

// ---------------------------------------------------------------------------------------
// expresiones
// ---------------------------------------------------------------------------------------

std::string SemanticChecker::checkExpression(const NodePtr& node) {
    const std::string& kind = node->kind;
    std::string resultType = ERROR_TYPE;

    if (kind == "Int") {
        resultType = "int";
    } else if (kind == "Bool") {
        resultType = "bool";
    } else if (kind == "Str") {
        resultType = "str";
    } else if (kind == "None") {
        resultType = NONE_TYPE;
    } else if (kind == "Id") {
        Symbol* symbol = currentScope->lookupThroughParents(node->text);
        if (symbol == nullptr) {
            reportError(node->line, "la variable '" + node->text + "' no esta declarada");
        } else if (symbol->kind == "func" || symbol->kind == "class") {
            reportError(node->line, "'" + node->text + "' es una " + (symbol->kind == "func" ? "funcion" : "clase") +
                              " y no se puede usar como valor");
        } else {
            resultType = symbol->type;
        }
    } else if (kind == "BinOp") {
        resultType = checkBinaryOperation(node);
    } else if (kind == "UnOp") {
        const std::string operand = checkExpression(node->children[0]);
        const std::string needed = (node->text == "not") ? "bool" : "int";
        if (operand == needed) {
            resultType = needed;
        } else if (operand != ERROR_TYPE) {
            reportError(node->line, "el operador '" + node->text + "' no se puede aplicar a " + operand);
        }
    } else if (kind == "IfExpr") {
        const std::string whenTrue = checkExpression(node->children[0]);
        checkBooleanCondition(node->children[1], "if");
        const std::string whenFalse = checkExpression(node->children[2]);
        if (whenTrue == ERROR_TYPE || whenFalse == ERROR_TYPE) resultType = ERROR_TYPE;
        else if (isAssignable(whenFalse, whenTrue)) resultType = whenTrue;
        else if (isAssignable(whenTrue, whenFalse)) resultType = whenFalse;
        else resultType = "object";
    } else if (kind == "ListExpr") {
        if (node->children.empty()) {
            resultType = EMPTY_LIST_TYPE;
        } else {
            std::string elementType = checkExpression(node->children[0]);
            for (size_t i = 1; i < node->children.size(); ++i) {
                const std::string other = checkExpression(node->children[i]);
                if (other == ERROR_TYPE) elementType = ERROR_TYPE;
                else if (other != elementType && elementType != ERROR_TYPE) elementType = "object";
            }
            if (elementType != ERROR_TYPE) resultType = "[" + elementType + "]";
        }
    } else if (kind == "Index") {
        const std::string objectType = checkExpression(node->children[0]);
        const std::string indexType = checkExpression(node->children[1]);
        if (objectType != ERROR_TYPE && indexType != ERROR_TYPE) {
            if (!isListType(objectType) && objectType != "str") {
                reportError(node->line, "no se puede indexar un valor de tipo " + objectType);
            } else if (indexType != "int") {
                reportError(node->line, "el indice debe ser int, pero es " + indexType);
            } else {
                resultType = (objectType == "str") ? "str" : listElementType(objectType);
            }
        }
    } else if (kind == "Attr") {
        const std::string objectType = checkExpression(node->children[0]);
        if (objectType != ERROR_TYPE) {
            Symbol* member = findClassMember(objectType, node->text);
            if (member == nullptr || member->kind != "var") {
                reportError(node->line, "el tipo " + objectType + " no tiene el atributo '" + node->text + "'");
            } else {
                resultType = member->type;
            }
        }
    } else if (kind == "Call") {
        resultType = checkFunctionCall(node);
    } else {
        reportError(node->line, "expresion no reconocida: " + kind);
    }

    node->type = resultType;
    return resultType;
}

std::string SemanticChecker::checkBinaryOperation(const NodePtr& node) {
    const std::string leftType = checkExpression(node->children[0]);
    const std::string rightType = checkExpression(node->children[1]);
    if (leftType == ERROR_TYPE || rightType == ERROR_TYPE) return ERROR_TYPE;

    const std::string& operatorText = node->text;
    if (operatorText == "and" || operatorText == "or") {
        if (leftType == "bool" && rightType == "bool") return "bool";
    } else if (operatorText == "+") {
        if (leftType == "int" && rightType == "int") return "int";
        if (leftType == "str" && rightType == "str") return "str";
        if (isListType(leftType) && isListType(rightType)) {
            if (leftType == rightType) return leftType;
            if (leftType == EMPTY_LIST_TYPE) return rightType;
            if (rightType == EMPTY_LIST_TYPE) return leftType;
        }
    } else if (operatorText == "-" || operatorText == "*" || operatorText == "//" || operatorText == "%") {
        if (leftType == "int" && rightType == "int") return "int";
    } else if (operatorText == "<" || operatorText == ">" || operatorText == "<=" || operatorText == ">=") {
        if (leftType == "int" && rightType == "int") return "bool";
    } else if (operatorText == "==" || operatorText == "!=") {
        if (leftType == rightType && isPrimitiveType(leftType)) return "bool";
    } else if (operatorText == "is") {
        if (!isPrimitiveType(leftType) && !isPrimitiveType(rightType)) return "bool";
    }
    reportError(node->line, "el operador '" + operatorText + "' no se puede aplicar a " + leftType + " y " + rightType);
    return ERROR_TYPE;
}

std::string SemanticChecker::checkFunctionCall(const NodePtr& node) {
    const NodePtr& callee = node->children[0];
    const size_t argumentCount = node->children.size() - 1;

    // primero se revisan los argumentos, para que sus errores se reporten siempre
    std::vector<std::string> argumentTypes;
    for (size_t i = 1; i < node->children.size(); ++i) argumentTypes.push_back(checkExpression(node->children[i]));

    Symbol* function = nullptr;
    size_t selfParameterCount = 0;   // en un metodo el primer parametro es self y no se pasa
    callee->type = ERROR_TYPE;       // se reemplaza por la firma cuando se encuentra la funcion
    if (callee->kind == "Id") {
        Symbol* symbol = currentScope->lookupThroughParents(callee->text);
        if (symbol == nullptr) {
            reportError(callee->line, "la funcion '" + callee->text + "' no esta declarada");
            return ERROR_TYPE;
        }
        if (symbol->kind == "class") {   // C() crea un objeto de la clase C
            if (argumentCount != 0) {
                reportError(node->line, "el constructor de '" + callee->text + "' no recibe argumentos");
            }
            callee->type = "() -> " + symbol->name;
            return symbol->name;
        }
        if (symbol->kind != "func") {
            reportError(callee->line, "'" + callee->text + "' no es una funcion");
            return ERROR_TYPE;
        }
        function = symbol;
    } else if (callee->kind == "Attr") {   // objeto.metodo(...)
        const std::string objectType = checkExpression(callee->children[0]);
        if (objectType == ERROR_TYPE) return ERROR_TYPE;
        Symbol* method = findClassMember(objectType, callee->text);
        if (method == nullptr || method->kind != "func") {
            reportError(callee->line, "el tipo " + objectType + " no tiene el metodo '" + callee->text + "'");
            return ERROR_TYPE;
        }
        function = method;
        selfParameterCount = 1;
    } else {
        reportError(node->line, "esta expresion no se puede llamar");
        return ERROR_TYPE;
    }

    callee->type = functionSignature(*function, selfParameterCount);

    const size_t expectedCount = function->parameterTypes.size() - selfParameterCount;
    if (argumentCount != expectedCount) {
        reportError(node->line, "'" + function->name + "' espera " + std::to_string(expectedCount) +
                          " argumento(s) pero recibio " + std::to_string(argumentCount));
    } else {
        for (size_t i = 0; i < argumentCount; ++i) {
            const std::string& expectedType = function->parameterTypes[i + selfParameterCount];
            if (argumentTypes[i] != ERROR_TYPE && !isAssignable(argumentTypes[i], expectedType)) {
                reportError(node->line, "argumento " + std::to_string(i + 1) + " de '" + function->name +
                                  "': se esperaba " + (expectedType == LIST_OR_STRING_PARAM ? "una lista o str" : expectedType) +
                                  " pero es " + argumentTypes[i]);
            }
        }
    }
    return function->type;
}

// ---------------------------------------------------------------------------------------
// tipos
// ---------------------------------------------------------------------------------------

// se puede guardar un valor de tipo sourceType donde se espera targetType?
bool SemanticChecker::isAssignable(const std::string& sourceType, const std::string& targetType) {
    if (sourceType == ERROR_TYPE || targetType == ERROR_TYPE) return true;
    if (sourceType == targetType || targetType == "object") return true;
    if (targetType == LIST_OR_STRING_PARAM) return sourceType == "str" || isListType(sourceType);
    if (sourceType == NONE_TYPE) return isListType(targetType) || globalScope->lookupInThisScope(targetType) != nullptr;
    if (sourceType == EMPTY_LIST_TYPE) return isListType(targetType);
    // una clase hija se puede usar donde se espera su clase padre
    Symbol* classSymbol = globalScope->lookupInThisScope(sourceType);
    while (classSymbol != nullptr && classSymbol->kind == "class") {
        if (classSymbol->parentClassName == targetType) return true;
        classSymbol = globalScope->lookupInThisScope(classSymbol->parentClassName);
    }
    return false;
}

// comprueba que un tipo escrito en el codigo exista: int, bool, str, object, una clase o [tipo]
void SemanticChecker::checkTypeExists(const std::string& typeName, int line) {
    std::string base = typeName;
    while (isListType(base)) base = listElementType(base);
    if (isPrimitiveType(base) || base == "object" || base == NONE_TYPE) return;
    Symbol* symbol = globalScope->lookupInThisScope(base);
    if (symbol == nullptr || symbol->kind != "class") reportError(line, "el tipo '" + base + "' no existe");
}

// busca un atributo o metodo en la clase y, si no esta, en sus clases padre
Symbol* SemanticChecker::findClassMember(const std::string& className, const std::string& memberName) {
    std::string current = className;
    while (!current.empty()) {
        Symbol* classSymbol = globalScope->lookupInThisScope(current);
        if (classSymbol == nullptr || classSymbol->kind != "class") return nullptr;
        Symbol* member = classSymbol->memberScope->lookupInThisScope(memberName);
        if (member != nullptr) return member;
        current = classSymbol->parentClassName;
    }
    return nullptr;
}
