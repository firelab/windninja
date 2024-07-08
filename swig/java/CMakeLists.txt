# Find SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Find Java and JNI
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# Include directories
include_directories(
    ${JNI_INCLUDE_DIRS}
    ${NINJA_INCLUDES}
    ${PROJECT_SOURCE_DIR}/src/ninja
    ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
)

# SWIG configuration
set(SWIG_INPUT_FILE ${PROJECT_SOURCE_DIR}/swig/java/windninja.i)
set(SWIG_OUTFILE_DIR ${PROJECT_SOURCE_DIR}/swig/java)
set(SWIG_CXX_EXTENSION "cpp")

# Set SWIG properties
set_property(SOURCE ${SWIG_INPUT_FILE} PROPERTY USE_SWIG_DEPENDENCIES TRUE)
set_property(SOURCE ${SWIG_INPUT_FILE} PROPERTY SWIG_MODULE_NAME WindNinjaJava)
set_property(SOURCE ${SWIG_INPUT_FILE} PROPERTY CPLUSPLUS ON)
set_property(SOURCE ${SWIG_INPUT_FILE} PROPERTY USE_TARGET_INCLUDE_DIRECTORIES TRUE)

# Add SWIG library target for Java
swig_add_library(WindNinjaJava
    LANGUAGE java
    OUTPUT_DIR ${SWIG_OUTFILE_DIR}
    SOURCES ${SWIG_INPUT_FILE}
)
target_link_libraries(WindNinjaJava ninja)
install (TARGETS WindNinjaJava DESTINATION ${PROJECT_SOURCE_DIR}/swig/java)
