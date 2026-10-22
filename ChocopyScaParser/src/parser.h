#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <cctype>
#include <tuple>

struct Production {
    std::string left;                  // no terminal
    std::vector<std::string> right;    // secuencia 
};

// LR(1) [A -> beta • gamma, a] 
struct Item {
    int prod;       // Número de producción
    int dot;        // Posición del punto
    int lookahead;  // Índice del terminal de anticipación en 'lookaheads'

    bool operator<(const Item& o) const { return std::tie(prod, dot, lookahead) < std::tie(o.prod, o.dot, o.lookahead); }
    bool operator==(const Item& o) const { return std::tie(prod, dot, lookahead) == std::tie(o.prod, o.dot, o.lookahead); }
};

using ItemSet = std::set<Item>;

struct ParseTraceStep {
    std::string states;
    std::string symbols;
    std::string remaining_input;
    std::string action;
    std::string kind;
};

struct ParseResult {
    bool accepted = false;
    bool aborted = false;
    std::size_t syntax_errors = 0;
    std::size_t discarded_tokens = 0;
    std::vector<std::string> diagnostics;
    std::vector<ParseTraceStep> trace;
};

class Parser {
private:
    std::vector<Production> grammar;
    std::set<std::string> terminals;
    std::set<std::string> non_terminals;

    std::map<std::string, std::set<std::string>> first_sets;

    std::string user_start_symbol; // Símbolo inicial leído de la gramática
    std::string augmented_start;   // Símbolo inicial aumentado (ej: Goal)

    std::vector<std::string> symbols;      
    std::vector<std::string> lookaheads;  
    std::vector<ItemSet> collection;       
    std::map<std::pair<int, std::string>, int> transitions;  

    std::map<std::pair<int, std::string>, std::map<std::string, std::vector<Item>>> action;
    std::map<std::pair<int, std::string>, int> goto_table;

    bool isTerminal(const std::string& symbol);
    std::vector<std::string> body(int prod);   
    std::string itemText(const Item& item);
    std::map<std::string, std::set<std::string>> computeFollowSets();

public:
    Parser();

    bool loadGrammar(const std::string& filename);

    void computeFirstSets();

    std::set<std::string> computeFirstOfString(const std::vector<std::string>& beta);

    ItemSet computeClosure(ItemSet s);

    ItemSet computeGoto(const ItemSet& s, const std::string& x);

    void buildCanonicalCollection();

    int fillTables();
    ParseResult parseWithPanicRecovery(
        const std::vector<std::string>& tokens,
        const std::set<std::string>& panic_nonterminals);

    void printGrammar();
    void printFirstSets();
    void printCanonicalCollection();
    void printConflicts();
    std::string tablesText();  
};

#endif 
