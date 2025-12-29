cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
else()
    add_executable(WindNinja
        ${PROJECT_SOURCES}
    )
endif()

if(WIN32)
  target_link_libraries(WindNinja PRIVATE shapelib::shp)
endif()

target_link_libraries(WindNinja PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets PRIVATE ninja)
target_link_libraries(WindNinja PRIVATE Qt6::Core)

target_link_libraries(WindNinja
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    PRIVATE Qt${QT_VERSION_MAJOR}::WebChannel
    PRIVATE ninja
)

include(GNUInstallDirs)
install(TARGETS WindNinja
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WindNinja)
endif()
