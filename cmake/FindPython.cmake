find_package(PythonInterp REQUIRED)
find_package(PythonLibs   REQUIRED)

set(PYTHON_INCLUDE_PATH ${PYTHON_INCLUDE_DIRS} CACHE PATH "")
set(PYTHON_LIBRARY      ${PYTHON_LIBRARIES}    CACHE PATH "")

execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_config_var('Py_ENABLE_SHARED'))"
				OUTPUT_VARIABLE PYTHON_LIB_SHARED
				OUTPUT_STRIP_TRAILING_WHITESPACE
				)

cmake_print_variables(PYTHON_LIB_SHARED)
cmake_print_variables(CMAKE_CXX_COMPILER_ID)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Python
		REQUIRED_VARS PYTHON_EXECUTABLE PYTHON_LIBRARIES
		)

# The python interpreter can be linked against shared libpython.so or static libpython.a.
# When the interpreter (python executable) is linked statically against libpython,
# python extenison modules shouldn't link against libpython.so to avoid segfaults.
# But, then the linker will complain about missing symbols. So, the linker needs to
# be told not to look for undefined symbols. Clang's flag for that is
# "-undefined dynamic_lookup -flat_namespace", gcc's flag is "-undefined".
#
# Ref:
# https://github.com/conda-forge/boost-feedstock/issues/70#issuecomment-486398688
# https://github.com/conda-forge/conda-forge.github.io/issues/778

if(Python_FOUND AND NOT TARGET Python::Python)
	add_library(Python::Python INTERFACE IMPORTED)
	set_target_properties(Python::Python
			PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_DIRS}
#			Py_ENABLE_SHARED is None on Windows, so the compiler is checked if it is Microsoft Visual Studio.
#			Link against shared python library, if the compiler is MSVC 
#			or if Py_ENABLE_SHARED is 1 (Python interpreter is not linked statically against libpython).
			INTERFACE_LINK_LIBRARIES      $<$<OR:$<CXX_COMPILER_ID:MSVC>,$<BOOL:${PYTHON_LIB_SHARED}>>:${PYTHON_LIBRARIES}>
			)
	target_link_options(Python::Python INTERFACE
						#			If the compiler is Clang and if Py_ENABLE_SHARED is 0 (Python interpreter is linked statically against libpython),
						#			use link options "-undefined dynamic_lookup -flat_namespace" to tell the linker not to look for undefined symbols.
						#			They will be found at runtime.
						"$<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<NOT:$<BOOL:${PYTHON_LIB_SHARED}>>>:-undefined;dynamic_lookup;-flat_namespace>"
						#			If the compiler is GCC and if Py_ENABLE_SHARED is 0 (Python interpreter is linked statically against libpython),
						#			use link options "-undefined" to tell the linker not to look for undefined symbols.
						#			They will be found at runtime.
						"$<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<BOOL:${PYTHON_LIB_SHARED}>>>:-undefined>"
						)
endif()
