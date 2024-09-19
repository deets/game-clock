idf_component_register(
  SRCS
  main.cpp
  INCLUDE_DIRS "."
  REQUIRES esp_driver_spi esp_driver_gpio
)
