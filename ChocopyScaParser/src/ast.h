#ifndef AST_H
#define AST_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// AST de ChocoPy. Todos los nodos usan la misma estructura: "kind" dice qué construcción
// es, y "children" guarda sus partes en un orden fijo.
//
//   kind        text                  type                 children
//   ----------  --------------------  -------------------  ------------------------------------
//   Program                                                 declaraciones y luego sentencias
//   VarDef      nombre                tipo declarado       [literal inicial]
//   FuncDef     nombre                tipo de retorno      [Params, Body]
//   Params                                                  Param, Param, ...
//   Param       nombre                tipo declarado
//   Body                                                    Global/Nonlocal/VarDef/FuncDef y sentencias
//   ClassDef    nombre                clase padre          VarDef y FuncDef
//   Global      nombre
//   Nonlocal    nombre
//   Pass
//   Return                                                  [expresión] (vacío si no devuelve nada)
//   Assign                                                  [objetivo1, objetivo2, ..., valor]
//   If                                                      [condición, Block, Elif..., Else]
//   Elif                                                    [condición, Block]
//   Else                                                    [Block]
//   While                                                   [condición, Block]
//   For         variable                                    [iterable, Block]
//   Block                                                   sentencias
//   Int / Str / Bool / None   valor   (type = tipo del valor)
//   Id          nombre                tipo de la variable
//   BinOp       operador (+, ==, ...) tipo del resultado   [izquierda, derecha]
//   UnOp        operador (-, not)     tipo del resultado   [operando]
//   IfExpr                            tipo del resultado   [valor si cierto, condición, valor si falso]
//   ListExpr                          tipo de la lista     elementos
//   Index                             tipo del elemento    [lista o cadena, índice]
//   Attr        nombre del atributo   tipo del atributo    [objeto]
//   Call                              tipo del resultado   [función, argumento1, argumento2, ...]
//
// "line" es la línea del código fuente. "type" lo llena el chequeo semántico en las expresiones.

struct Node;
using NodePtr = std::shared_ptr<Node>;

struct Node {
    std::string kind;
    std::string text;
    std::string type;
    int line = 0;
    std::vector<NodePtr> children;
};

NodePtr newNode(const std::string& kind, const std::string& text = "", int line = 0);

// Nodo para un token que el parser acaba de desplazar (kind = tipo del token, text = su lexema)
NodePtr newTokenNode(const std::string& tokenType, const std::string& lexeme, int line);

// Se llama cada vez que el parser reduce una producción "lhsSymbol -> rhsSymbols". "rhsNodes" son los
// nodos de los símbolos del lado derecho, en el mismo orden. Devuelve el nodo del lado izquierdo.
NodePtr buildNodeFromProduction(const std::string& lhsSymbol, const std::vector<std::string>& rhsSymbols,
                                const std::vector<NodePtr>& rhsNodes);

// Imprime el árbol con una sangría de dos espacios por nivel
void printAst(const NodePtr& node, std::ostream& out, int depth = 0);

#endif
