add_executable(kyronext-settings main.cpp)
target_link_libraries(kyronext-settings PRIVATE KyronextUi Qt6::Widgets)
install(TARGETS kyronext-settings RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES kyronext-settings.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
