add_subdirectory(foundation)
add_subdirectory(geometry)
add_subdirectory(model)
add_subdirectory(engine)
add_subdirectory(visualization)
add_subdirectory(vtk-visualization)
add_subdirectory(app)
add_subdirectory(ui)

add_executable(pipecad_app
	main.cpp
)

target_compile_features(pipecad_app PRIVATE cxx_std_17)

target_compile_definitions(pipecad_app
	PRIVATE
		PIPECAD_QML_MAIN="${CMAKE_SOURCE_DIR}/ui/main.qml"
)

target_link_libraries(pipecad_app
	PRIVATE
		ui
		Qt6::Quick
		Qt6::Qml
)
