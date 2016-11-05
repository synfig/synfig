add_library(mptr_mplayer MODULE "")

target_sources(mptr_mplayer
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/mptr_mplayer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

install (
    TARGETS mptr_mplayer
    DESTINATION lib/synfig/modules
)
