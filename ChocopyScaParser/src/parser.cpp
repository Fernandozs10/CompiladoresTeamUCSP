#include "parser.h"
#include "panic_recovery.h"

#include <algorithm>
#include <optional>
#include <stdexcept>

Parser::Parser() {}

bool Parser::isTerminal(const std::string& symbol) {
    if (symbol.empty()) return false;
    if (symbol == "epsilon" || symbol == "$") return true;
    return non_terminals.find(symbol) == non_terminals.end();
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
    user_start_symbol.clear();
    augmented_start.clear();

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
            is_first_rule = false;
        }

        non_terminals.insert(left);

        Production prod;
        prod.left = left;

        while (ss >> symbol) {
            if (symbol == "|") {
                grammar.push_back(prod);
                prod.right.clear();
                continue;
            }

            prod.right.push_back(symbol);
        }
        
        if (!prod.right.empty()) {
            grammar.push_back(prod);
        }
    }

    file.close();

    if (grammar.empty() || user_start_symbol.empty()) {
        std::cerr << "Error: El archivo " << filename
                  << " no contiene producciones válidas.\n";
        return false;
    }

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

    for (const Production& production : grammar) {
        for (const std::string& symbol : production.right) {
            if (isTerminal(symbol)) terminals.insert(symbol);
        }
    }

    terminals.insert("epsilon");
    terminals.insert("$");

    return true;
}

std::set<std::string> Parser::computeFirstOfString(const std::vector<std::string>& beta) {
    std::set<std::string> result;

    if (beta.empty()) {
        result.insert("epsilon");
        return result;
    }

    bool all_nullable = true;

    for (const std::string& symbol : beta) {
        std::set<std::string> sym_first = first_sets[symbol];

        for (const auto& s : sym_first) {
            if (s != "epsilon") {
                result.insert(s);
            }
        }

        if (sym_first.count("epsilon") == 0) {
            all_nullable = false;
            break;
        }
    }

    if (all_nullable) {
        result.insert("epsilon");
    }

    return result;
}

void Parser::computeFirstSets() {
    first_sets.clear();

    for (const auto& t : terminals) {
        first_sets[t].insert(t);
    }

    for (const auto& nt : non_terminals) {
        first_sets[nt] = std::set<std::string>();
    }

    bool changed = true;

    while (changed) {
        changed = false;

        for (const auto& prod : grammar) {
            std::string A = prod.left;
            const std::vector<std::string>& beta = prod.right;

            if (beta.size() == 1 && beta[0] == "epsilon") {
                if (first_sets[A].count("epsilon") == 0) {
                    first_sets[A].insert("epsilon");
                    changed = true;
                }
                continue;
            }

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

    for (const auto& t : terminals) {
        if (t == "epsilon") continue;
        std::cout << "FIRST(" << showSymbol(t) << ") = { " << showSymbol(t) << " }" << std::endl;
    }
    std::cout << std::endl;
}


std::vector<std::string> Parser::body(int prod) {
    if (grammar[prod].right.size() == 1 && grammar[prod].right[0] == "epsilon") return {};
    return grammar[prod].right;
}


ItemSet Parser::computeClosure(ItemSet s) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Item& item : ItemSet(s)) {   
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

    int conflicts = 0;
    for (const auto& cell : action) {
        if (cell.second.size() > 1) conflicts++;
    }
    return conflicts;
}

std::map<std::string, std::set<std::string>> Parser::computeFollowSets() {
    std::map<std::string, std::set<std::string>> follow_sets;
    for (const std::string& non_terminal : non_terminals) {
        follow_sets[non_terminal];
    }
    follow_sets[user_start_symbol].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t production_index = 0; production_index < grammar.size();
             ++production_index) {
            const Production& production = grammar[production_index];
            const std::vector<std::string> rhs = body(static_cast<int>(production_index));
            for (std::size_t i = 0; i < rhs.size(); ++i) {
                const std::string& symbol = rhs[i];
                if (non_terminals.count(symbol) == 0) continue;

                std::vector<std::string> suffix(rhs.begin() + i + 1, rhs.end());
                const std::set<std::string> suffix_first = computeFirstOfString(suffix);
                std::set<std::string>& symbol_follow = follow_sets[symbol];
                const std::size_t original_size = symbol_follow.size();

                for (const std::string& terminal : suffix_first) {
                    if (terminal != "epsilon") symbol_follow.insert(terminal);
                }
                if (suffix_first.count("epsilon") > 0) {
                    const std::set<std::string>& left_follow = follow_sets[production.left];
                    symbol_follow.insert(left_follow.begin(), left_follow.end());
                }

                if (symbol_follow.size() != original_size) changed = true;
            }
        }
    }
    return follow_sets;
}

ParseResult Parser::parseWithPanicRecovery(
    const std::vector<std::string>& tokens,
    const std::set<std::string>& panic_nonterminals,
    const std::vector<ParseTokenLocation>& locations) {
    if (grammar.empty() || collection.empty() || action.empty()) {
        throw std::logic_error(
            "Primero cargue la gramática y construya la colección y las tablas LR.");
    }
    if (panic_nonterminals.empty()) {
        throw std::invalid_argument("El conjunto de no terminales de pánico no puede estar vacío.");
    }
    for (const std::string& non_terminal : panic_nonterminals) {
        if (non_terminals.count(non_terminal) == 0) {
            throw std::invalid_argument(
                "El símbolo de pánico '" + non_terminal + "' no es un no terminal de la gramática.");
        }
    }
    for (const auto& cell : action) {
        if (cell.second.size() > 1) {
            throw std::logic_error(
                "No se puede analizar con una tabla LR que contiene conflictos.");
        }
    }
    if (std::find(tokens.begin(), tokens.end(), "epsilon") != tokens.end()) {
        throw std::invalid_argument("'epsilon' no es un token válido de entrada.");
    }
    const auto eof = std::find(tokens.begin(), tokens.end(), "$");
    if (eof != tokens.end() && eof + 1 != tokens.end()) {
        throw std::invalid_argument("'$' solo puede aparecer al final de la entrada.");
    }

    std::vector<std::string> input = tokens;
    if (input.empty() || input.back() != "$") input.push_back("$");

    const auto follow_sets = computeFollowSets();
    ParseResult result;
    std::vector<int> state_stack{ 0 };
    std::vector<std::string> symbol_stack{ "$" };
    std::size_t input_position = 0;
    std::optional<std::size_t> previous_error_position;

    const auto token_location = [&locations](std::size_t position) -> ParseTokenLocation {
        if (position < locations.size()) return locations[position];
        return ParseTokenLocation{};
    };

    const auto join = [](const std::vector<std::string>& values, std::size_t start) {
        std::string text;
        for (std::size_t i = start; i < values.size(); ++i) {
            if (!text.empty()) text += " ";
            text += values[i];
        }
        return text;
    };
    const auto state_text = [&state_stack]() {
        std::string text;
        for (int current_state : state_stack) {
            if (!text.empty()) text += " ";
            text += std::to_string(current_state);
        }
        return text;
    };
    const auto record_trace = [&](std::size_t position, const std::string& action_text,const std::string& kind) {
        result.trace.push_back({
            state_text(),
            join(symbol_stack, 0),
            join(input, position),
            action_text,
            kind
        });
    };

    while (input_position < input.size()) {
        const int state = state_stack.back();
        const std::string& lookahead = input[input_position];
        const auto cell = action.find({ state, lookahead });

        if (cell == action.end() || cell->second.empty()) {
            record_trace(input_position,"error: Action[" + std::to_string(state) + ", " + lookahead +"] vacía","error");
            ++result.syntax_errors;
            const std::size_t error_position = input_position;
            std::string diagnostic = "Error sintáctico en el token '" + lookahead +
                                     "' (posición " + std::to_string(error_position) + ").";
            const ParseTokenLocation location = token_location(error_position);
            if (location.line > 0) {
                diagnostic += " Línea " + std::to_string(location.line);
                if (location.column > 0) diagnostic += ", columna " + std::to_string(location.column);
                if (!location.source_line.empty()) {
                    diagnostic += "\n    " + location.source_line;
                    if (location.column > 0) {
                        diagnostic += "\n    " + std::string(location.column - 1, ' ') + '^';
                    }
                }
            }
            result.diagnostics.push_back(diagnostic);

            if (previous_error_position && *previous_error_position == error_position) {
                if (lookahead == "$") {
                    result.aborted = true;
                    result.diagnostics.push_back(
                        "Se abortó la recuperación: otro error ocurrió en fin de entrada.");
                    break;
                }
                ++input_position;
                ++result.discarded_tokens;
                record_trace(input_position,"sin avance: se descarta " + lookahead,"recovery");
                result.diagnostics.push_back("Sin avance desde el error anterior; se descartó el token '" +lookahead + "'.");
            }
            previous_error_position = error_position;

            const std::size_t recovery_start = input_position;
            const PanicRecoveryOutcome recovery = PanicRecovery::recover(
                state_stack,
                input,
                input_position,
                panic_nonterminals,
                follow_sets,
                [this](int current_state, const std::string& non_terminal) -> std::optional <int> {
                    const auto transition = goto_table.find({ current_state, non_terminal });
                    if (transition == goto_table.end()) return std::nullopt;
                    return transition->second;
                });
            result.discarded_tokens += recovery.discarded_tokens;

            if (recovery.popped_states > 0) {
                const std::size_t symbols_to_pop =
                    std::min(recovery.popped_states, symbol_stack.size());
                symbol_stack.resize(symbol_stack.size() - symbols_to_pop);
                record_trace(error_position,"error-follow 1: desapila " + std::to_string(recovery.popped_states) +
                " estado(s); busca Goto para " +(recovery.non_terminal.empty() ? "N" : recovery.non_terminal),"recovery");
            }

            if (!recovery.non_terminal.empty()) {
                std::string skipped_tokens;
                for (std::size_t i = recovery_start; i < input_position; ++i) {
                    if (!skipped_tokens.empty()) skipped_tokens += " ";
                    skipped_tokens += input[i];
                }
                record_trace(input_position,"error-follow 2: consume " +(skipped_tokens.empty() ? "nada" : skipped_tokens) +
                " hasta FOLLOW(" + recovery.non_terminal + ")","recovery");
            }

            if (!recovery.recovered) {
                record_trace(input_position,"fin de entrada o sin anclaje; aborta","error");
                result.aborted = true;
                result.diagnostics.push_back(
                    "Se abortó la recuperación: " + recovery.failure + ".");
                break;
            }
            state_stack.push_back(recovery.target_state);
            symbol_stack.push_back(recovery.non_terminal);
            record_trace(input_position,"error-follow 3 y 4: apila " + recovery.non_terminal + 
                " y Goto = " +std::to_string(state_stack.back()),"recovery");
            result.diagnostics.push_back(
                "Recuperación mediante " + recovery.non_terminal + "; se descartaron " +
                std::to_string(recovery.discarded_tokens) + " token(s).");
            continue;
        }

        const std::string& selected_action = cell->second.begin()->first;
        if (selected_action == "acc") {
            record_trace(input_position, "acepta", "normal");
            result.accepted = true;
            break;
        }
        if (selected_action.empty()) {
            throw std::logic_error("La tabla LR contiene una acción vacía.");
        }

        if (selected_action[0] == 's') {
            const int next_state = std::stoi(selected_action.substr(1));
            record_trace(input_position,"shift " + lookahead + " a " + std::to_string(next_state),"normal");
            state_stack.push_back(next_state);
            symbol_stack.push_back(lookahead);
            ++input_position;
            continue;
        }
        if (selected_action[0] == 'r') {
            const int production_index = std::stoi(selected_action.substr(1));
            if (production_index < 0 ||
                static_cast<std::size_t>(production_index) >= grammar.size()) {
                throw std::logic_error("La tabla LR referencia una producción inexistente.");
            }
            const std::vector<std::string> rhs = body(production_index);
            std::string production_text = grammar[production_index].left + " ->";
            for (const std::string& symbol : rhs) production_text += " " + symbol;
            record_trace(input_position,"reduce " + std::to_string(production_index) + ": " + production_text,"normal");
            if (rhs.size() >= state_stack.size()) {
                throw std::logic_error("La reducción vaciaría la pila de estados LR.");
            }
            state_stack.resize(state_stack.size() - rhs.size());
            if (rhs.size() > symbol_stack.size()) {
                throw std::logic_error("La reducción vaciaría la pila de símbolos LR.");
            }
            symbol_stack.resize(symbol_stack.size() - rhs.size());
            const auto transition = goto_table.find(
                { state_stack.back(), grammar[production_index].left });
            if (transition == goto_table.end()) {
                throw std::logic_error("Falta una transición Goto para una reducción LR.");
            }
            state_stack.push_back(transition->second);
            symbol_stack.push_back(grammar[production_index].left);
            continue;
        }

        throw std::logic_error("La tabla LR contiene una acción desconocida.");
    }

    return result;
}

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
