

# Recursively collect all header files from the install directory
file(GLOB_RECURSE HEADER_FILES_TOP  ${JSONUI_HEADER_DIR}/jsonqml/*.h )

# Recursively collect all header files from the internal directory
file(GLOB_RECURSE HEADER_FILES  *.h )

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES  *.cpp )

file(GLOB QRC_FILES *.qrc)
qt_add_resources(JSONUI_SRCS ${QRC_FILES})


# Check if a shared library is to be built
if(BUILD_SHARED_LIBS)
   # Enable automatic creation of a module definition (.def) file for a SHARED library on Windows.
   set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

   qt_add_library(jsonui SHARED
        ${SOURCE_FILES} ${HEADER_FILES_TOP} 
        ${HEADER_FILES} ${JSONUI_SRCS}
   )
    

    add_library(jsonui::jsonui ALIAS jsonui)
    target_link_libraries(jsonui PUBLIC jsonio::jsonio)
    target_link_libraries(jsonui PUBLIC Qt6::Widgets Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Charts Qt6::Svg)

    install( TARGETS jsonui
         EXPORT jsonuiTargets DESTINATION "lib" COMPONENT libraries)

endif()

# Check if a static library is to be built
if(BUILD_STATIC_LIBS)

   qt_add_library(jsonui-static STATIC
        ${SOURCE_FILES} ${HEADER_FILES_TOP} 
        ${HEADER_FILES} ${JSONUI_SRCS} 
   )
    
    add_library(jsonui::jsonui ALIAS jsonui-static)
    target_link_libraries(jsonui-static  PUBLIC jsonio::jsonio)
    target_link_libraries(jsonui-static  PUBLIC Qt6::Widgets Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Charts Qt6::Svg)

    install( TARGETS jsonui-static
         EXPORT jsonuiTargets DESTINATION "lib" COMPONENT libraries)


endif()

install(
    DIRECTORY   "${JSONUI_HEADER_DIR}/jsonqml"
    DESTINATION include
)
