set(SOURCES testTiler.cpp)
add_executable(testTiler ${SOURCES})
target_link_libraries(testTiler ${Libraries})

set(SOURCES profileTiler.cxx)
add_executable(profileTiler ${SOURCES})
target_link_libraries(profileTiler ${Libraries})

set(cexx mainTiler)
add_executable(${cexx} ${cexx}.cxx)
target_link_libraries(${cexx} ${Libraries})

