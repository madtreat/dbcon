project(dbcon)

find_package(Qt5Core    REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql     REQUIRED)
find_package(Qt5Network REQUIRED)
# find_package(MySQL      REQUIRED)

set(SOURCES
  dbcon.cpp
  dbconfwindow.cpp
  dbsettings.cpp
)

set(HEADERS
  dbconsts.h
  dbcon.h
  dbconfwindow.h
  dbsettings.h
)


add_library(${PROJECT_NAME} SHARED
  ${SOURCES}
  ${MOC_SRC}
)

message(STATUS "Qt5::Sql libraries:" Qt5::Sql)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Widgets
  Qt5::Sql
  Qt5::Network
  # ${MYSQL_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)

