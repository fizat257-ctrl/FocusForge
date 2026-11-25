cmake_minimum_required(VERSION 3.16)

project(FocusForge VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(FocusForge
    main.cpp
    mainwindow.cpp
    mainwindow.h
    task.h
    taskstore.cpp
    taskstore.h
    subject.h
    subjectstore.cpp
    subjectstore.h
    subjectsmanagerpage.cpp
    subjectsmanagerpage.h
)

target_link_libraries(FocusForge PRIVATE Qt6::Widgets)

install(TARGETS FocusForge
    RUNTIME DESTINATION bin
)

set(CPACK_PACKAGE_NAME "FocusForge")
set(CPACK_PACKAGE_VENDOR "FocusForge Studio")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "FocusForge")
set(CPACK_GENERATOR "WIX")
set(CPACK_WIX_VERSION 4)

include(CPack)