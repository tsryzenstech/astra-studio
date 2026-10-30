# SPDX-License-Identifier: Apache-2.0
# Astra.Controls, built from a copy of astra/frameworks/qml/controls
# (see cmake/AstraPreviewModules.cmake).
file(GLOB CONTROLS_QML RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.qml")
file(GLOB CONTROLS_ICONS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/icons/*.svg")
set_source_files_properties(Theme.qml Catalog.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(astracontrols
    URI Astra.Controls
    VERSION 1.0
    PLUGIN_TARGET astracontrols
    OUTPUT_DIRECTORY "${STUDIO_QML_DIR}/Astra/Controls"
    IMPORTS Astra.Animation Astra.Core
    QML_FILES ${CONTROLS_QML}
    RESOURCES ${CONTROLS_ICONS}
)
target_link_libraries(astracontrols PRIVATE Qt6::Quick)
