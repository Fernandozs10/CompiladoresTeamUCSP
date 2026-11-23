#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>
#include <vector>

class Scope;

// Una entrada de la tabla: lo que se sabe de un nombre
struct Symbol {
    std::string name;
    std::string kind;                     // "var", "param", "func", "class" o "ref"
    std::string type;                     // tipo de la variable; tipo de retorno si es función
    std::vector<std::string> paramTypes;  // solo funciones: el tipo de cada parámetro
    Symbol* refersTo = nullptr;           // solo "ref": la variable declarada con global o nonlocal
    Scope* members = nullptr;             // solo clases: tabla con sus atributos y métodos
    std::string parentClass;              // solo clases: nombre de la clase padre
};

// Una tabla por scope. Cada tabla apunta a la del scope que la contiene (parent).
class Scope {
public:
    Scope(const std::string& scopeName, Scope* parentScope) : name(scopeName), parent(parentScope) {}

    // Inserta el símbolo solo en esta tabla. Devuelve false si el nombre ya estaba aquí.
    bool insert(const Symbol& symbol) {
        return symbols.emplace(symbol.name, symbol).second;
    }

    // Busca solo en esta tabla
    Symbol* lookupHere(const std::string& symbolName) {
        const auto found = symbols.find(symbolName);
        return found == symbols.end() ? nullptr : &found->second;
    }

    // Busca en esta tabla y, si no está, en la de su parent, y así hasta la global.
    // Si encuentra un "ref" (global/nonlocal) devuelve la variable a la que apunta.
    Symbol* lookup(const std::string& symbolName) {
        for (Scope* scope = this; scope != nullptr; scope = scope->parent) {
            Symbol* found = scope->lookupHere(symbolName);
            if (found != nullptr) return found->kind == "ref" ? found->refersTo : found;
        }
        return nullptr;
    }

    std::string name;
    Scope* parent;

private:
    std::unordered_map<std::string, Symbol> symbols;   // tabla hash: nombre -> símbolo
};

#endif
