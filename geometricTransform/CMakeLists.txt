
SET(DGTAL_TOOLS_SRC
 rigidTransform2D
 rigidTransform3D
)


FOREACH(FILE ${DGTAL_TOOLS_SRC})
  add_executable(${FILE} ${FILE}.cpp)
  target_link_libraries (${FILE} ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies} )
  install(TARGETS ${FILE}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
ENDFOREACH(FILE)

