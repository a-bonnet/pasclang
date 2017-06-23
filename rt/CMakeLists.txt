add_library(pasclang-rt STATIC libpasclang-rt.cpp)

install(TARGETS pasclang-rt ARCHIVE DESTINATION lib)
