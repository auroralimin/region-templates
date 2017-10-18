set(programs mainTiler profileTiler testTiler)

foreach(program ${programs})
		add_executable(${program} ${program}.cxx)
		# Link the executable to the Region Templates library
        target_link_libraries(${program} ${Libraries})
endforeach(program)        

