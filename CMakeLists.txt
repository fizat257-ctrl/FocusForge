cmake_minimum_required(VERSION 3.16)

project(FocusForge VERSION 1.0.0)

if(MINGW)
    get_filename_component(FOCUSFORGE_MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    get_filename_component(FOCUSFORGE_RC_BIN "${CMAKE_RC_COMPILER}" DIRECTORY)
    if(NOT FOCUSFORGE_RC_BIN STREQUAL FOCUSFORGE_MINGW_BIN
       AND EXISTS "${FOCUSFORGE_MINGW_BIN}/windres.exe")
        set(CMAKE_RC_COMPILER "${FOCUSFORGE_MINGW_BIN}/windres.exe"
            CACHE FILEPATH "Resource compiler matching the active MinGW toolchain" FORCE)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

configure_file(
    resources/focusforge.rc.in
    "${CMAKE_CURRENT_BINARY_DIR}/focusforge.rc"
    @ONLY
)

qt_add_executable(FocusForge
    "${CMAKE_CURRENT_BINARY_DIR}/focusforge.rc"
    resources/focusforge.qrc
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
    studysession.h
    studysessionstore.cpp
    studysessionstore.h
    studyreportpage.cpp
    studyreportpage.h
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