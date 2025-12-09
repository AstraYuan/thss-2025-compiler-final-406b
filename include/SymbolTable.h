#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Type.h"

class Symbol {
public:
    std::string name;
    std::shared_ptr<Type> type;
    bool isConst;
    int intValue;  // For constant values
    std::vector<int> arrayValues;  // For constant arrays
    
    Symbol(const std::string& n, std::shared_ptr<Type> t, bool c = false)
        : name(n), type(t), isConst(c), intValue(0) {}
};

class SymbolTable {
private:
    std::vector<std::map<std::string, std::shared_ptr<Symbol>>> scopes;
    
public:
    SymbolTable() {
        // Create global scope
        scopes.push_back(std::map<std::string, std::shared_ptr<Symbol>>());
    }
    
    void enterScope() {
        scopes.push_back(std::map<std::string, std::shared_ptr<Symbol>>());
    }
    
    void exitScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
        }
    }
    
    bool addSymbol(const std::string& name, std::shared_ptr<Symbol> symbol) {
        // Check if symbol already exists in current scope
        if (scopes.back().find(name) != scopes.back().end()) {
            return false;
        }
        scopes.back()[name] = symbol;
        return true;
    }
    
    std::shared_ptr<Symbol> lookup(const std::string& name) {
        // Search from innermost to outermost scope
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return nullptr;
    }
    
    std::shared_ptr<Symbol> lookupInCurrentScope(const std::string& name) {
        auto found = scopes.back().find(name);
        if (found != scopes.back().end()) {
            return found->second;
        }
        return nullptr;
    }
    
    bool isGlobalScope() const {
        return scopes.size() == 1;
    }
};

#endif // SYMBOLTABLE_H
