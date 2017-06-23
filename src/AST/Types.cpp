#include "AST/Types.h"

#include <iostream>
#include <memory>

namespace pasclang::AST {


static std::map<TableOfTypes::TypeKind, std::map<std::uint32_t, std::unique_ptr<TableOfTypes::Type>>> tableOfTypes;

const TableOfTypes::Type* TableOfTypes::get(TypeKind kind, std::uint32_t dimension)
{
    auto resultKind = tableOfTypes.find(kind);
    if(resultKind != tableOfTypes.end())
    {
        auto resultDimension = resultKind->second.find(dimension);
        if(resultDimension != resultKind->second.end())
        {
            return resultDimension->second.get();
        }
        else
        {
            tableOfTypes[kind][dimension] = std::make_unique<Type>(kind, dimension);
            return tableOfTypes[kind][dimension].get();
        }
    }
    tableOfTypes[kind][dimension] = std::make_unique<Type>(kind, dimension);
    return tableOfTypes[kind][dimension].get();
}

} // namespace pasclang::AST

