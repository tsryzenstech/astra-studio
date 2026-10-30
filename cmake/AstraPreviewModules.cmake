# SPDX-License-Identifier: Apache-2.0
# The QML modules the preview loads apps with, so an app looks exactly as on
# the phone:
#   Astra.Animation  astraOS's springs (C++), built from ASTRA_SOURCE
#   Astra.Controls   astraOS's components (QML + icons), from ASTRA_SOURCE
#   Astra.Core       a preview stand-in for the system services (preview/)
#
# The astraOS sources are copied into the build tree and built there as
# subprojects (cmake/preview/<module>.cmake as their CMakeLists.txt): Qt names
# generated files after paths relative to the source directory, and paths
# leading outside it ("../astra/...") cannot be created on Windows.

function(astra_preview_module name source_dir)
    set(staged "${CMAKE_BINARY_DIR}/preview-src/${name}")
    file(REMOVE_RECURSE "${staged}")
    file(COPY "${source_dir}/" DESTINATION "${staged}" PATTERN "CMakeLists.txt" EXCLUDE)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/preview/${name}.cmake" "${staged}/CMakeLists.txt" COPYONLY)
    # Edits in the astraOS checkout copy again on the next build.
    file(GLOB_RECURSE inputs "${source_dir}/*")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${inputs})
    add_subdirectory("${staged}" "${CMAKE_BINARY_DIR}/preview-build/${name}")
endfunction()

astra_preview_module(animation "${ASTRA_SOURCE}/frameworks/animation")
astra_preview_module(controls "${ASTRA_SOURCE}/frameworks/qml/controls")

# ---- Astra.Core (preview) --------------------------------------------------------
file(GLOB CORE_QML RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/preview/Astra/Core/*.qml")
set_source_files_properties(${CORE_QML} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
foreach(f IN LISTS CORE_QML)
    get_filename_component(n "${f}" NAME)
    set_source_files_properties("${f}" PROPERTIES QT_RESOURCE_ALIAS "${n}")
endforeach()
qt_add_qml_module(astracore_preview
    URI Astra.Core
    VERSION 1.0
    PLUGIN_TARGET astracore_preview
    OUTPUT_DIRECTORY "${STUDIO_QML_DIR}/Astra/Core"
    QML_FILES ${CORE_QML}
)

add_custom_target(astra_preview_modules DEPENDS astraanimation astracore_preview astracontrols)
