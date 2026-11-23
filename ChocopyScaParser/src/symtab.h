#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>
#include <vector>

class Scope;

// una entrada de la tabla: lo que se sabe de un nombre
struct Symbol {
    std::string name;
    std::string kind;                         // "var", "param", "func", "class" o "ref"
    std::string type;                         // tipo de la variable; tipo de retorno si es funcion
    std::vector<std::string> parameterTypes;  // solo funciones: el tipo de cada parametro
    Symbol* referencedSymbol = nullptr;       // solo "ref": la variable declarada con global o nonlocal
    Scope* memberScope = nullptr;             // solo clases: tabla con sus atributos y metodos
    std::string parentClassName;              // solo clases: nombre de la clase padre
};

// una tabla por scope. cada tabla apunta a la del scope que la contiene (parentScope).
class Scope {
public:
    Scope(const std::string& name, Scope* parent) : scopeName(name), parentScope(parent) {}

    // inserta el simbolo solo en esta tabla. devuelve false si el nombre ya estaba aqui.
    bool insertSymbol(const Symbol& symbol) {
        return symbolsByName.emplace(symbol.name, symbol).second;
    }

    // busca solo en esta tabla
    Symbol* lookupInThisScope(const std::string& symbolName) {
        const auto found = symbolsByName.find(symbolName);
        return found == symbolsByName.end() ? nullptr : &found->second;
    }

    // busca en esta tabla y, si no esta, en la de su parentScope, y asi hasta la global.
    // si encuentra un "ref" (global/nonlocal) devuelve la variable a la que apunta.
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
    std::unordered_map<std::string, Symbol> symbolsByName;   // tabla hash: nombre -> simbolo
};

#endif
