# generate_demo needs a local (non-redistributable) sample library; skip the
# tool entirely when the directory is not present so clean checkouts build
set(RUMPU_DEMO_SAMPLES_DIR "$ENV{HOME}/tmp/rumpu_samples/Real Drums Vol. 1"
	CACHE PATH "Sample library directory used by the generate_demo tool")

if(EXISTS "${RUMPU_DEMO_SAMPLES_DIR}")
	add_executable(generate_demo generate_demo.cpp)
	target_compile_definitions(generate_demo PRIVATE SAMPLES_DIR="${RUMPU_DEMO_SAMPLES_DIR}")
	target_link_libraries(generate_demo rumpu_core)
else()
	message(STATUS "generate_demo skipped: RUMPU_DEMO_SAMPLES_DIR not found (${RUMPU_DEMO_SAMPLES_DIR})")
endif()
