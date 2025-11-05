# CMakeLists.txt snippet (updated)
add_library(MonJeuLib
    src/VoxelWorld.cpp
    src/Player.cpp
    src/GameDebugOverlay.cpp
)

set_target_properties(MonJeuLib PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

target_include_directories(MonJeuLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_SOURCE_DIR}/NihilEngine/include
)

target_link_libraries(MonJeuLib PUBLIC NihilEngine)

add_executable(MonJeu
    src/main.cpp
)

set_target_properties(MonJeu PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

target_include_directories(MonJeu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/NihilEngine/include
)

target_link_libraries(MonJeu PRIVATE MonJeuLib NihilEngine)