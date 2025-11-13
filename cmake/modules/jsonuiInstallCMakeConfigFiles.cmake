# The path where cmake config files are installed
set(JSONQML_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/jsonqml)

install(EXPORT jsonqmlTargets
    FILE jsonqmlTargets.cmake
    NAMESPACE jsonqml::
    DESTINATION ${JSONQML_INSTALL_CONFIGDIR}
    COMPONENT cmake)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/jsonqmlConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/jsonqmlConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/jsonqmlConfig.cmake
    INSTALL_DESTINATION ${JSONQML_INSTALL_CONFIGDIR}
    PATH_VARS JSONQML_INSTALL_CONFIGDIR)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/jsonqmlConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/jsonqmlConfigVersion.cmake
    DESTINATION ${JSONQML_INSTALL_CONFIGDIR})
