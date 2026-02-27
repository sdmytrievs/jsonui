# The path where cmake config files are installed
set(JSONUI_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/jsonui)

install(EXPORT jsonuiTargets
    FILE jsonuiTargets.cmake
    NAMESPACE jsonui::
    DESTINATION ${JSONUI_INSTALL_CONFIGDIR}
    COMPONENT cmake)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/jsonuiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/jsonuiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/jsonuiConfig.cmake
    INSTALL_DESTINATION ${JSONUI_INSTALL_CONFIGDIR}
    PATH_VARS JSONUI_INSTALL_CONFIGDIR)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/jsonuiConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/jsonuiConfigVersion.cmake
    DESTINATION ${JSONUI_INSTALL_CONFIGDIR})
