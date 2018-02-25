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
    install(TARGETS pasclg LIBRARY DESTINATION "lib/pasclang/")
endif(PASCLANG_BUILD_SHARED)

llvm_map_components_to_libnames(LLVM_LIBS support core irreader native codegen)
target_link_libraries(pasclang ${LLVM_LIBS})

add_custom_command(TARGET pasclang POST_BUILD COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
    "To run tests, type make runtests or make runmoretests.")

install(TARGETS pasclang
    RUNTIME DESTINATION bin
)
