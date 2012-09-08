project(qtorm)

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
)

# Automoc
qt4_automoc(${qtorm_SRCS})

# Bibliothèque statique
add_library(qtorm_static STATIC ${qtorm_SRCS})

# Installation
install(TARGETS qtorm_static ARCHIVE DESTINATION /usr/lib)
install(FILES ${qtorm_HEADERS} DESTINATION /usr/include/qtorm)
