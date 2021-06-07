# Find GeographicLib helpful: "sudo ln -s /usr/share/cmake/geographiclib/FindGeographicLib.cmake /usr/share/cmake-3.16/Modules/"
find_package(GeographicLib REQUIRED)
include_directories(${GeographicLib_INCLUDE_DIR})

add_executable(navigation_client navigation_client.cpp)
add_dependencies(navigation_client ${catkin_EXPORTED_TARGETS})
target_link_libraries(navigation_client ${catkin_LIBRARIES} ${GeographicLib_LIBRARIES})

install(
        TARGETS navigation_client
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
