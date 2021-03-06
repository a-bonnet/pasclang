// The class that manages the different types used in the program. Types are
// uniqued to avoid allocating on the heap almost as many as there are tree
// nodes. They are then accessed with the static TableOfTypes::get method.

#ifndef PASCLANG_AST_TYPES_H
#define PASCLANG_AST_TYPES_H

#include <map>
#include <memory>

namespace pasclang::AST {

// Holds shared types for a program. Only creates new types when a new type is
// requested.
class TableOfTypes {
  public:
    enum TypeKind { Boolean, Integer };

    struct Type {
      private:
        const TableOfTypes* parentTable;

      public:
        TypeKind kind = TypeKind::Boolean;
        mutable std::uint32_t dimension = 0;

        Type(const TableOfTypes* parentTable, TypeKind t, std::uint32_t d)
            : parentTable(parentTable), kind(t), dimension(d) {}
        const Type* increaseDimension() const {
            return parentTable->get(kind, dimension + 1);
        }
    };

  private:
    mutable std::map<TypeKind, std::map<std::uint32_t, std::unique_ptr<Type>>>
        tableOfTypes;

  public:
    TableOfTypes() {}
    ~TableOfTypes() {}
    const Type* get(TypeKind kind, std::uint32_t dimension) const;
};

} // namespace pasclang::AST

#endif
