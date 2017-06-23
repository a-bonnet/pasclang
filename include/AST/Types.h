// The class that manages the different types used in the program. Types are uniqued
// to avoid allocating on the heap almost as many as there are tree nodes. They are
// then accessed with the static TableOfTypes::get method.

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

