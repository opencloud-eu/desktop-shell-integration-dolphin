#---HELPER---
add_library(openclouddolphinpluginhelper SHARED dolphinpluginhelper.cpp dolphinpluginhelper.h)
generate_export_header(openclouddolphinpluginhelper BASE_NAME openclouddolphinpluginhelper)
target_link_libraries(openclouddolphinpluginhelper Qt6::Core Qt6::Network Qt6::Gui)
target_compile_definitions(openclouddolphinpluginhelper PRIVATE QT_USE_QSTRINGBUILDER)


#---OVERLAY PLUGIN---
kcoreaddons_add_plugin(openclouddolphinoverlayplugin INSTALL_NAMESPACE "kf6/overlayicon"
                       SOURCES dolphinoverlayplugin.cpp)
target_link_libraries(openclouddolphinoverlayplugin KF6::CoreAddons KF6::KIOCore KF6::KIOWidgets openclouddolphinpluginhelper)

#---ACTION PLUGIN---
kcoreaddons_add_plugin(openclouddolphinactionplugin SOURCES dolphinactionplugin.cpp INSTALL_NAMESPACE "kf6/kfileitemaction")
target_link_libraries(openclouddolphinactionplugin KF6::CoreAddons KF6::KIOCore KF6::KIOWidgets openclouddolphinpluginhelper)

install(TARGETS openclouddolphinpluginhelper LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
