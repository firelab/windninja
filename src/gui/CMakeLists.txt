cmake_minimum_required(VERSION 3.16)

project(WindNinja VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_SOURCES
        main.cpp
        mainWindow.cpp
        mainWindow.h
        mainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WindNinja
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${CMAKE_SOURCE_DIR}/wn-resources.qrc
        appState.h
        appState.cpp
        splashScreen.h
        splashScreen.cpp
        surfaceInput.h
        surfaceInput.cpp
        setConfigurationDialogOption.h
        setConfigurationDialogOption.cpp
        setConfigurationDialogOption.ui
        menuBar.h
        menuBar.cpp
        mapBridge.h
        mapBridge.cpp
        surfaceInput.h
        surfaceInput.cpp
        serverBridge.h
        serverBridge.cpp
        domainAverageInput.h
        domainAverageInput.cpp
        pointInitializationInput.h
        pointInitializationInput.cpp
        weatherModelInput.h
        weatherModelInput.cpp
        outputs.h
        outputs.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WindNinja APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WindNinja SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WindNinja
            ${PROJECT_SOURCES}
        )
    endif()
endif()


target_link_libraries(WindNinja PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets PRIVATE ninja)
target_link_libraries(WindNinja PRIVATE Qt6::Core)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WindNinja)
endif()
set_target_properties(WindNinja PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WindNinja
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WindNinja)
endif()
