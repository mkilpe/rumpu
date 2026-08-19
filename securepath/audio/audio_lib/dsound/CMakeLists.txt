file(GLOB BACKEND_SRC CONFIGURE_DEPENDS dsound/*.cpp dsound/*.hpp)
target_sources(audio PRIVATE ${BACKEND_SRC})
target_link_libraries(audio dsound dxguid)
