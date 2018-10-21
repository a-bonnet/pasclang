#include "AST/Types.h"

#include <iostream>
#include <memory>

namespace pasclang::AST {

// Returns the shared structure pointer to the requested type
const TableOfTypes::Type* TableOfTypes::get(TypeKind kind,
                                            std::uint32_t dimension) const {
    auto resultKind = this->tableOfTypes.find(kind);
    if (resultKind != this->tableOfTypes.end()) {
        auto resultDimension = resultKind->second.find(dimension);
        if (resultDimension != resultKind->second.end()) {
            return resultDimension->second.get();
        } else {
            this->tableOfTypes[kind][dimension] =
                std::make_unique<Type>(this, kind, dimension);
            return this->tableOfTypes[kind][dimension].get();
        }
    }
    tableOfTypes[kind][dimension] =
        std::make_unique<Type>(this, kind, dimension);
    return tableOfTypes[kind][dimension].get();
}

} // namespace pasclang::AST
