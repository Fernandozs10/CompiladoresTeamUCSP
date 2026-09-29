#include "parser.h"

#include <algorithm>

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
            // El lado izquierdo de la primera regla es el símbolo inicial.
            // La aumentación se decide al terminar de leer (ver abajo).
            user_start_symbol = left;
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

    // Gramática aumentada: si la primera regla ya tiene la forma Goal -> S
    // (una sola producción y Goal no aparece a la derecha) se usa tal cual.
    // Si no, se agrega S' -> S como producción 0.
    int start_productions = 0;
    bool start_in_rhs = false;
    for (const auto& prod : grammar) {
        if (prod.left == user_start_symbol) start_productions++;
        for (const auto& sym : prod.right) {
            if (sym == user_start_symbol) start_in_rhs = true;
        }
    }

    augmented_start = user_start_symbol;
    if (start_productions > 1 || start_in_rhs) {
        augmented_start = user_start_symbol + "'";
        grammar.insert(grammar.begin(), Production{ augmented_start, { user_start_symbol } });
        non_terminals.insert(augmented_start);
    }

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

// El marcador de fin se guarda como "$" y se muestra como "eof"
static std::string showSymbol(const std::string& symbol) {
    return symbol == "$" ? "eof" : symbol;
}

void Parser::printFirstSets() {
    std::cout << "--- Conjuntos FIRST ---" << std::endl;
    for (const auto& nt : non_terminals) {
        std::cout << "FIRST(" << nt << ") = { ";
        for (auto it = first_sets[nt].begin(); it != first_sets[nt].end(); ++it) {
            std::cout << showSymbol(*it) << (std::next(it) != first_sets[nt].end() ? ", " : "");
        }
        std::cout << " }" << std::endl;
    }

    // FIRST de los terminales: FIRST(t) = { t }
    for (const auto& t : terminals) {
        if (t == "epsilon") continue;
        std::cout << "FIRST(" << showSymbol(t) << ") = { " << showSymbol(t) << " }" << std::endl;
    }
    std::cout << std::endl;
}

// ===========================================================================
//  Generador LR(1)
// ===========================================================================

// La producción vacía se guarda como { "epsilon" }: su cuerpo real es vacío
std::vector<std::string> Parser::body(int prod) {
    if (grammar[prod].right.size() == 1 && grammar[prod].right[0] == "epsilon") return {};
    return grammar[prod].right;
}

// 3. closure(s): por cada [A -> beta • C delta, a] con C no terminal, agrega
//    [C -> • gamma, b] para cada C -> gamma y cada b en FIRST(delta a).
//    Se repite hasta que no cambie (punto fijo).
ItemSet Parser::computeClosure(ItemSet s) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Item& item : ItemSet(s)) {   // Se recorre una copia porque s crece
            std::vector<std::string> b = body(item.prod);
            if (item.dot >= (int)b.size() || isTerminal(b[item.dot])) continue;

            std::vector<std::string> delta_a(b.begin() + item.dot + 1, b.end());
            delta_a.push_back(lookaheads[item.lookahead]);
            std::set<std::string> first = computeFirstOfString(delta_a);

            for (size_t q = 0; q < grammar.size(); ++q) {
                if (grammar[q].left != b[item.dot]) continue;
                for (const std::string& t : first) {
                    if (t == "epsilon") continue;
                    int la = (int)(std::find(lookaheads.begin(), lookaheads.end(), t) - lookaheads.begin());
                    if (s.insert({ (int)q, 0, la }).second) changed = true;
                }
            }
        }
    }
    return s;
}

// 4. goto(s, x): avanza el punto sobre x y calcula la cerradura
ItemSet Parser::computeGoto(const ItemSet& s, const std::string& x) {
    ItemSet moved;
    for (const Item& item : s) {
        std::vector<std::string> b = body(item.prod);
        if (item.dot < (int)b.size() && b[item.dot] == x) {
            moved.insert({ item.prod, item.dot + 1, item.lookahead });
        }
    }
    return moved.empty() ? moved : computeClosure(moved);
}

// 5. coleccionCanonica(): cc0 = closure({[Goal -> • S, eof]}). Los cc_i se
//    procesan en orden de creación y las transiciones siguen el orden de
//    primera aparición de los símbolos en la gramática (sin el inicial).
void Parser::buildCanonicalCollection() {
    symbols.clear();
    for (const auto& prod : grammar) {
        std::vector<std::string> all = { prod.left };
        all.insert(all.end(), prod.right.begin(), prod.right.end());
        for (const auto& x : all) {
            if (x != augmented_start && x != "epsilon" &&
                std::find(symbols.begin(), symbols.end(), x) == symbols.end()) {
                symbols.push_back(x);
            }
        }
    }

    lookaheads = { "$" };
    for (const auto& x : symbols) {
        if (isTerminal(x)) lookaheads.push_back(x);
    }

    collection = { computeClosure({ { 0, 0, 0 } }) };   // Anticipación 0 = eof
    transitions.clear();

    for (size_t i = 0; i < collection.size(); ++i) {
        for (const auto& x : symbols) {
            ItemSet next = computeGoto(collection[i], x);
            if (next.empty()) continue;

            // Si el conjunto ya existe se reutiliza; si no, es un cc nuevo
            auto found = std::find(collection.begin(), collection.end(), next);
            int j = (int)(found - collection.begin());
            if (found == collection.end()) collection.push_back(next);
            transitions[{ (int)i, x }] = j;
        }
    }
}

// 6. llenarTablas():
//    [A -> beta • c gamma, a], c terminal  => Action[i, c] = shift j
//    [A -> beta •, a], A != Goal           => Action[i, a] = reduce A -> beta
//    [Goal -> S •, eof]                    => Action[i, eof] = accept
//    goto(cc_i, N) = cc_j                  => Goto[i, N] = j
int Parser::fillTables() {
    action.clear();
    goto_table.clear();

    for (size_t i = 0; i < collection.size(); ++i) {
        int state = (int)i;
        for (const Item& item : collection[i]) {
            std::vector<std::string> b = body(item.prod);
            std::string a = lookaheads[item.lookahead];

            if (item.dot < (int)b.size()) {
                std::string c = b[item.dot];
                if (isTerminal(c)) {
                    action[{ state, c }]["s" + std::to_string(transitions.at({ state, c }))].push_back(item);
                }
            } else if (grammar[item.prod].left == augmented_start) {
                action[{ state, a }]["acc"].push_back(item);
            } else {
                action[{ state, a }]["r" + std::to_string(item.prod)].push_back(item);
            }
        }

        for (const auto& x : symbols) {
            if (!isTerminal(x) && transitions.count({ state, x })) {
                goto_table[{ state, x }] = transitions.at({ state, x });
            }
        }
    }

    // Conflicto: una celda con dos acciones distintas
    int conflicts = 0;
    for (const auto& cell : action) {
        if (cell.second.size() > 1) conflicts++;
    }
    return conflicts;
}

// [A -> beta • gamma, a]
std::string Parser::itemText(const Item& item) {
    std::vector<std::string> b = body(item.prod);
    std::string text = "[" + grammar[item.prod].left + " ->";
    for (int k = 0; k <= (int)b.size(); ++k) {
        if (k == item.dot) text += " •";
        if (k < (int)b.size()) text += " " + b[k];
    }
    return text + ", " + showSymbol(lookaheads[item.lookahead]) + "]";
}

void Parser::printCanonicalCollection() {
    std::cout << "--- Colección Canónica LR(1) ---" << std::endl;
    for (size_t i = 0; i < collection.size(); ++i) {
        std::cout << "cc" << i << ":" << std::endl;
        for (const Item& item : collection[i]) {
            std::cout << "    " << itemText(item) << std::endl;
        }
        for (const auto& x : symbols) {
            if (transitions.count({ (int)i, x })) {
                std::cout << "    goto(cc" << i << ", " << x << ") = cc" << transitions.at({ (int)i, x }) << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

// Tablas Action (eof y terminales) y Goto (no terminales) separadas por
// tabuladores. Si una celda tiene conflicto se muestran sus acciones con '/'.
std::string Parser::tablesText() {
    std::vector<std::string> columns = lookaheads;
    for (const auto& x : symbols) {
        if (!isTerminal(x)) columns.push_back(x);
    }

    std::string text = "estado";
    for (const auto& c : columns) text += "\t" + showSymbol(c);
    text += "\n";

    for (size_t i = 0; i < collection.size(); ++i) {
        text += std::to_string(i);
        for (const auto& c : columns) {
            std::string cell;
            if (action.count({ (int)i, c })) {
                for (const auto& entry : action.at({ (int)i, c })) {
                    cell += (cell.empty() ? "" : "/") + entry.first;
                }
            } else if (goto_table.count({ (int)i, c })) {
                cell = std::to_string(goto_table.at({ (int)i, c }));
            }
            text += "\t" + cell;
        }
        text += "\n";
    }
    return text;
}

// Estado, terminal y los ítems que causan cada conflicto
void Parser::printConflicts() {
    for (const auto& cell : action) {
        if (cell.second.size() < 2) continue;

        bool has_shift = false;
        for (const auto& entry : cell.second) {
            if (entry.first[0] == 's') has_shift = true;
        }

        std::cout << "CONFLICTO " << (has_shift ? "shift/reduce" : "reduce/reduce")
                  << " en el estado " << cell.first.first
                  << " con '" << showSymbol(cell.first.second) << "':" << std::endl;
        for (const auto& entry : cell.second) {
            for (const Item& item : entry.second) {
                std::cout << "    " << entry.first << "  por  " << itemText(item) << std::endl;
            }
        }
    }
}
