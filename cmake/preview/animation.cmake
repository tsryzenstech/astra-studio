# SPDX-License-Identifier: Apache-2.0
# Astra.Animation, built from a copy of astra/frameworks/animation
# (see cmake/AstraPreviewModules.cmake).
qt_add_qml_module(astraanimation
    URI Astra.Animation
    VERSION 1.0
    PLUGIN_TARGET astraanimation
    OUTPUT_DIRECTORY "${STUDIO_QML_DIR}/Astra/Animation"
    SOURCES
        motion.cpp motion.h
        spring.cpp spring.h
        springvalue.cpp springvalue.h
        velocitytracker.cpp velocitytracker.h
    QML_FILES
        SpringNumberAnimation.qml
)
target_link_libraries(astraanimation PRIVATE Qt6::Core Qt6::Qml Qt6::Quick)
