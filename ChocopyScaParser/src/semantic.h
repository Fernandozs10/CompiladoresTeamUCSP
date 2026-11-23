#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "symtab.h"

// recorre el AST, construye las tablas de simbolos y comprueba nombres y tipos.
// no se detiene en el primer error: los junta todos.
class SemanticChecker {
public:
    SemanticChecker();

    // devuelve los errores encontrados, cada uno con su linea (vacio si el programa es correcto)
    std::vector<std::string> checkProgram(const NodePtr& program);

private:
    // tablas de simbolos
    Scope* createScope(const std::string& scopeName, Scope* parentScope);
    void declareClassNames(const NodePtr& program);
    void declareNamesInScope(const NodePtr& container);
    void declareGlobalAndNonlocalNames(const NodePtr& body);

    // recorrido
    void checkBodyContents(const NodePtr& container);
    void checkFunctionDefinition(const NodePtr& function);
    void checkStatement(const NodePtr& statement);
    void checkBlockStatements(const NodePtr& block);
    void checkBooleanCondition(const NodePtr& conditionNode, const std::string& statementName);
    std::string checkAssignmentTarget(const NodePtr& target);

    // expresiones: devuelven su tipo y lo guardan en node->type
    std::string checkExpression(const NodePtr& node);
    std::string checkBinaryOperation(const NodePtr& node);
    std::string checkFunctionCall(const NodePtr& node);

    // tipos
    bool isAssignable(const std::string& sourceType, const std::string& targetType);
    void checkTypeExists(const std::string& typeName, int line);
    Symbol* findClassMember(const std::string& className, const std::string& memberName);

    void reportError(int line, const std::string& message);

    std::vector<std::unique_ptr<Scope>> ownedScopes;   // duena de todas las tablas
    Scope* globalScope = nullptr;
    Scope* currentScope = nullptr;
    std::string expectedReturnType;                    // tipo de retorno de la funcion que se revisa
    bool isInsideFunction = false;
    std::vector<std::string> collectedErrors;
};

#endif
