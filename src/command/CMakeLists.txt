add_library(command STATIC
    MacroCommand.cpp
    CommandStack.cpp
    SetPropertyCommand.cpp
    BatchSetPropertyCommand.cpp
)

target_include_directories(command
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(command PUBLIC cxx_std_17)

target_link_libraries(command
    PUBLIC
        model
        foundation
        nlohmann_json::nlohmann_json
)
