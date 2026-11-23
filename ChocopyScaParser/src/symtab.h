#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>
#include <vector>

class Scope;

// Una entrada de la tabla: lo que se sabe de un nombre
struct Symbol {
    std::string name;
    std::string kind;                         // "var", "param", "func", "class" o "ref"
    std::string type;                         // tipo de la variable; tipo de retorno si es función
    std::vector<std::string> parameterTypes;  // solo funciones: el tipo de cada parámetro
    Symbol* referencedSymbol = nullptr;       // solo "ref": la variable declarada con global o nonlocal
    Scope* memberScope = nullptr;             // solo clases: tabla con sus atributos y métodos
    std::string parentClassName;              // solo clases: nombre de la clase padre
};

// Una tabla por scope. Cada tabla apunta a la del scope que la contiene (parentScope).
class Scope {
public:
    Scope(const std::string& name, Scope* parent) : scopeName(name), parentScope(parent) {}

    // Inserta el símbolo solo en esta tabla. Devuelve false si el nombre ya estaba aquí.
    bool insertSymbol(const Symbol& symbol) {
        return symbolsByName.emplace(symbol.name, symbol).second;
    }

    // Busca solo en esta tabla
    Symbol* lookupInThisScope(const std::string& symbolName) {
        const auto found = symbolsByName.find(symbolName);
        return found == symbolsByName.end() ? nullptr : &found->second;
    }

    // Busca en esta tabla y, si no está, en la de su parentScope, y así hasta la global.
    // Si encuentra un "ref" (global/nonlocal) devuelve la variable a la que apunta.
    Symbol* lookupThroughParents(const std::string& symbolName) {
        for (Scope* scope = this; scope != nullptr; scope = scope->parentScope) {
            Symbol* found = scope->lookupInThisScope(symbolName);
            if (found != nullptr) return found->kind == "ref" ? found->referencedSymbol : found;
        }
        return nullptr;
    }

    std::string scopeName;
    Scope* parentScope;

private:
    std::unordered_map<std::string, Symbol> symbolsByName;   // tabla hash: nombre -> símbolo
};

#endif
