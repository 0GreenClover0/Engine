# Add source files
file(GLOB_RECURSE SOURCE_FILES 
     *.c
     *.cpp)

# Add header files
file(GLOB_RECURSE HEADER_FILES 
     *.h
     *.hpp)

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")

# Set FW1 directories
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(FW1_DIR "${PARENT_DIR}/thirdparty/FW1")
set(FW1_SOURCE_DIR ${FW1_DIR} CACHE INTERNAL "")

# Set EHT directory
set(EHT_DIR "${PARENT_DIR}/EngineHeaderTool")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                  ${glad_SOURCE_DIR}
                                                  ${stb_image_SOURCE_DIR}
                                                  ${imgui_SOURCE_DIR}
                                                  ${miniaudio_SOURCE_DIR}
                                                  ${FW1_SOURCE_DIR}
                                                  ${imguizmo_SOURCE_DIR}
                                                  ${implot_SOURCE_DIR}
                                                  ${ddstextureloader_SOURCE_DIR})

add_library(FW1 STATIC IMPORTED)
set_target_properties(FW1 PROPERTIES IMPORTED_LOCATION ${FW1_DIR}/FW1FontWrapper.lib)
target_link_libraries(${PROJECT_NAME} FW1)

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} stb_image)
target_link_libraries(${PROJECT_NAME} assimp)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} spdlog)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} yaml-cpp)
target_link_libraries(${PROJECT_NAME} miniaudio)
target_link_libraries(${PROJECT_NAME} imguizmo)
target_link_libraries(${PROJECT_NAME} implot)
target_link_libraries(${PROJECT_NAME} ddstextureloader)

# Copy FW1FontWrapper.dll to output directory after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${FW1_DIR}/FW1FontWrapper.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying FW1FontWrapper.dll to output directory"
)

if(NOT DEFINED ENV{IS_CI})
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND py
            "${EHT_DIR}/EngineHeaderTool.py -d \"${PARENT_DIR}\""
        COMMENT "Running EngineHeaderTool"
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_SOURCE_DIR}/res
                   ${CMAKE_CURRENT_BINARY_DIR}/res)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()
