#include "parser.h"

#include <algorithm>
#include <cstdint>

Parser::Parser() {}

bool Parser::isTerminal(const std::string& symbol) {
    if (symbol.empty()) return false;
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
    // FNV-1a de la gramática. Se mezcla con la fecha de compilación de este archivo:
    // si el código del generador cambia y se recompila, las cachés viejas dejan de valer.
    std::uint64_t hash = 1469598103934665603ULL;
    for (unsigned char c : std::string(__DATE__ " " __TIME__)) { hash ^= c; hash *= 1099511628211ULL; }

    while (std::getline(file, line)) {
        for (unsigned char c : line) { hash ^= c; hash *= 1099511628211ULL; }
        hash ^= '\n'; hash *= 1099511628211ULL;

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

    std::ostringstream hash_text;
    hash_text << std::hex << hash;
    grammar_hash = hash_text.str();

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
        const std::set<std::string>& sym_first = first_sets[symbol];

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

std::vector<std::string> Parser::body(int prod) {
    if (grammar[prod].right.size() == 1 && grammar[prod].right[0] == "epsilon") return {};
    return grammar[prod].right;
}

ItemSet Parser::computeClosure(ItemSet s) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Item& item : ItemSet(s)) {   // Se recorre una copia porque s crece
            std::vector<std::string> b = body(item.prod);
            if (item.dot >= (int)b.size() || isTerminal(b[item.dot])) continue;

            // FIRST(delta a) solo depende de (producción, punto, anticipación): se guarda
            const auto key = std::make_tuple(item.prod, item.dot, item.lookahead);
            auto cached = closure_first_cache.find(key);
            if (cached == closure_first_cache.end()) {
                std::vector<std::string> delta_a(b.begin() + item.dot + 1, b.end());
                delta_a.push_back(lookaheads[item.lookahead]);
                std::vector<int> indexes;
                for (const std::string& t : computeFirstOfString(delta_a)) {
                    if (t != "epsilon") indexes.push_back(lookahead_index.at(t));
                }
                cached = closure_first_cache.emplace(key, indexes).first;
            }

            for (size_t q = 0; q < grammar.size(); ++q) {
                if (grammar[q].left != b[item.dot]) continue;
                for (int la : cached->second) {
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

    lookahead_index.clear();
    for (size_t i = 0; i < lookaheads.size(); ++i) lookahead_index[lookaheads[i]] = (int)i;
    closure_first_cache.clear();

    collection = { computeClosure({ { 0, 0, 0 } }) };   // Anticipación 0 = eof
    transitions.clear();

    std::map<ItemSet, int> state_index = { { collection[0], 0 } };   // búsqueda rápida de cc existentes

    for (size_t i = 0; i < collection.size(); ++i) {
        for (const auto& x : symbols) {
            ItemSet next = computeGoto(collection[i], x);
            if (next.empty()) continue;

            // Si el conjunto ya existe se reutiliza; si no, es un cc nuevo
            auto found = state_index.find(next);
            int j;
            if (found == state_index.end()) {
                j = (int)collection.size();
                state_index[next] = j;
                collection.push_back(next);
            } else {
                j = found->second;
            }
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
    std::cout << "--- Colección Canónica LR(1) ---\n";
    for (size_t i = 0; i < collection.size(); ++i) {
        std::cout << "cc" << i << ":\n";
        for (const Item& item : collection[i]) {
            std::cout << "    " << itemText(item) << '\n';
        }
        for (const auto& x : symbols) {
            if (transitions.count({ (int)i, x })) {
                std::cout << "    goto(cc" << i << ", " << x << ") = cc" << transitions.at({ (int)i, x }) << '\n';
            }
        }
        std::cout << '\n';
    }
    std::cout.flush();
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

// ===========================================================================
//  Caché de las tablas
//  Archivo de texto: encabezado (versión, hash de la gramática, nº de
//  producciones), símbolos, anticipaciones, cc, transiciones, Action y Goto,
//  y la marca final END. Si algo no coincide o falta, se calcula de nuevo.
// ===========================================================================

static const int CACHE_VERSION = 1;   // Subirla si cambia el algoritmo o el formato
static const size_t CACHE_MAX_COUNT = 50000000;

bool Parser::saveTablesCache(const std::string& filename) {
    if (collection.empty() || grammar_hash.empty()) return false;
    for (const auto& cell : action) {
        if (cell.second.size() > 1) return false;   // Nunca se guardan tablas con conflictos
    }

    std::ofstream out(filename);
    if (!out) return false;

    const auto writeItem = [&out](const Item& item) {
        out << ' ' << item.prod << ' ' << item.dot << ' ' << item.lookahead;
    };

    out << "LR1CACHE " << CACHE_VERSION << ' ' << grammar_hash << ' ' << grammar.size() << '\n';
    out << "SYMBOLS " << symbols.size();
    for (const auto& x : symbols) out << ' ' << x;
    out << "\nLOOKAHEADS " << lookaheads.size();
    for (const auto& x : lookaheads) out << ' ' << x;
    out << "\nSTATES " << collection.size() << '\n';
    for (const ItemSet& cc : collection) {
        out << cc.size();
        for (const Item& item : cc) writeItem(item);
        out << '\n';
    }
    out << "TRANSITIONS " << transitions.size() << '\n';
    for (const auto& t : transitions) {
        out << t.first.first << ' ' << t.first.second << ' ' << t.second << '\n';
    }
    out << "ACTION " << action.size() << '\n';
    for (const auto& cell : action) {
        out << cell.first.first << ' ' << cell.first.second << ' ' << cell.second.size();
        for (const auto& entry : cell.second) {
            out << ' ' << entry.first << ' ' << entry.second.size();
            for (const Item& item : entry.second) writeItem(item);
        }
        out << '\n';
    }
    out << "GOTO " << goto_table.size() << '\n';
    for (const auto& g : goto_table) {
        out << g.first.first << ' ' << g.first.second << ' ' << g.second << '\n';
    }
    out << "END\n";
    out.flush();
    return static_cast<bool>(out);
}

bool Parser::loadTablesCache(const std::string& filename) {
    std::ifstream in(filename);
    if (!in || grammar_hash.empty()) return false;

    std::string word, hash;
    int version = 0;
    size_t productions = 0;
    if (!(in >> word >> version >> hash >> productions)) return false;
    if (word != "LR1CACHE" || version != CACHE_VERSION ||
        hash != grammar_hash || productions != grammar.size()) return false;

    // Todo se lee en variables locales y solo al final se copia al objeto
    std::vector<std::string> new_symbols, new_lookaheads;
    std::vector<ItemSet> new_collection;
    std::map<std::pair<int, std::string>, int> new_transitions, new_goto;
    std::map<std::pair<int, std::string>, std::map<std::string, std::vector<Item>>> new_action;
    size_t count = 0;

    const auto readList = [&](const char* name, std::vector<std::string>& list) {
        if (!(in >> word >> count) || word != name || count > CACHE_MAX_COUNT) return false;
        for (size_t i = 0; i < count; ++i) {
            std::string x;
            if (!(in >> x)) return false;
            list.push_back(x);
        }
        return true;
    };
    const auto readItem = [&](Item& item) {
        if (!(in >> item.prod >> item.dot >> item.lookahead)) return false;
        if (item.prod < 0 || item.prod >= (int)productions) return false;
        if (item.dot < 0 || item.dot > (int)body(item.prod).size()) return false;
        return item.lookahead >= 0 && item.lookahead < (int)new_lookaheads.size();
    };

    if (!readList("SYMBOLS", new_symbols) || !readList("LOOKAHEADS", new_lookaheads)) return false;
    if (new_lookaheads.empty() || new_lookaheads[0] != "$") return false;
    const std::set<std::string> symbol_set(new_symbols.begin(), new_symbols.end());
    const std::set<std::string> lookahead_set(new_lookaheads.begin(), new_lookaheads.end());

    size_t states = 0;
    if (!(in >> word >> states) || word != "STATES" || states == 0 || states > CACHE_MAX_COUNT) return false;
    for (size_t i = 0; i < states; ++i) {
        ItemSet cc;
        if (!(in >> count) || count > CACHE_MAX_COUNT) return false;
        for (size_t k = 0; k < count; ++k) {
            Item item;
            if (!readItem(item)) return false;
            cc.insert(item);
        }
        new_collection.push_back(cc);
    }

    const auto stateOk = [states](int s) { return s >= 0 && s < (int)states; };

    if (!(in >> word >> count) || word != "TRANSITIONS" || count > CACHE_MAX_COUNT) return false;
    for (size_t i = 0; i < count; ++i) {
        int from, to;
        std::string x;
        if (!(in >> from >> x >> to) || !stateOk(from) || !stateOk(to) || !symbol_set.count(x)) return false;
        new_transitions[{ from, x }] = to;
    }

    if (!(in >> word >> count) || word != "ACTION" || count > CACHE_MAX_COUNT) return false;
    for (size_t i = 0; i < count; ++i) {
        int state;
        std::string terminal;
        size_t actions;
        if (!(in >> state >> terminal >> actions) || !stateOk(state) ||
            !lookahead_set.count(terminal) || actions != 1) return false;   // sin conflictos: una acción por celda
        std::string name;
        size_t items;
        if (!(in >> name >> items) || name.empty() || items > CACHE_MAX_COUNT) return false;
        std::vector<Item> list;
        for (size_t k = 0; k < items; ++k) {
            Item item;
            if (!readItem(item)) return false;
            list.push_back(item);
        }
        new_action[{ state, terminal }][name] = list;
    }

    if (!(in >> word >> count) || word != "GOTO" || count > CACHE_MAX_COUNT) return false;
    for (size_t i = 0; i < count; ++i) {
        int from, to;
        std::string x;
        if (!(in >> from >> x >> to) || !stateOk(from) || !stateOk(to) || !symbol_set.count(x)) return false;
        new_goto[{ from, x }] = to;
    }

    if (!(in >> word) || word != "END") return false;

    symbols = new_symbols;
    lookaheads = new_lookaheads;
    lookahead_index.clear();
    for (size_t i = 0; i < lookaheads.size(); ++i) lookahead_index[lookaheads[i]] = (int)i;
    closure_first_cache.clear();
    collection = new_collection;
    transitions = new_transitions;
    action = new_action;
    goto_table = new_goto;
    return true;
}
