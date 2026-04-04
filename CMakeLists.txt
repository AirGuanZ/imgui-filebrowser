
cmake_minimum_required(VERSION 3.23)

project(imfilebrowser
  HOMEPAGE_URL https://github.com/AirGuanZ/imgui-filebrowser
  LANGUAGES CXX)

include(GNUInstallDirs)

add_library(imfilebrowser INTERFACE)
target_sources(imfilebrowser INTERFACE FILE_SET HEADERS FILES imfilebrowser.h)
target_compile_features(imfilebrowser INTERFACE cxx_std_17)

install(EXPORT imfilebrowser_export DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imfilebrowser
  NAMESPACE imfilebrowser::
  FILE imfilebrowser-config.cmake)
install(TARGETS imfilebrowser EXPORT imfilebrowser_export FILE_SET HEADERS)
