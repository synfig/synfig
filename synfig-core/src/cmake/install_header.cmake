## TODO: deduce DEST from CMAKE_CURRENT_LIST_DIR
function(install_all_headers DEST)
    file(GLOB HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

    SYNFIG_INSTALL(
        FILES ${HEADERS}
        DESTINATION include/synfig/${DEST}
    )
endfunction(install_all_headers)
