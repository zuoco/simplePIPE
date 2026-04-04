add_library(command STATIC
    MacroCommand.cpp
    CommandStack.cpp
    SetPropertyCommand.cpp
    BatchSetPropertyCommand.cpp
    CreatePipePointCommand.cpp
    DeletePipePointCommand.cpp
    InsertComponentCommand.cpp
    CommandRegistry.cpp
)

target_include_directories(command
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(command PUBLIC cxx_std_17)

target_link_libraries(command
    PUBLIC
        model
        engine
        foundation
        nlohmann_json::nlohmann_json
)
