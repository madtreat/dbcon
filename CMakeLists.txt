project(dbcon)

find_package(Qt5Core    REQUIRED)
find_package(Qt5Sql     REQUIRED)
find_package(Qt5Network REQUIRED)

set(SOURCES
  dbcon.cpp
  dbsettings.cpp
)

set(HEADERS
  dbconsts.h
  dbcon.h
  dbsettings.h
)


add_library(${PROJECT_NAME} SHARED
  ${SOURCES}
  ${MOC_SRC}
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Sql
  Qt5::Network
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)

