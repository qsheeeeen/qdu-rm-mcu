cmake_minimum_required(VERSION 3.11)
project(bsp)

file(GLOB ${PROJECT_NAME}_BOARD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD_DIR}/*.c"
)

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE ${${PROJECT_NAME}_BOARD_SOURCES}
    PRIVATE ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE hal
    PRIVATE component
    PRIVATE rtos
    PRIVATE usb
)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE $<TARGET_PROPERTY:hal,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:component,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:rtos,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:usb,INTERFACE_INCLUDE_DIRECTORIES>
)

add_dependencies(${PROJECT_NAME}
    hal
    component
    rtos
    usb
)
