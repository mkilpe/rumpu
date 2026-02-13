file(GLOB BACKEND_SRC *.cpp *.hpp)
target_sources(audio PRIVATE ${BACKEND_SRC})
target_link_libraries(audio Dsound dxguid)
