add_library(KyronextUi STATIC src/ui.cpp include/kyronext/ui.h)
target_include_directories(KyronextUi PUBLIC include)
target_link_libraries(KyronextUi PUBLIC Qt6::Widgets)
install(FILES assets/kyronext-suite.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
