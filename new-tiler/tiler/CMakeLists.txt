option(build_testTiler "build testTiler?" ON)
if (build_testTiler)
    set(SOURCES testTiler.cpp Tiler.h Tiler.cpp)
    add_executable(testTiler ${SOURCES})
    target_link_libraries(testTiler ${Libraries})
endif (build_testTiler)

option(build_mainTiler "build mainTiler?" ON)
if (build_mainTiler)
    set(cexx mainTiler)
    add_executable(${cexx} ${cexx}.cxx)
    target_link_libraries(${cexx} ${Libraries})
endif (build_mainTiler)


