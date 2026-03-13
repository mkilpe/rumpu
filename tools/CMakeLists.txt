add_executable(generate_demo generate_demo.cpp)
target_compile_definitions(generate_demo PRIVATE SAMPLES_DIR="$ENV{HOME}/tmp/rumpu_samples/Real Drums Vol. 1")
target_link_libraries(generate_demo rumpu_core)
