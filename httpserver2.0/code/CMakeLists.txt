#AUX_SOURCE_DIRECTORY(. LIST_ARRAY)
add_definitions(-std=c++11)
ADD_LIBRARY(log  SHARED Log.cpp)
ADD_LIBRARY(smtp  SHARED smtp.cpp)

TARGET_LINK_LIBRARIES(log smtp)
