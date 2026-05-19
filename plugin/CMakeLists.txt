set(quicknoteplugin_SRCS
    quicknoteplugin.cpp
    quicknotestore.cpp
)

add_library(quicknoteplugin SHARED ${quicknoteplugin_SRCS})
set(QUICKNOTE_PRIVATE_QML_SUBDIR "org/sintori/quicknote/private")
set(QUICKNOTE_PRIVATE_QML_BUILD_DIR "${CMAKE_BINARY_DIR}/package/contents/ui/${QUICKNOTE_PRIVATE_QML_SUBDIR}")
set(QUICKNOTE_PRIVATE_QML_INSTALL_DIR "${KDE_INSTALL_DATADIR}/plasma/plasmoids/org.sintori.quicknote/contents/ui/${QUICKNOTE_PRIVATE_QML_SUBDIR}")

set_target_properties(quicknoteplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${QUICKNOTE_PRIVATE_QML_BUILD_DIR}"
    INSTALL_RPATH ""
)

target_link_libraries(quicknoteplugin
    PRIVATE
        Qt6::Core
        Qt6::Qml
)

configure_file(qmldir "${QUICKNOTE_PRIVATE_QML_BUILD_DIR}/qmldir" COPYONLY)

install(TARGETS quicknoteplugin DESTINATION "${QUICKNOTE_PRIVATE_QML_INSTALL_DIR}")
install(FILES qmldir DESTINATION "${QUICKNOTE_PRIVATE_QML_INSTALL_DIR}")
