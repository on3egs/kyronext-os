cmake_minimum_required(VERSION 3.21)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(KyronextBackup VERSION 0.1.0 LANGUAGES CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_AUTOMOC ON)
  include(GNUInstallDirs)
  include(CTest)
  find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
  add_subdirectory(../ui "${CMAKE_CURRENT_BINARY_DIR}/ui-framework")
endif()

add_library(KyronextBackupBackend STATIC archive_backend.cpp archive_backend.h)
target_link_libraries(KyronextBackupBackend PUBLIC Qt6::Core)
target_include_directories(KyronextBackupBackend PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(kyronext-backup main.cpp backup_window.cpp backup_window.h)
target_link_libraries(kyronext-backup PRIVATE KyronextUi KyronextBackupBackend Qt6::Widgets)
install(TARGETS kyronext-backup RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources/kyronext-backup.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/kyronext-backup.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

if(BUILD_TESTING)
  find_package(Qt6 6.4 REQUIRED COMPONENTS Test)
  add_executable(test-kyronext-backup tests/backup_test.cpp)
  target_link_libraries(test-kyronext-backup PRIVATE KyronextBackupBackend Qt6::Test)
  add_test(NAME kyronext-backup-archive-safety COMMAND test-kyronext-backup)
  # Qt marks each case QSKIP when run as root. Keep CTest from reporting that
  # invocation as a successful functional test.
  set_tests_properties(kyronext-backup-archive-safety PROPERTIES
    TIMEOUT 60
    SKIP_REGULAR_EXPRESSION "SKIP")
endif()
