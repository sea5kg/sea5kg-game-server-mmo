cmake_minimum_required(VERSION 3.18)

project(game-server-mmo C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${game-server-mmo_SOURCE_DIR})

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")
list (APPEND WSJCPP_INCLUDE_DIRS "src/db")
list (APPEND WSJCPP_INCLUDE_DIRS "src/game")

list (APPEND WSJCPP_SOURCES "src/main.cpp")

#### BEGIN_WSJCPP_APPEND
#### END_WSJCPP_APPEND

# third_party
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/third_party/smallsha1/smallsha1.cpp")

# database
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/db/database_map.cpp")

# objects
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/gs_mmo/objects/gs_mmo_error_info.cpp")
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/gs_mmo/objects/gs_mmo_game_net_events.cpp")
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/gs_mmo/objects/gs_mmo_player_context.cpp")

# employees
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/gs_mmo/employees/employ_config.cpp")
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/gs_mmo/employees/employ_players_db.cpp")
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/gs_mmo/employees/employ_web_server.cpp")

# game
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/game/game_map_object.cpp")
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/game/game_map_objects.cpp")
list (APPEND WSJCPP_SOURCES "${CMAKE_SOURCE_DIR}/src/game/game_map_generator.cpp")

# libhv
set(WITH_OPENSSL OFF CACHE BOOL "")
set(WITH_CURL OFF CACHE BOOL "")
# set(WITH_MQTT OFF CACHE BOOL "")
# set(WITH_EVPP OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")
# set(WITH_HTTP_CLIENT OFF CACHE BOOL "")
set(BUILD_SHARED OFF CACHE BOOL "")

add_subdirectory(src/third_party/libhv)
list (APPEND WSJCPP_LIBRARIES hv_static)
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/src/third_party/libhv/include/hv")

add_subdirectory(src/third_party/sqlite3)
# https://www.sqlite.org/threadsafe.html
# target_compile_definitions(sqlite3-static
#   PRIVATE
#     SQLITE_THREADSAFE=0
# )
list (APPEND WSJCPP_LIBRARIES sqlite3-static)
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/src/third_party/sqlite3/src")


include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (game-server-mmo ${WSJCPP_SOURCES})

target_link_libraries(game-server-mmo ${WSJCPP_LIBRARIES})

install(
    TARGETS
        game-server-mmo
    RUNTIME DESTINATION
        /usr/bin
)


