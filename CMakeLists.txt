# SPDX-License-Identifier: Apache-2.0
# Astra Studio - the IDE for astraOS apps.
cmake_minimum_required(VERSION 3.21)
project(AstraStudio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
if(MSVC)
    # Sources are UTF-8, and the astraOS sources use M_PI from <cmath>.
    add_compile_options(/utf-8)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

# A checkout of https://github.com/tsryzenstech/astra: the preview builds
# astraOS's own components (Astra.Controls, Astra.Animation) from it, and the
# SDK (astra command, templates, design check) ships next to the program.
set(ASTRA_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../astra" CACHE PATH "astraOS source tree")
# A relative -DASTRA_SOURCE is relative to this directory; sources and globs
# below need it absolute.
file(REAL_PATH "${ASTRA_SOURCE}" ASTRA_SOURCE BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
if(NOT EXISTS "${ASTRA_SOURCE}/frameworks/qml/controls/Theme.qml")
    message(FATAL_ERROR "ASTRA_SOURCE (${ASTRA_SOURCE}) is not an astraOS checkout")
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickWidgets Svg)
qt_standard_project_setup(REQUIRES 6.5)

# Every QML module lands in <build>/qml, next to the program at install time.
set(STUDIO_QML_DIR "${CMAKE_BINARY_DIR}/qml")
include(cmake/AstraPreviewModules.cmake)

qt_add_executable(AstraStudio WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/codeeditor.cpp src/codeeditor.h
    src/highlighter.cpp src/highlighter.h
    src/newprojectdialog.cpp src/newprojectdialog.h
    src/previewpane.cpp src/previewpane.h
    src/sdk.cpp src/sdk.h
    src/welcome.cpp src/welcome.h
    resources/studio.qrc
)
if(WIN32)
    enable_language(RC)
    target_sources(AstraStudio PRIVATE resources/studio.rc)
endif()
target_compile_definitions(AstraStudio PRIVATE STUDIO_VERSION="${PROJECT_VERSION}")
target_link_libraries(AstraStudio PRIVATE Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickWidgets Qt6::Svg)
add_dependencies(AstraStudio astra_preview_modules)

# Install layout (what the Windows zip contains):
#   AstraStudio.exe, Qt (windeployqt), qml/Astra/*, sdk/, fonts/
install(TARGETS AstraStudio RUNTIME DESTINATION . BUNDLE DESTINATION .)
install(DIRECTORY "${STUDIO_QML_DIR}/" DESTINATION qml)
install(DIRECTORY "${ASTRA_SOURCE}/sdk/" DESTINATION sdk PATTERN "__pycache__" EXCLUDE)
install(FILES "${ASTRA_SOURCE}/tools/apkgtool/apkgtool.py" "${ASTRA_SOURCE}/tools/adu/adu.py"
              "${ASTRA_SOURCE}/tools/i18n/i18n.py" DESTINATION sdk/tools)
install(DIRECTORY "${ASTRA_SOURCE}/docs/" DESTINATION docs)
install(DIRECTORY "${ASTRA_SOURCE}/prebuilt/fonts/" DESTINATION fonts FILES_MATCHING PATTERN "Poppins-*.ttf" PATTERN "OFL.txt")
