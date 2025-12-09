#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <memory>
#include <string>

enum class TypeKind {
    INT,
    VOID,
    ARRAY,
    FUNCTION,
    POINTER
};

class Type {
public:
    TypeKind kind;
    
    Type(TypeKind k) : kind(k) {}
    virtual ~Type() = default;
    
    bool isInt() const { return kind == TypeKind::INT; }
    bool isVoid() const { return kind == TypeKind::VOID; }
    bool isArray() const { return kind == TypeKind::ARRAY; }
    bool isFunction() const { return kind == TypeKind::FUNCTION; }
    bool isPointer() const { return kind == TypeKind::POINTER; }
    
    virtual std::string toString() const = 0;
};

class IntType : public Type {
public:
    IntType() : Type(TypeKind::INT) {}
    std::string toString() const override { return "i32"; }
};

class VoidType : public Type {
public:
    VoidType() : Type(TypeKind::VOID) {}
    std::string toString() const override { return "void"; }
};

class ArrayType : public Type {
public:
    std::shared_ptr<Type> elementType;
    std::vector<int> dimensions;
    
    ArrayType(std::shared_ptr<Type> elemType, const std::vector<int>& dims)
        : Type(TypeKind::ARRAY), elementType(elemType), dimensions(dims) {}
    
    int getTotalSize() const {
        int size = 1;
        for (int dim : dimensions) {
            size *= dim;
        }
        return size;
    }
    
    std::string toString() const override {
        std::string result = elementType->toString();
        for (auto it = dimensions.rbegin(); it != dimensions.rend(); ++it) {
            result = "[" + std::to_string(*it) + " x " + result + "]";
        }
        return result;
    }
};

class PointerType : public Type {
public:
    std::shared_ptr<Type> pointeeType;
    
    PointerType(std::shared_ptr<Type> pType)
        : Type(TypeKind::POINTER), pointeeType(pType) {}
    
    std::string toString() const override {
        return pointeeType->toString() + "*";
    }
};

class FunctionType : public Type {
public:
    std::shared_ptr<Type> returnType;
    std::vector<std::shared_ptr<Type>> paramTypes;
    
    FunctionType(std::shared_ptr<Type> retType, const std::vector<std::shared_ptr<Type>>& params)
        : Type(TypeKind::FUNCTION), returnType(retType), paramTypes(params) {}
    
    std::string toString() const override {
        std::string result = returnType->toString() + " (";
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (i > 0) result += ", ";
            result += paramTypes[i]->toString();
        }
        result += ")";
        return result;
    }
};

#endif // TYPE_H
