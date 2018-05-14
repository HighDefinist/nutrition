cmake_minimum_required (VERSION 3.1)

set (SOURCE_LIST
  default
)

foreach (subproject ${SOURCE_LIST})
  add_executable(${subproject} ${subproject}/${subproject}.cpp)
  target_compile_features(${subproject} INTERFACE cxx_std_11)
endforeach()
