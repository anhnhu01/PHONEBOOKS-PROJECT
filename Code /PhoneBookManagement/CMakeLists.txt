cmake_minimum_required(VERSION 3.21)
project(PhoneBookManagement VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets Sql Network Test)

set(SOURCES
    src/main.cpp
    src/core/Models.h
    src/core/Database.h src/core/Database.cpp
    src/modules/auth/AuthService.h src/modules/auth/AuthService.cpp
    src/modules/auth/AuthPage.h src/modules/auth/AuthPage.cpp
    src/modules/auth/ProfileDialog.h src/modules/auth/ProfileDialog.cpp
    src/modules/contacts/ContactService.h src/modules/contacts/ContactService.cpp
    src/modules/contacts/ContactsPage.h src/modules/contacts/ContactsPage.cpp
    src/modules/tags/TagFavouriteService.h src/modules/tags/TagFavouriteService.cpp
    src/modules/tags/TagFavouritePage.h src/modules/tags/TagFavouritePage.cpp
    src/modules/importstats/ImportStatsService.h src/modules/importstats/ImportStatsService.cpp
    src/modules/importstats/ImportStatsPage.h src/modules/importstats/ImportStatsPage.cpp
    src/modules/admin/AdminService.h src/modules/admin/AdminService.cpp
    src/modules/admin/AdminPage.h src/modules/admin/AdminPage.cpp
    src/ui/MainWindow.h src/ui/MainWindow.cpp
)

qt_add_executable(PhoneBookManagement ${SOURCES})
target_link_libraries(PhoneBookManagement PRIVATE Qt6::Widgets Qt6::Sql Qt6::Network)
set_target_properties(PhoneBookManagement PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)

add_executable(test_auth
    tests/TestAuth.cpp
    src/modules/auth/AuthService.h
    src/modules/auth/AuthService.cpp
)
set_target_properties(test_auth PROPERTIES AUTOMOC ON WIN32_EXECUTABLE FALSE)
target_link_libraries(test_auth PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Test)
