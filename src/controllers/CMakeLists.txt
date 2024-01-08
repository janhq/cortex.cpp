# add_library_unity(controllers OBJECT 
# health.cc llamaCPP.cc processManager.cc)

# set(ALL_OBJECT_FILES
#     ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:controllers>
#     PARENT_SCOPE)
function(convert_to_absolute_paths relative_paths_var output_var)
    # Retrieve the list of relative paths
    set(relative_paths "${${relative_paths_var}}")

    # Initialize an empty list for the absolute paths
    set(absolute_paths "")

    # Loop through each relative path, convert it to absolute, and append to the list
    foreach(relative_path IN LISTS relative_paths)
        get_filename_component(absolute_path "${relative_path}" ABSOLUTE)
        list(APPEND absolute_paths "${absolute_path}")
    endforeach()

    # Set the output variable in the parent scope
    set(${output_var} "${absolute_paths}" PARENT_SCOPE)
endfunction()



set(obj_paths health.cc llamaCPP.cc processManager.cc)
convert_to_absolute_paths(obj_paths obj_abs_path)




set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} 
    "${obj_abs_path}"
    PARENT_SCOPE)