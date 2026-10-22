file(GLOB parser_build_paths LIST_DIRECTORIES true "${CMAKE_CURRENT_LIST_DIR}/build*")

foreach(parser_build_path IN LISTS parser_build_paths)
    if(IS_DIRECTORY "${parser_build_path}")
        message(STATUS "Eliminando ${parser_build_path}")
        file(REMOVE_RECURSE "${parser_build_path}")
    endif()
endforeach()