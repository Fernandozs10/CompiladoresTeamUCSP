#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "symtab.h"

// Recorre el AST, construye las tablas de símbolos y comprueba nombres y tipos.
// No se detiene en el primer error: los junta todos.
class SemanticChecker {
public:
    SemanticChecker();

    // Devuelve los errores encontrados, cada uno con su línea (vacío si el programa es correcto)
    std::vector<std::string> check(const NodePtr& program);

private:
    // Tablas de símbolos
    Scope* openScope(const std::string& name, Scope* parent);
    void declareClasses(const NodePtr& program);
    void declareNames(const NodePtr& container);
    void declareOuterNames(const NodePtr& body);

    // Recorrido
    void checkBody(const NodePtr& container);
    void checkFunction(const NodePtr& function);
    void checkStatement(const NodePtr& statement);
    void checkBlock(const NodePtr& block);
    void checkCondition(const NodePtr& condition, const std::string& where);
    std::string checkTarget(const NodePtr& target);

    // Expresiones: devuelven su tipo y lo guardan en node->type
    std::string checkExpr(const NodePtr& node);
    std::string checkBinOp(const NodePtr& node);
    std::string checkCall(const NodePtr& node);

    // Tipos
    bool isAssignable(const std::string& from, const std::string& to);
    void validateType(const std::string& type, int line);
    Symbol* findMember(const std::string& className, const std::string& memberName);

    void error(int line, const std::string& message);

    std::vector<std::unique_ptr<Scope>> allScopes;   // dueña de todas las tablas
    Scope* globalScope = nullptr;
    Scope* currentScope = nullptr;
    std::string currentReturnType;                    // tipo de retorno de la función que se revisa
    bool insideFunction = false;
    std::vector<std::string> errors;
};

#endif
