	# /////////////////////////////////////////////////////////////////////////////////////////////
# // Copyright 2017 Intel Corporation
# //
# // Licensed under the Apache License, Version 2.0 (the "License");
# // you may not use this file except in compliance with the License.
# // You may obtain a copy of the License at
# //
# // http://www.apache.org/licenses/LICENSE-2.0
# //
# // Unless required by applicable law or agreed to in writing, software
# // distributed under the License is distributed on an "AS IS" BASIS,
# // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# // See the License for the specific language governing permissions and
# // limitations under the License.
# /////////////////////////////////////////////////////////////////////////////////////////////

cmake_minimum_required (VERSION 3.0)
project( "IntroductionToVulkan" )

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

if( CMAKE_BUILD_TYPE STREQUAL "" )
	set( CMAKE_BUILD_TYPE "debug" )
endif()

if( NOT USE_PLATFORM MATCHES "VK_USE_PLATFORM_.*" )
	if( WIN32 )
		set( USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" )
	elseif( UNIX )
		set( USE_PLATFORM "VK_USE_PLATFORM_XCB_KHR" )
	endif()
endif()

if( USE_PLATFORM STREQUAL "VK_USE_PLATFORM_WIN32_KHR" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO    "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE} /MT" )
	set( CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG} /MTd" )
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
	set( CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} /MTd" )
	set( PLATFORM_LIBRARY                "" )
	add_definitions( -DNOMINMAX )
elseif( USE_PLATFORM STREQUAL "VK_USE_PLATFORM_XCB_KHR" )
	add_definitions( -std=c++11 )
	set( PLATFORM_LIBRARY dl xcb )
elseif( USE_PLATFORM STREQUAL "VK_USE_PLATFORM_XLIB_KHR" )
	add_definitions( -std=c++11 )
	set( PLATFORM_LIBRARY dl X11 )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "debug" )
	add_definitions(-D_DEBUG)
endif()

macro( add_executable _type _number _name )
	set( TARGET_NAME "${_number}-${_name}" )
	file( GLOB PROJECT_FILES "${CMAKE_CURRENT_LIST_DIR}/${_type}/${_number}/*.*" )
	_add_executable( ${TARGET_NAME} ${ARGN} ${PROJECT_FILES} )
	target_compile_definitions( ${TARGET_NAME} PRIVATE USE_SWAPCHAIN_EXTENSIONS )
	target_link_libraries( ${TARGET_NAME} ${PLATFORM_LIBRARY} )
	set_property( TARGET ${TARGET_NAME} PROPERTY FOLDER "${_type}" )

	if( EXISTS "${CMAKE_SOURCE_DIR}/${_type}/${_number}/Data" )
		file( GLOB DATA_FILES "${CMAKE_SOURCE_DIR}/${_type}/${_number}/Data/*.*" )
                file( COPY ${DATA_FILES} DESTINATION "${CMAKE_BINARY_DIR}/Data/${_type}/${_number}/" )
	endif()
endmacro()

add_definitions( -DVK_NO_PROTOTYPES )
add_definitions( -D${USE_PLATFORM} )
include_directories( "External" )
include_directories( "Common" )

file( GLOB BASIC_SHARED_HEADER_FILES
		"Common/OperatingSystem.h"
		"Common/ListOfFunctions.inl"
		"Common/VulkanFunctions.h" )

file( GLOB BASIC_SHARED_SOURCE_FILES
		"Common/OperatingSystem.cpp"
		"Common/VulkanFunctions.cpp" )

file( GLOB EXTERNAL_HEADER_FILES
		"External/vk_platform.h"
		"External/vulkan.h"
		"External/stb_image.h" )

set( ALL_BASIC_SHARED_FILES
		${BASIC_SHARED_HEADER_FILES}
		${BASIC_SHARED_SOURCE_FILES}
		${EXTERNAL_HEADER_FILES} )

file( GLOB ADVANCED_SHARED_HEADER_FILES
		"Common/Tools.h"
		"Common/VulkanCommon.h" )

file( GLOB ADVANCED_SHARED_SOURCE_FILES
		"Common/Tools.cpp"
		"Common/VulkanCommon.cpp" )

set( ALL_BASIC_AND_ADVANCED_SHARED_FILES
		${ALL_BASIC_SHARED_FILES}
		${ADVANCED_SHARED_HEADER_FILES}
		${ADVANCED_SHARED_SOURCE_FILES} )

set( PROJECT_HEADER_FILES REGULAR_EXPRESSION
		"(Tutorial|Benchmark)[0-9]+[.]h" )

set( PROJECT_SOURCE_FILES REGULAR_EXPRESSION
		"(Tutorial|Benchmark)[0-9]+[.]cpp" )

source_group( "Header Files\\Common" FILES ${BASIC_SHARED_HEADER_FILES} ${ADVANCED_SHARED_HEADER_FILES} )
source_group( "Source Files\\Common" FILES ${BASIC_SHARED_SOURCE_FILES} ${ADVANCED_SHARED_SOURCE_FILES} )
source_group( "Header Files\\External" FILES ${EXTERNAL_HEADER_FILES} )
source_group( "Header Files" FILES ${PROJECT_HEADER_FILES} )
source_group( "Source Files" FILES ${PROJECT_SOURCE_FILES} )

_add_executable( "01-The_Beginning" ${ALL_BASIC_SHARED_FILES}
	Tutorials/01/Tutorial01.h
	Tutorials/01/main.cpp
	Tutorials/01/Tutorial01.cpp )
	target_link_libraries( "01-The_Beginning" ${PLATFORM_LIBRARY} )
	set_property( TARGET "01-The_Beginning" PROPERTY FOLDER "Tutorials" )

add_executable( "Tutorials" "02" "Swapchain" ${ALL_BASIC_SHARED_FILES} )

add_executable( "Tutorials" "03" "First_Triangle" ${ALL_BASIC_AND_ADVANCED_SHARED_FILES} )

add_executable( "Tutorials" "04" "Vertex_Attributes" ${ALL_BASIC_AND_ADVANCED_SHARED_FILES} )

add_executable( "Tutorials" "05" "Staging_Resources" ${ALL_BASIC_AND_ADVANCED_SHARED_FILES} )

add_executable( "Tutorials" "06" "Descriptor_Sets" ${ALL_BASIC_AND_ADVANCED_SHARED_FILES} )

add_executable( "Tutorials" "07" "Uniform_Buffers" ${ALL_BASIC_AND_ADVANCED_SHARED_FILES} )
