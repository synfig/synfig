target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/optimizerblendassociative.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizerblendmerge.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/optimizerblendsplit.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizerblendtotarget.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/optimizercalcbounds.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizerdraft.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/optimizerlinear.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizerlist.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizersplit.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizertransformation.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optimizerpass.cpp"
)
