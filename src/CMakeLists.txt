set(
    PASCLANG_SOURCE_FILES
    AST/Types.cpp
    AST/PPPrinter.cpp
    Parsing/Driver.cpp
    Parsing/Lexer.cpp
    Parsing/Parser.cpp
    Semantic/TypeChecker.cpp
    Message/ConsoleReporter.cpp
    LLVMBackend/IRGenerator.cpp
    LLVMBackend/IROptimizer.cpp
    LLVMBackend/ObjectGenerator.cpp
)

add_executable(
    pasclang
    main.cpp
    ${PASCLANG_SOURCE_FILES}
)

if(PASCLANG_BUILD_SHARED)
    add_library(pasclg SHARED ${PASCLANG_SOURCE_FILES})
    install(TARGETS pasclg LIBRARY DESTINATION "lib/pasclang/${VERSION}")
endif(PASCLANG_BUILD_SHARED)

llvm_map_components_to_libnames(LLVM_LIBS support core irreader native)
target_link_libraries(pasclang ${LLVM_LIBS})

install(TARGETS pasclang
    RUNTIME DESTINATION bin
)
