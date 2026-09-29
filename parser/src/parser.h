#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <cctype>
#include <tuple>

// Estructura para guardar una regla de producción (ej: List -> List Pair)
struct Production {
    std::string left;                  // Símbolo del lado izquierdo (No Terminal)
    std::vector<std::string> right;    // Secuencia de símbolos del lado derecho
};

// Ítem LR(1) [A -> beta • gamma, a] representado como tres enteros
struct Item {
    int prod;       // Número de producción
    int dot;        // Posición del punto
    int lookahead;  // Índice del terminal de anticipación en 'lookaheads'

    bool operator<(const Item& o) const { return std::tie(prod, dot, lookahead) < std::tie(o.prod, o.dot, o.lookahead); }
    bool operator==(const Item& o) const { return std::tie(prod, dot, lookahead) == std::tie(o.prod, o.dot, o.lookahead); }
};

using ItemSet = std::set<Item>;

class Parser {
private:
    std::vector<Production> grammar;
    std::set<std::string> terminals;
    std::set<std::string> non_terminals;

    // Almacena FIRST(X) para cada símbolo individual
    std::map<std::string, std::set<std::string>> first_sets;

    std::string user_start_symbol; // Símbolo inicial leído de la gramática
    std::string augmented_start;   // Símbolo inicial aumentado (ej: Goal)

    std::vector<std::string> symbols;      // Orden de primera aparición, sin el inicial
    std::vector<std::string> lookaheads;   // [0] = "$" (eof), luego los terminales
    std::vector<ItemSet> collection;       // CC = {cc0, cc1, ...}
    std::map<std::pair<int, std::string>, int> transitions;   // goto(cc_i, X) = cc_j

    // Action[i, a]: cada acción ("s3", "r2", "acc") con los ítems que la causan.
    // Más de una acción en la misma celda es un conflicto.
    std::map<std::pair<int, std::string>, std::map<std::string, std::vector<Item>>> action;
    std::map<std::pair<int, std::string>, int> goto_table;

    bool isTerminal(const std::string& symbol);
    std::vector<std::string> body(int prod);   // Lado derecho sin "epsilon"
    std::string itemText(const Item& item);

public:
    Parser();

    // Carga la gramática y crea automáticamente la producción aumentada
    bool loadGrammar(const std::string& filename);

    // 1. first(): Algoritmo por Punto Fijo para todos los símbolos
    void computeFirstSets();

    // 2. firstCadena(beta): Cálculo de FIRST para una cadena beta = s1 s2 ... sk
    std::set<std::string> computeFirstOfString(const std::vector<std::string>& beta);

    // 3. closure(s): Cerradura de un conjunto de ítems LR(1)
    ItemSet computeClosure(ItemSet s);

    // 4. goto(s, x): "goto" es palabra reservada de C++
    ItemSet computeGoto(const ItemSet& s, const std::string& x);

    // 5. coleccionCanonica(): CC y registro de transiciones
    void buildCanonicalCollection();

    // 6. llenarTablas(): Action y Goto. Devuelve el número de conflictos.
    int fillTables();

    void printGrammar();
    void printFirstSets();
    void printCanonicalCollection();
    void printConflicts();
    std::string tablesText();   // Tablas separadas por tabuladores
};

#endif // PARSER_H
