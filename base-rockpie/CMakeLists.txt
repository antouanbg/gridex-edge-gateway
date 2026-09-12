cmake_minimum_required(VERSION 3.20)
project(gridex_rockpie_base VERSION 0.1.0 LANGUAGES CXX)

# The core project registers its own CTest target.  Do not exclude it from the
# default build here, otherwise CTest sees gridex_edge_tests but cannot run it.
add_subdirectory(.. ${CMAKE_BINARY_DIR}/gridex-core)

add_library(gridex_rockpie_transport
    src/MqttHealthPublisher.cpp
    src/NodeTcpPollingService.cpp
    src/NorthboundModbusTcpServer.cpp
    src/NorthboundRegisterBank.cpp
    src/PosixModbusTcpClient.cpp
)
target_include_directories(gridex_rockpie_transport PUBLIC include)
target_link_libraries(gridex_rockpie_transport PUBLIC gridex_edge)
target_compile_features(gridex_rockpie_transport PUBLIC cxx_std_20)

option(GRIDEX_ENABLE_PRIVATE_MQTT "Build private MQTT telemetry publisher" ON)
if(GRIDEX_ENABLE_PRIVATE_MQTT)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(MOSQUITTO QUIET IMPORTED_TARGET libmosquitto)
        # Homebrew's mosquitto header includes cJSON directly.  Debian's
        # libmosquitto-dev has the dependency transitively; declaring it here
        # also keeps a local TLS build reproducible.
        pkg_check_modules(CJSON QUIET IMPORTED_TARGET libcjson)
    endif()
    if(TARGET PkgConfig::MOSQUITTO)
        target_link_libraries(gridex_rockpie_transport PRIVATE PkgConfig::MOSQUITTO)
        if(TARGET PkgConfig::CJSON)
            target_link_libraries(gridex_rockpie_transport PRIVATE PkgConfig::CJSON)
        endif()
        target_compile_definitions(gridex_rockpie_transport PRIVATE GRIDEX_WITH_MOSQUITTO=1)
    else()
        message(STATUS "libmosquitto not found: private MQTT publisher is compiled as disabled")
    endif()
endif()

if(MSVC)
    target_compile_options(gridex_rockpie_transport PRIVATE /W4 /permissive-)
else()
    target_compile_options(gridex_rockpie_transport PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(gridex_rockpie_service src/main.cpp)
target_link_libraries(gridex_rockpie_service PRIVATE gridex_rockpie_transport)

# The OTA utility is deliberately a local, operator-invoked client. It has no
# listener and cannot expose the ESP32 directly to a VPN or Internet caller.
add_executable(gridex_ota_apply src/OtaApply.cpp)
target_compile_features(gridex_ota_apply PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(gridex_ota_apply PRIVATE /W4 /permissive-)
else()
    target_compile_options(gridex_ota_apply PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_executable(gridex_rockpie_tests tests/test_northbound.cpp)
target_link_libraries(gridex_rockpie_tests PRIVATE gridex_rockpie_transport)
add_test(NAME gridex_rockpie_tests COMMAND gridex_rockpie_tests)

find_package(Threads REQUIRED)
add_executable(gridex_input_range_tests tests/test_input_range.cpp)
target_link_libraries(gridex_input_range_tests PRIVATE gridex_rockpie_transport Threads::Threads)
add_test(NAME gridex_input_range_tests COMMAND gridex_input_range_tests)
set_tests_properties(gridex_input_range_tests PROPERTIES TIMEOUT 15)

add_executable(gridex_mqtt_health_tests tests/test_mqtt_health.cpp)
target_link_libraries(gridex_mqtt_health_tests PRIVATE gridex_rockpie_transport)
add_test(NAME gridex_mqtt_health_tests COMMAND gridex_mqtt_health_tests)

add_executable(gridex_node_tcp_polling_tests tests/test_node_tcp_polling.cpp)
target_link_libraries(gridex_node_tcp_polling_tests PRIVATE gridex_rockpie_transport Threads::Threads)
add_test(NAME gridex_node_tcp_polling_tests COMMAND gridex_node_tcp_polling_tests)
set_tests_properties(gridex_node_tcp_polling_tests PROPERTIES TIMEOUT 15)

add_test(NAME gridex_ota_apply_self_test COMMAND gridex_ota_apply --self-test)

install(TARGETS gridex_rockpie_service gridex_ota_apply RUNTIME DESTINATION bin)
install(FILES systemd/gridex-rockpie.service DESTINATION lib/systemd/system)
install(FILES config/gridex-rockpie.env.example DESTINATION share/gridex)
