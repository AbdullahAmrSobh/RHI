# Define the include directory and library file for the prebuilt library
set(PREBUILT_LIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PREBUILT_LIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}//lib")
set(PREBUILT_LIB_NAME "slang")  # The name of the library (without extension)

# Create an IMPORTED target for the prebuilt library
add_library(${PREBUILT_LIB_NAME} STATIC IMPORTED)

# Specify the include directories for the prebuilt library
set_target_properties(${PREBUILT_LIB_NAME} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PREBUILT_LIB_INCLUDE_DIR}"
)

# Specify the location of the prebuilt .lib file
set_target_properties(${PREBUILT_LIB_NAME} PROPERTIES
    IMPORTED_LOCATION "${PREBUILT_LIB_LIB_DIR}/${PREBUILT_LIB_NAME}.lib"
)