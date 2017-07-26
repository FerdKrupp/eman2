FUNCTION(message_var var)
	message("${var}: ${${var}}")
endfunction()

function(set_cache_var var val)
	if(val AND NOT ${var})
		set(${var} ${val} CACHE PATH "" FORCE)
	else()
		set(${var} ${var}-NOTFOUND CACHE PATH "")
	endif()
endfunction()

function(set_cache_var_to_var var val)
	if(${val} AND NOT ${var})
		set(${var} ${${val}} CACHE PATH "" FORCE)
	else()
		set(${var} ${var}-NOTFOUND CACHE PATH "")
	endif()
endfunction()