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

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

configure_file(
    resources/focusforge.rc.in
    "${CMAKE_CURRENT_BINARY_DIR}/focusforge.rc"
    @ONLY)

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
    quiz.h
    quizattempt.h
    quizstore.cpp
    quizstore.h
    quizattemptstore.cpp
    quizattemptstore.h
    quiztextractor.cpp
    quiztextractor.h
    windowscredentialmanager.cpp
    windowscredentialmanager.h
    apikeyeditordialog.cpp
    apikeyeditordialog.h
    quizgenerator.cpp
    quizgenerator.h
    quizgeneratorresult.h
    studyreportpage.cpp
    studyreportpage.h
    quizgeneratorpage.cpp
    quizgeneratorpage.h
    stopwatchpage.cpp
    stopwatchpage.h
)

target_link_libraries(FocusForge PRIVATE
    Qt6::Widgets
    Qt6::Network
)