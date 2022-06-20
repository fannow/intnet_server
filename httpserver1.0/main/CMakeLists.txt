include_directories(${project_source_dir}/code)
AUX_SOURCE_DIRECTORY(. LIST_ARRAY)
add_Definitions(-std=c++11)
ADD_EXECUTABLE(main ${LIST_ARRAY})
#为demo链接共享库
TARGET_LINK_LIBRARIES(main log -lpthread)


