file(GLOB SRC *.cpp *.hpp spdrum.qrc)

qt5_add_resources(QT_RESOURCES spdrum.qrc)
add_application(spdrum ${SRC} ${QT_RESOURCES})
add_deps(spdrum spdrum_common common_qt)

enable_qt(spdrum)
enable_qt_debug()