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

// Estructura para guardar una regla de producción (ej: List -> List Pair)
struct Production {
    std::string left;                  // Símbolo del lado izquierdo (No Terminal)
    std::vector<std::string> right;    // Secuencia de símbolos del lado derecho
};

class Parser {
private:
    std::vector<Production> grammar;
    std::set<std::string> terminals;
    std::set<std::string> non_terminals;
    
    // Almacena FIRST(X) para cada símbolo individual
    std::map<std::string, std::set<std::string>> first_sets;
    
    std::string user_start_symbol; // Símbolo inicial leído de la gramática
    std::string augmented_start;   // Símbolo inicial aumentado (ej: Goal)

    bool isTerminal(const std::string& symbol);

public:
    Parser();
    
    // Carga la gramática y crea automáticamente la producción aumentada
    bool loadGrammar(const std::string& filename);
    
    // 1. first(): Algoritmo por Punto Fijo para todos los símbolos
    void computeFirstSets();
    
    // 2. firstCadena(beta): Cálculo de FIRST para una cadena beta = s1 s2 ... sk
    std::set<std::string> computeFirstOfString(const std::vector<std::string>& beta);

    void printGrammar();
    void printFirstSets();
};

#endif // PARSER_H