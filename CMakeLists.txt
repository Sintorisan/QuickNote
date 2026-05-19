cmake_minimum_required(VERSION 3.20)

project(kde-quick-note VERSION 0.1.0)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Qml Test)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Package I18n)
find_package(Plasma ${KF_MIN_VERSION} REQUIRED)

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.sintori.quicknote\")

enable_testing()

plasma_install_package(package org.sintori.quicknote)

add_subdirectory(plugin)
add_subdirectory(tests)
