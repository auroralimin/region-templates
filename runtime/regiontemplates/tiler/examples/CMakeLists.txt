set(SOURCES testTiler.cpp)
add_executable(testTiler ${SOURCES})
target_link_libraries(testTiler ${Libraries})

set(cexx mainTiler)
add_executable(${cexx} ${cexx}.cxx)
target_link_libraries(${cexx} ${Libraries})

