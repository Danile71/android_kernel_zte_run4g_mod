# Because compiler-rt spends a lot of time setting up custom compile flags,
# define a handy helper function for it. The compile flags setting in CMake
# has serious issues that make its syntax challenging at best.
function(set_target_compile_flags target)
  foreach(arg ${ARGN})
    set(argstring "${argstring} ${arg}")
  endforeach()
  set_property(TARGET ${target} PROPERTY COMPILE_FLAGS "${argstring}")
endfunction()

function(set_target_link_flags target)
  foreach(arg ${ARGN})
    set(argstring "${argstring} ${arg}")
  endforeach()
  set_property(TARGET ${target} PROPERTY LINK_FLAGS "${argstring}")
endfunction()

# Check if a given flag is present in a space-separated flag_string.
# Store the result in out_var.
function(find_flag_in_string flag_string flag out_var)
  string(REPLACE " " ";" flag_list ${flag_string})
  list(FIND flag_list ${flag} flag_pos)
  if(NOT flag_pos EQUAL -1)
    set(${out_var} TRUE PARENT_SCOPE)
  else()
    set(${out_var} FALSE PARENT_SCOPE)
  endif()
endfunction()
