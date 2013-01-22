cmake_minimum_required(VERSION 2.6)
project(qtorm)

find_package(Qt4 REQUIRED)
include(GNUInstallDirs)

add_definitions(-fPIC)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_QTCORE_INCLUDE_DIR}
        ${QT_QTSQL_INCLUDE_DIR}
        ${QT_INCLUDE_DIR}
        ${QT_QT_INCLUDE_DIR}
)

# Sources
set(qtorm_SRCS
    qassign.cpp
    qdatetimefield.cpp
    qdoublefield.cpp
    qf.cpp
    qfield.cpp
    qforeignkey.cpp
    qintfield.cpp
    qmodel.cpp
    qqueryset.cpp
    qstringfield.cpp
    qwhere.cpp
    qtormdatabase.cpp
)

set(qtorm_HEADERS
    qassign.h
    qdatetimefield.h
    qdoublefield.h
    qf.h
    qfield.h
    qfield_p.h
    qforeignkey.h
    qforeignkey_p.h
    qintfield.h
    qmodel.h
    qqueryset.h
    qstringfield.h
    qwhere.h
    qtormdatabase.h
)

# Automoc
qt4_automoc(${qtorm_SRCS})

# Shared library
add_library(qtorm SHARED ${qtorm_SRCS})

set_target_properties(qtorm PROPERTIES
    VERSION 0.1.0
    SOVERSION 0.1.0
)
target_link_libraries(qtorm
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
)


install(TARGETS qtorm LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${qtorm_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtorm)
