if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
endif()

# Setup test
add_executable(the_test main.cpp)
target_compile_features(the_test INTERFACE cxx_std_11)

# Enable coverage testing
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
if(CMAKE_COMPILER_IS_GNUCXX)
  include(CodeCoverage)
  setup_target_for_coverage(setup_coverage the_test coverage)
endif()  

# Enable testing
include(CTest)
enable_testing()
add_test(test_all the_test)


