#ifndef PANIC_RECOVERY_H
#define PANIC_RECOVERY_H

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

struct PanicRecoveryOutcome {
    bool recovered = false;
    std::string non_terminal;
    std::size_t discarded_tokens = 0;
    std::size_t popped_states = 0;
    int target_state = -1;
    std::string failure;
};

class PanicRecovery {
public:
    using FollowSets = std::map<std::string, std::set<std::string>>;
    using GotoLookup = std::function<std::optional<int>(int, const std::string&)>;

    static PanicRecoveryOutcome recover(
        std::vector<int>& state_stack,
        const std::vector<std::string>& tokens,
        std::size_t& input_position,
        const std::set<std::string>& panic_nonterminals,
        const FollowSets& follow_sets,
        const GotoLookup& goto_lookup) {
        std::string anchor;
        std::optional<int> target_state;
        std::size_t popped_states = 0;

        while (!state_stack.empty()) {
            for (const std::string& non_terminal : panic_nonterminals) {
                target_state = goto_lookup(state_stack.back(), non_terminal);
                if (target_state) {
                    anchor = non_terminal;
                    break;
                }
            }

            if (target_state) break;
            state_stack.pop_back();
            ++popped_states;
        }

        if (!target_state) {
            return { false, {}, 0, popped_states, -1, "la pila de estados quedó vacía" };
        }

        const auto follow = follow_sets.find(anchor);
        if (follow == follow_sets.end()) {
            return { false, anchor, 0, popped_states, *target_state,
                     "no se encontró FOLLOW(" + anchor + ")" };
        }

        std::size_t discarded = 0;
        while (input_position < tokens.size() &&
               follow->second.count(tokens[input_position]) == 0) {
            if (tokens[input_position] == "$") {
                return { false, anchor, discarded,
                         popped_states,
                         *target_state,
                         "se alcanzó el fin de entrada antes de FOLLOW(" + anchor + ")" };
            }
            ++input_position;
            ++discarded;
        }

        if (input_position == tokens.size()) {
            return { false, anchor, discarded,
                     popped_states,
                     *target_state,
                     "se alcanzó el fin de entrada antes de FOLLOW(" + anchor + ")" };
        }

        return { true, anchor, discarded, popped_states, *target_state, {} };
    }
};

#endif
