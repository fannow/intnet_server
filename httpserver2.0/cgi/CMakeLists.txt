AUX_SOURCE_DIRECTORY(. LIST_ARRAY)
include_directories(${project_source_dir}/code)
add_definitions(-lmysqlclient)
aDD_EXECUTABLE(cgi ${LIST_ARRAY})
#为demo链接共享库
TARGET_LINK_LIBRARIES(cgi log -lmysqlclient)


