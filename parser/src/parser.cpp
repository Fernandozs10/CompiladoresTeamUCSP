#include "Parser.h"

Parser::Parser() {}

bool Parser::isTerminal(const std::string& symbol) {
    if (symbol.empty()) return false;
    // Convención: Si el primer carácter no es mayúscula y no es un símbolo especial
    // (o si es una cadena específica), se considera terminal.
    if (symbol == "epsilon" || symbol == "$") return true;
    return !isupper(symbol[0]);
}

bool Parser::loadGrammar(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return false;
    }

    grammar.clear();
    terminals.clear();
    non_terminals.clear();

    std::string line;
    bool is_first_rule = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string left, arrow, symbol;
        
        ss >> left >> arrow;
        if (arrow != "->" && arrow != "::=") continue;

        if (is_first_rule) {
            user_start_symbol = left;
            augmented_start = left + "'"; // Gramática aumentada: Goal -> S
            
            // Agregar la regla aumentada Aumentada -> S
            Production aug_prod;
            aug_prod.left = augmented_start;
            aug_prod.right = { user_start_symbol };
            grammar.push_back(aug_prod);
            non_terminals.insert(augmented_start);
            
            is_first_rule = false;
        }

        non_terminals.insert(left);

        Production prod;
        prod.left = left;

        while (ss >> symbol) {
            // Soporte para alternativas representadas por '|'
            if (symbol == "|") {
                grammar.push_back(prod);
                prod.right.clear();
                continue;
            }

            prod.right.push_back(symbol);
            if (isTerminal(symbol)) {
                terminals.insert(symbol);
            } else {
                non_terminals.insert(symbol);
            }
        }
        
        if (!prod.right.empty()) {
            grammar.push_back(prod);
        }
    }

    file.close();

    // Símbolos reservados por la herramienta
    terminals.insert("epsilon");
    terminals.insert("$");

    return true;
}

// 2. firstCadena(beta): Calcula FIRST(s1 s2 ... sk)
std::set<std::string> Parser::computeFirstOfString(const std::vector<std::string>& beta) {
    std::set<std::string> result;

    if (beta.empty()) {
        result.insert("epsilon");
        return result;
    }

    bool all_nullable = true;

    for (const std::string& symbol : beta) {
        std::set<std::string> sym_first = first_sets[symbol];

        // Se agrega FIRST(si) - {epsilon}
        for (const auto& s : sym_first) {
            if (s != "epsilon") {
                result.insert(s);
            }
        }

        // Si el símbolo actual NO contiene epsilon, la cadena se detiene
        if (sym_first.count("epsilon") == 0) {
            all_nullable = false;
            break;
        }
    }

    // Contiene epsilon SOLO si toda la cadena s1...sk es anulable
    if (all_nullable) {
        result.insert("epsilon");
    }

    return result;
}

// 1. first(): Algoritmo por Punto Fijo
void Parser::computeFirstSets() {
    first_sets.clear();

    // Inicialización: FIRST(t) = {t} para todos los terminales
    for (const auto& t : terminals) {
        first_sets[t].insert(t);
    }

    // Inicialización: FIRST(A) = {} para todos los no terminales
    for (const auto& nt : non_terminals) {
        first_sets[nt] = std::set<std::string>();
    }

    bool changed = true;

    // Iteración de punto fijo
    while (changed) {
        changed = false;

        for (const auto& prod : grammar) {
            std::string A = prod.left;
            const std::vector<std::string>& beta = prod.right;

            // Manejo directo de producciones vacías A -> epsilon
            if (beta.size() == 1 && beta[0] == "epsilon") {
                if (first_sets[A].count("epsilon") == 0) {
                    first_sets[A].insert("epsilon");
                    changed = true;
                }
                continue;
            }

            // Calculamos FIRST de la cadena beta mediante firstCadena(beta)
            std::set<std::string> rhs_first = computeFirstOfString(beta);

            size_t old_size = first_sets[A].size();
            for (const auto& sym : rhs_first) {
                first_sets[A].insert(sym);
            }

            if (first_sets[A].size() > old_size) {
                changed = true;
            }
        }
    }
}

void Parser::printGrammar() {
    std::cout << "--- Gramática Cargada (Aumentada) ---" << std::endl;
    for (size_t i = 0; i < grammar.size(); ++i) {
        std::cout << "  [" << i << "] " << grammar[i].left << " -> ";
        for (const auto& sym : grammar[i].right) {
            std::cout << sym << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Parser::printFirstSets() {
    std::cout << "--- Conjuntos FIRST ---" << std::endl;
    for (const auto& nt : non_terminals) {
        std::cout << "FIRST(" << nt << ") = { ";
        for (auto it = first_sets[nt].begin(); it != first_sets[nt].end(); ++it) {
            std::cout << *it << (std::next(it) != first_sets[nt].end() ? ", " : "");
        }
        std::cout << " }" << std::endl;
    }
}