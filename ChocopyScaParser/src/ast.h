#ifndef AST_H
#define AST_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// AST de ChocoPy. todos los nodos usan la misma estructura: "kind" dice que construccion
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
//   Return                                                  [expresion] (vacio si no devuelve nada)
//   Assign                                                  [objetivo1, objetivo2, ..., valor]
//   If                                                      [condicion, Block, Elif..., Else]
//   Elif                                                    [condicion, Block]
//   Else                                                    [Block]
//   While                                                   [condicion, Block]
//   For         variable                                    [iterable, Block]
//   Block                                                   sentencias
//   Int / Str / Bool / None   valor   (type = tipo del valor)
//   Id          nombre                tipo de la variable
//   BinOp       operador (+, ==, ...) tipo del resultado   [izquierda, derecha]
//   UnOp        operador (-, not)     tipo del resultado   [operando]
//   IfExpr                            tipo del resultado   [valor si cierto, condicion, valor si falso]
//   ListExpr                          tipo de la lista     elementos
//   Index                             tipo del elemento    [lista o cadena, indice]
//   Attr        nombre del atributo   tipo del atributo    [objeto]
//   Call                              tipo del resultado   [funcion, argumento1, argumento2, ...]
//
// "line" es la linea del codigo fuente. "type" lo llena el chequeo semantico en las expresiones.
// en el Id o Attr que se llama en un Call, "type" es la firma de la funcion: "(int) -> str".

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

// nodo para un token que el parser acaba de desplazar (kind = tipo del token, text = su lexema)
NodePtr newTokenNode(const std::string& tokenType, const std::string& lexeme, int line);

// se llama cada vez que el parser reduce una produccion "lhsSymbol -> rhsSymbols". "rhsNodes" son los
// nodos de los simbolos del lado derecho, en el mismo orden. devuelve el nodo del lado izquierdo.
NodePtr buildNodeFromProduction(const std::string& lhsSymbol, const std::vector<std::string>& rhsSymbols,
                                const std::vector<NodePtr>& rhsNodes);

// imprime el arbol con una sangria de dos espacios por nivel
void printAst(const NodePtr& node, std::ostream& out, int depth = 0);

#endif
