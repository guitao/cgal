project(CGAL_ipelets_Demo)

cmake_minimum_required(VERSION 2.8.11)
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD)
endif()

macro( remove_leading_zero var )
  string(SUBSTRING "${${var}}" 0 1 ONECHAR)
  string(COMPARE EQUAL "${ONECHAR}" "0" ISZERO)
  if (${ISZERO})
    string(SUBSTRING "${${var}}" 1 1 ONECHAR)
    set(${var} ${ONECHAR})
  endif()
endmacro()



#path where to build libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

find_package(CGAL QUIET COMPONENTS Core)

if ( CGAL_FOUND )
  include( ${CGAL_USE_FILE} )

  find_package(IPE 6)

  if ( IPE_FOUND )
    include_directories(BEFORE ${IPE_INCLUDE_DIR})

    if (${IPE_VERSION} EQUAL "7")
      set(WITH_IPE_7 ON)
    elseif(${IPE_VERSION} EQUAL "6")
      set(WITH_IPE_7 OFF)
    else()
      message("-- Error: ${IPE_VERSION} is not a supported version of IPE (only 6 and 7 are).")
      set(IPE_FOUND FALSE)
    endif()
    # starting ipe 7.2.1, a compiler with c++11 must be used to compile ipelets
    if (${IPE_VERSION} EQUAL "7" AND
        ${IPE_MINOR_VERSION_1} GREATER "1" AND
        ${IPE_MINOR_VERSION_2} GREATER "0")
      message(STATUS "Starting from Ipe 7.2.1 a compiler with c++11 support must be used")
      include(CheckCXXCompilerFlag)
      CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
      CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
      if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
      elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
      endif()
    endif()
  endif()

  if ( IPE_FOUND AND IPE_VERSION)
    if (WITH_IPE_7)
      add_definitions(-DCGAL_USE_IPE_7)
    endif()
    
    message("-- Using IPE version ${IPE_VERSION} compatibility.") 
    
    #setting installation directory
    get_filename_component(IPE_LIBRARY_DIR ${IPE_LIBRARIES} PATH)
    if (IPE_FOUND AND NOT IPELET_INSTALL_DIR)
      if (WITH_IPE_7)
        remove_leading_zero(IPE_MINOR_VERSION_1)
        remove_leading_zero(IPE_MINOR_VERSION_2)
        set(INSTALL_PATHS "${IPE_LIBRARY_DIR}/ipe/7.${IPE_MINOR_VERSION_1}.${IPE_MINOR_VERSION_2}/ipelets/")
        find_path(IPELET_INSTALL_DIR 
                      NAMES libgoodies.lua goodies.lua
                      PATHS ${INSTALL_PATHS}
                      DOC "The folder where ipelets will be installed"
                      ENV IPELETPATH
                     )
      else()
        foreach (VER RANGE 28 40)
        string(REPLACE XX ${VER} PATHC "${IPE_LIBRARY_DIR}/ipe/6.0preXX/ipelets/" )
        set(INSTALL_PATHS ${INSTALL_PATHS} ${PATHC})
        endforeach()
        set(INSTALL_PATHS ${INSTALL_PATHS} ${PATHC})
        set(INSTALL_PATHS ${INSTALL_PATHS} /usr/lib64/ipe/6.0/ipelets)
        set(INSTALL_PATHS ${INSTALL_PATHS} /usr/lib/ipe/6.0/ipelets)


        find_library(IPELET_INSTALL_DIR_FILES 
                      NAMES align
                      PATHS ${INSTALL_PATHS}
                            ENV IPELETPATH
                    )
        if (IPELET_INSTALL_DIR_FILES)
          get_filename_component(IPELET_INSTALL_DIR ${IPELET_INSTALL_DIR_FILES} PATH)
        endif()                
      endif()
    endif()

    set(CGAL_IPELETS ${CGAL_IPELETS})
    set(CGAL_IPELETS ${CGAL_IPELETS} alpha_shapes)
    set(CGAL_IPELETS ${CGAL_IPELETS} arrangement)
    set(CGAL_IPELETS ${CGAL_IPELETS} bbox_restriction)
    set(CGAL_IPELETS ${CGAL_IPELETS} diagrams)
    set(CGAL_IPELETS ${CGAL_IPELETS} hilbert_sort)
    set(CGAL_IPELETS ${CGAL_IPELETS} hull)
    set(CGAL_IPELETS ${CGAL_IPELETS} generator)
    set(CGAL_IPELETS ${CGAL_IPELETS} mesh_2)
    set(CGAL_IPELETS ${CGAL_IPELETS} minkowski)
    set(CGAL_IPELETS ${CGAL_IPELETS} multi_delaunay)
    set(CGAL_IPELETS ${CGAL_IPELETS} multi_regular)
    set(CGAL_IPELETS ${CGAL_IPELETS} partition)
    set(CGAL_IPELETS ${CGAL_IPELETS} pca)    
    set(CGAL_IPELETS ${CGAL_IPELETS} skeleton)
    set(CGAL_IPELETS ${CGAL_IPELETS} svdlinf)
    set(CGAL_IPELETS ${CGAL_IPELETS} triangulation)
    set(CGAL_IPELETS ${CGAL_IPELETS} circle_pencils)
    set(CGAL_IPELETS ${CGAL_IPELETS} hyperbolic)
    set(CGAL_IPELETS ${CGAL_IPELETS} distance)
    set(CGAL_IPELETS ${CGAL_IPELETS} cone_spanners)


    if ( IPELET_INSTALL_DIR )
      message(STATUS "Set Ipelets install dir: ${IPELET_INSTALL_DIR}")
    endif()

    foreach(IPELET ${CGAL_IPELETS})
      add_library(CGAL_${IPELET} MODULE ${IPELET}.cpp)
      add_to_cached_list(CGAL_EXECUTABLE_TARGETS CGAL_${IPELET})
      target_link_libraries(CGAL_${IPELET} ${IPE_LIBRARIES})
      if ( IPELET_INSTALL_DIR )
        install(TARGETS CGAL_${IPELET} DESTINATION ${IPELET_INSTALL_DIR})
        if (WITH_IPE_7)
          install(FILES ./lua/libCGAL_${IPELET}.lua DESTINATION ${IPELET_INSTALL_DIR}) #only for ipe 7
        endif()
      endif ()
    endforeach(IPELET)
    #example in doc not installed
    add_library(simple_triangulation MODULE simple_triangulation.cpp)
    add_to_cached_list(CGAL_EXECUTABLE_TARGETS simple_triangulation)
    target_link_libraries(simple_triangulation ${IPE_LIBRARIES})

  else()
    message(STATUS "NOTICE: This program requires the Ipe include files and library, and will not be compiled.")
  endif()
else()
    message(STATUS "NOTICE: This program requires the CGAL library, and will not be compiled.")
endif()
