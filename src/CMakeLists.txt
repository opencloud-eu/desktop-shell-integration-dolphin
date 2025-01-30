#---HELPER---
add_library(openclouddolphinpluginhelper SHARED ownclouddolphinpluginhelper.cpp ownclouddolphinpluginhelper.h)
generate_export_header(openclouddolphinpluginhelper BASE_NAME openclouddolphinpluginhelper)
target_link_libraries(openclouddolphinpluginhelper Qt6::Core Qt6::Network Qt6::Gui)
target_compile_definitions(openclouddolphinpluginhelper PRIVATE QT_USE_QSTRINGBUILDER)


#---OVERLAY PLUGIN---
kcoreaddons_add_plugin(openclouddolphinoverlayplugin INSTALL_NAMESPACE "kf6/overlayicon"
                       SOURCES ownclouddolphinoverlayplugin.cpp)
target_link_libraries(openclouddolphinoverlayplugin KF6::CoreAddons KF6::KIOCore KF6::KIOWidgets openclouddolphinpluginhelper)

#---ACTION PLUGIN---
kcoreaddons_add_plugin(openclouddolphinactionplugin SOURCES ownclouddolphinactionplugin.cpp INSTALL_NAMESPACE "kf6/kfileitemaction")
target_link_libraries(openclouddolphinactionplugin KF6::CoreAddons KF6::KIOCore KF6::KIOWidgets openclouddolphinpluginhelper)

install(TARGETS openclouddolphinpluginhelper LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
