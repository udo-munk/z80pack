add_library(pico-ds3231 INTERFACE)

target_include_directories(pico-ds3231
  INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico-ds3231
  INTERFACE
  hardware_i2c
)

target_sources(pico-ds3231
  INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/ds3231.c
  ${CMAKE_CURRENT_LIST_DIR}/ds3231.h
)
