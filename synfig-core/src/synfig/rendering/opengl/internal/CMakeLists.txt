target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/context.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/environment.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/framebuffer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/shaders.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/plane.cpp"
)

set(RENDERING_OPENGL_INTERNAL_SHADER_FILES
	"shaders/solid.fs"
	"shaders/colormatrix.fs"
	"shaders/blend.fs"
	"shaders/blit.fs"
	"shaders/basic.vs"
)

foreach (SHADER IN ITEMS ${RENDERING_OPENGL_INTERNAL_SHADER_FILES})
	configure_file("${CMAKE_CURRENT_LIST_DIR}/${SHADER}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER}" COPYONLY)
endforeach ()

# TODO: install shaders
