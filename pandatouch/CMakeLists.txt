file(GLOB_RECURSE SRCS src/*.c)

if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.3")
    set(REQ esp_driver_i2c esp_driver_gpio)
    set(PRIV_REQ esp_driver_ledc)
else()
    set(REQ driver)
    set(PRIV_REQ "")
endif()

idf_component_register(
    SRCS            ${SRCS}
    INCLUDE_DIRS    "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES        ${REQ}
    PRIV_REQUIRES   ${PRIV_REQ} esp_lcd esp_psram
)
