target_sources(libsynfig
    PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/antialiasing.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/buffers.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/context.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/environment.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/framebuffers.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/misc.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/predeclarations.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/samplers.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders.cpp"
)

find_package(OpenGL REQUIRED)
target_link_libraries(libsynfig PRIVATE OpenGL::GL OpenGL::GLU)

set(RENDERING_OPENGL_INTERNAL_GLSL_FILES
	"${CMAKE_CURRENT_LIST_DIR}/glsl/antialiased_textured_rect_fragment.glsl"
	"${CMAKE_CURRENT_LIST_DIR}/glsl/antialiased_textured_rect_vertex.glsl"
	"${CMAKE_CURRENT_LIST_DIR}/glsl/blend_fragment.glsl"
	"${CMAKE_CURRENT_LIST_DIR}/glsl/color_fragment.glsl"
	"${CMAKE_CURRENT_LIST_DIR}/glsl/simple_vertex.glsl"
	"${CMAKE_CURRENT_LIST_DIR}/glsl/texture_fragment.glsl"
	"${CMAKE_CURRENT_LIST_DIR}/glsl/texture_vertex.glsl"
)

add_custom_command(TARGET libsynfig POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory 
                       "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glsl/"
                   )

add_custom_command(TARGET libsynfig POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
					   ${RENDERING_OPENGL_INTERNAL_GLSL_FILES}
                       "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glsl/"
                   )
