

# Recursively collect all header files from the install directory
file(GLOB_RECURSE HEADER_FILES_TOP  ${JSONQML_HEADER_DIR}/jsonqml/*.h )

# Recursively collect all header files from the internal directory
file(GLOB_RECURSE HEADER_FILES  *.h )

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES  *.cpp )

list(APPEND MODULE_QML_FILES
    qml/PreferencesForm.qml
    qml/SelectTableView.qml
    qml/JsonView.qml
    qml/JsonEditor.qml
    qml/VertexKeys.qml
    qml/VertexForm.qml
    qml/EdgeKeys.qml
    qml/EdgeForm.qml
    qml/EditTableView.qml
    qml/ChartsPreferences.qml
    qml/ChartsSeriesView.qml
    qml/SymbolForm.qml
)

list(APPEND MODULE_IMAGES
    qml/images/Incoming24.png
    qml/images/Outgoing24.png
    qml/images/jsonui-logo-icon.png
    qml/images/SaveCurrentRecordIcon24.png
    qml/images/ShowFilesIcon24.png
    qml/images/PlotDataOnGraphIcon24.png
)

# Check if a shared library is to be built
if(BUILD_SHARED_LIBS)

    qt_add_library(jsonqml SHARED)
    qt_add_qml_module(jsonqml
        URI jsonqml
        VERSION 1.0
        RESOURCE_PREFIX /qt/qml
        SOURCES
           ${SOURCE_FILES} ${HEADER_FILES_TOP} ${HEADER_FILES}
        QML_FILES
           ${MODULE_QML_FILES}
        RESOURCES
           ${MODULE_IMAGES}
     )

    add_library(jsonqml::jsonqml ALIAS jsonqml)
    target_link_libraries(jsonqml PUBLIC jsonio::jsonio)
    target_link_libraries(jsonqml PUBLIC Qt6::Gui Qt6::Quick Qt6::Charts Qt6::Svg Qt6::PrintSupport)

    install( TARGETS jsonqml
         EXPORT jsonqmlTargets DESTINATION "lib" COMPONENT libraries)

endif()

install(
    DIRECTORY   "${JSONQML_HEADER_DIR}/jsonqml"
    DESTINATION include
)
