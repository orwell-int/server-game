# Tasks' CMakeLists

#
# This submodule contains all the stuff necessary to play with tasks
#

# Get together all the source files and all the headers
file(GLOB SOURCES src/*.cpp src/*.cc)
file(GLOB EXPORTABLEHEADERS include/*.hpp include/*.h)

# Required libraries
find_package(Boost REQUIRED COMPONENTS system)
find_package(Protobuf REQUIRED)
find_package(ZMQ REQUIRED)

find_package(LOG4CXX_LIBRARIES REQUIRED)

# Helpful debug stuff?
message("-- Protobuf binary found at ${PROTOBUF_PROTOC_EXECUTABLE}")

# Generate the protobuf messages
#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ./protobuf_messages/version1.proto)

# The -I flag for the compiler..
message("-- include ${PROTOBUF_INCLUDE_DIRS}")
message("-- include ${CMAKE_CURRENT_BINARY_DIR}")
message("-- include ${LOG4CXX_INCLUDE_DIR}")
message("-- include ${ZMQ_INCLUDE_DIRS}")
message("-- include ${CMAKE_BINARY_DIR}/com/")
message("-- include ${CMAKE_SOURCE_DIR}/com/include")
message("-- include ${Boost_INCLUDE_DIRS}")
include_directories(
		"include"
		"${CMAKE_SOURCE_DIR}/com/include"
		${CMAKE_BINARY_DIR}/messages
		${CMAKE_CURRENT_BINARY_DIR} # This is needed for protobuf's messages
		)

# Library compilation and linking
add_library(orwell_tasks SHARED ${SOURCES})
target_link_libraries(orwell_tasks
		orwell_com
		orwell_messages
		${ZMQ_LIBRARIES}
		${Boost_LIBRARIES}
		${LOG4CXX_LIBRARIES})

# Where to install stuff, protobuf messages generated H included
install(TARGETS orwell_tasks LIBRARY DESTINATION lib)
install(FILES ${EXPORTABLEHEADERS} DESTINATION include)

