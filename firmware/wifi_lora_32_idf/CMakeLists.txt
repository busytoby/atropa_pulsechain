cmake_minimum_required(VERSION 3.16)

# Exclude unused third-party components that have compiler incompatibilities
set(EXCLUDE_COMPONENTS "lmic-esp-idf")

# Include ESP-IDF project configuration
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wifi_lora_32_idf)
