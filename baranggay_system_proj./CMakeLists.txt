cmake_minimum_required(VERSION 3.10)

project(BarangaySystem)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
find_package(SQLite3 REQUIRED)

add_executable(barangay
    main.cpp
    database.cpp
    document.cpp
    report.cpp
    gui.cpp
)

target_include_directories(barangay PRIVATE ${GTK3_INCLUDE_DIRS})
target_compile_options(barangay PRIVATE ${GTK3_CFLAGS_OTHER})
target_link_libraries(barangay PRIVATE sqlite3 ${GTK3_LIBRARIES})
