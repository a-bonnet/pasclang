#ifndef PASCLANG_AST_TYPES_H
#define PASCLANG_AST_TYPES_H

#include <map>
#include <memory>

namespace pasclang::AST {

class TableOfTypes
{
    public:

        enum TypeKind {
            Boolean,
            Integer
        };

        struct Type {
            TypeKind kind = TypeKind::Boolean;
            std::uint32_t dimension = 0;

            Type(TypeKind t, std::uint32_t d) : kind(t), dimension(d) { }
        };

        TableOfTypes() = delete;
        ~TableOfTypes() = delete;
        static const Type* get(TypeKind kind, std::uint32_t dimension);
};

}

#endif

