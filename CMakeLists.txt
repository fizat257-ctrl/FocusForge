cmake_minimum_required(VERSION 3.16)

if(DEFINED CMAKE_CXX_COMPILER)
    get_filename_component(FOCUSFORGE_MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    if(NOT CMAKE_RC_COMPILER STREQUAL "${FOCUSFORGE_MINGW_BIN}/windres.exe"
       AND EXISTS "${FOCUSFORGE_MINGW_BIN}/windres.exe")
        set(CMAKE_RC_COMPILER "${FOCUSFORGE_MINGW_BIN}/windres.exe"
            CACHE FILEPATH "Resource compiler matching the active MinGW toolchain" FORCE)
    endif()
endif()

project(FocusForge VERSION 1.0.0)

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

qt_generate_deploy_app_script(
    TARGET FocusForge
    OUTPUT_SCRIPT FOCUSFORGE_DEPLOY_SCRIPT
    NO_TRANSLATIONS
)

install(TARGETS FocusForge
    RUNTIME DESTINATION bin
)
install(SCRIPT "${FOCUSFORGE_DEPLOY_SCRIPT}")

set(CPACK_PACKAGE_NAME "FocusForge")
set(CPACK_PACKAGE_VENDOR "FocusForge Studio")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "FocusForge")
set(CPACK_GENERATOR "WIX")
set(CPACK_WIX_VERSION 4)
set(CPACK_WIX_UPGRADE_GUID "A9DB347E-37E0-429A-9F4D-8D03ACB90512")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/focusforge.ico")
set(CPACK_WIX_INSTALL_SCOPE "perMachine")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "FocusForge")
set(CPACK_PACKAGE_EXECUTABLES "FocusForge;FocusForge")
set(CPACK_CREATE_DESKTOP_LINKS "FocusForge")

include(CPack)