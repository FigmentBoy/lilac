cmake_minimum_required(VERSION 3.8)

project(lilac_loader LANGUAGES CXX VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES 
	src/*.cpp
)

add_compile_definitions(LILAC_EXPORTING)

add_library(lilac_loader SHARED ${SOURCES})

set_target_properties(lilac_loader PROPERTIES PREFIX "" OUTPUT_NAME "lilac")

add_subdirectory(submodules/lib)

target_link_libraries(
	lilac_loader
	lilac_core_hook
	lilac_lib
)

if(DEBUG)
	target_link_libraries(lilac_loader staticZipperd)
else()
	target_link_libraries(lilac_loader staticZipper)
endif()

target_include_directories(
	lilac_loader PUBLIC
	"${CMAKE_SOURCE_DIR}/api"
	"${CMAKE_SOURCE_DIR}/api/lilac"
	"${CMAKE_SOURCE_DIR}/api/utils"
	"${CMAKE_SOURCE_DIR}/api/keybinds"
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/src/lilac"
	"${CMAKE_SOURCE_DIR}/src/lilac/internal"
	"${CMAKE_SOURCE_DIR}/submodules/zipper/zipper"
	"${CMAKE_SOURCE_DIR}/submodules/json"
)

add_compile_definitions(_EXPORTING)
add_compile_definitions(LILAC_PLATFORM_CONSOLE)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

function(create_lilac_file proname)
	add_custom_command(
		TARGET ${proname} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/mod.json"
			"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/mod.json"
			"${srcs}"
	)
	add_custom_command(
		TARGET ${proname}
		COMMAND ${CMAKE_COMMAND} -E tar "cf" 
			"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${proname}.lilac" --format=zip -- 
			"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/mod.json"
			"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${proname}.dll"
			"${srcs}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
		COMMAND ${CMAKE_COMMAND} -E echo "Creating zip file -> ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${proname}.lilac"
	)
endfunction()

add_subdirectory(test)
