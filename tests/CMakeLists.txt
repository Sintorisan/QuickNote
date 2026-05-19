add_executable(quicknotestoretest
    quicknotestoretest.cpp
    ../plugin/quicknotestore.cpp
)

target_include_directories(quicknotestoretest PRIVATE ../plugin)

target_link_libraries(quicknotestoretest
    PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
)

target_compile_definitions(quicknotestoretest PRIVATE
    QUICKNOTE_QML_IMPORT_PATH="${CMAKE_BINARY_DIR}/package/contents/ui"
)

add_test(NAME quicknotestoretest COMMAND quicknotestoretest)
