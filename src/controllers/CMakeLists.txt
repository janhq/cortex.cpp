add_library_unity(controllers OBJECT 
health.cc llamaCPP.cc processManager.cc)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:controllers>
    PARENT_SCOPE)

