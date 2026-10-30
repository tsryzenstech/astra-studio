# SPDX-License-Identifier: Apache-2.0
# The QML modules the preview loads apps with, so an app looks exactly as on
# the phone:
#   Astra.Animation  astraOS's springs (C++), built from ASTRA_SOURCE
#   Astra.Controls   astraOS's components (QML + icons), from ASTRA_SOURCE
#   Astra.Core       a preview stand-in for the system services (preview/)

# Files outside this tree keep their name inside the module.
function(astra_alias_files out_var base)
    set(files)
    foreach(f IN LISTS ARGN)
        file(RELATIVE_PATH rel "${base}" "${f}")
        set_source_files_properties("${f}" PROPERTIES QT_RESOURCE_ALIAS "${rel}")
        list(APPEND files "${f}")
    endforeach()
    set(${out_var} ${files} PARENT_SCOPE)
endfunction()

# ---- Astra.Animation ----------------------------------------------------------
set(ANIM_DIR "${ASTRA_SOURCE}/frameworks/animation")
astra_alias_files(ANIM_QML "${ANIM_DIR}" "${ANIM_DIR}/SpringNumberAnimation.qml")
qt_add_qml_module(astraanimation
    URI Astra.Animation
    VERSION 1.0
    PLUGIN_TARGET astraanimation
    OUTPUT_DIRECTORY "${STUDIO_QML_DIR}/Astra/Animation"
    SOURCES
        "${ANIM_DIR}/motion.cpp" "${ANIM_DIR}/motion.h"
        "${ANIM_DIR}/spring.cpp" "${ANIM_DIR}/spring.h"
        "${ANIM_DIR}/springvalue.cpp" "${ANIM_DIR}/springvalue.h"
        "${ANIM_DIR}/velocitytracker.cpp" "${ANIM_DIR}/velocitytracker.h"
    QML_FILES ${ANIM_QML}
)
target_include_directories(astraanimation PRIVATE "${ANIM_DIR}")
target_link_libraries(astraanimation PRIVATE Qt6::Core Qt6::Qml Qt6::Quick)

# ---- Astra.Core (preview) --------------------------------------------------------
file(GLOB CORE_QML "${CMAKE_CURRENT_SOURCE_DIR}/preview/Astra/Core/*.qml")
set_source_files_properties(${CORE_QML} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
astra_alias_files(CORE_QML_ALIASED "${CMAKE_CURRENT_SOURCE_DIR}/preview/Astra/Core" ${CORE_QML})
qt_add_qml_module(astracore_preview
    URI Astra.Core
    VERSION 1.0
    PLUGIN_TARGET astracore_preview
    OUTPUT_DIRECTORY "${STUDIO_QML_DIR}/Astra/Core"
    QML_FILES ${CORE_QML_ALIASED}
)

# ---- Astra.Controls -------------------------------------------------------------
set(CONTROLS_DIR "${ASTRA_SOURCE}/frameworks/qml/controls")
file(GLOB CONTROLS_QML "${CONTROLS_DIR}/*.qml")
file(GLOB CONTROLS_ICONS "${CONTROLS_DIR}/icons/*.svg")
set_source_files_properties("${CONTROLS_DIR}/Theme.qml" "${CONTROLS_DIR}/Catalog.qml"
                            PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
astra_alias_files(CONTROLS_QML_ALIASED "${CONTROLS_DIR}" ${CONTROLS_QML})
astra_alias_files(CONTROLS_ICONS_ALIASED "${CONTROLS_DIR}" ${CONTROLS_ICONS})
qt_add_qml_module(astracontrols
    URI Astra.Controls
    VERSION 1.0
    PLUGIN_TARGET astracontrols
    OUTPUT_DIRECTORY "${STUDIO_QML_DIR}/Astra/Controls"
    IMPORTS Astra.Animation Astra.Core
    QML_FILES ${CONTROLS_QML_ALIASED}
    RESOURCES ${CONTROLS_ICONS_ALIASED}
)
target_link_libraries(astracontrols PRIVATE Qt6::Quick)

add_custom_target(astra_preview_modules DEPENDS astraanimation astracore_preview astracontrols)
