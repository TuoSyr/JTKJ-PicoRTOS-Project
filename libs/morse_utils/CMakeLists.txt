##Made using tkjhat library CMakeLists as an example

set(APP_NAME morse_utils)


# Generate a library 
add_library(${APP_NAME} STATIC
  src/morse_utils.c
)

target_include_directories(${APP_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src               
)