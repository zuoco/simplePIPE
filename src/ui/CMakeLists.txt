add_library(ui STATIC placeholder.cpp)

target_include_directories(ui
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(ui PUBLIC cxx_std_17)

target_link_libraries(ui
    PUBLIC
        app
        Qt6::Quick
        Qt6::Qml
)
