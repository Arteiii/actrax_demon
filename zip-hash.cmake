set(DEFAULT_ZIP_PASSWORD "ISeeDeadPeople")

set(ZIP_PASSWORD "${DEFAULT_ZIP_PASSWORD}")
if (DEFINED ENV{ZIP_PASSWORD})
    set(ZIP_PASSWORD "$ENV{ZIP_PASSWORD}")
elseif (CMAKE_ARGUMENTS MATCHES "-DZIP_PASSWORD=(.*)")
    string(REGEX MATCH "-DZIP_PASSWORD=(.*)" ZIP_PASSWORD "${CMAKE_ARGUMENTS}")
    string(REGEX REPLACE "-DZIP_PASSWORD=" "" ZIP_PASSWORD "${ZIP_PASSWORD}")
endif ()

set(EXECUTABLE_NAME "${PROJECT_NAME}")
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(EXECUTABLE_NAME "${PROJECT_NAME}.exe")
endif ()

message(STATUS "Running release-specific tasks")

# TODO!: Fix hash output wrong (prob using old file)

execute_process(
        COMMAND sha256sum ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}
        RESULT_VARIABLE SHA256_RESULT
        OUTPUT_VARIABLE SHA256_OUTPUT
        ERROR_VARIABLE SHA256_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}" "" SHA256_HASH "${SHA256_OUTPUT}")

execute_process(
        COMMAND sha1sum ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}
        RESULT_VARIABLE SHA1_RESULT
        OUTPUT_VARIABLE SHA1_OUTPUT
        ERROR_VARIABLE SHA1_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}" "" SHA1_HASH "${SHA1_OUTPUT}")

execute_process(
        COMMAND md5sum ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}
        RESULT_VARIABLE MD5_RESULT
        OUTPUT_VARIABLE MD5_OUTPUT
        ERROR_VARIABLE MD5_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME}" "" MD5_HASH "${MD5_OUTPUT}")

set(HASH_MESSAGE
        "Hashes for ${PROJECT_NAME}:\n   - SHA256: ${SHA256_HASH}\n   - SHA1: ${SHA1_HASH}\n   - MD5: ${MD5_HASH}"
)

message(STATUS "${HASH_MESSAGE}")

file(WRITE ${FullOutputDir}/${PROJECT_NAME}_hashes.txt "${HASH_MESSAGE}")

file(WRITE ${FullOutputDir}/password.md
        "# Zip Password Information\n\n"
        "The zip password is: **${ZIP_PASSWORD}**\n\n"
        "### To unzip the package:\n"
        "Use the following command to unzip the archive with the password:\n"
        "```sh\n"
        "unzip -P ${ZIP_PASSWORD} ./${PROJECT_NAME}_${PROJECT_VERSION}.zip\n"
        "```\n\n"
        "For Windows users, download and install [7-Zip](https://www.7-zip.org/download.html)\n"
        "After installing 7-Zip, use the following command to unzip the archive:\n"
        "```sh\n"
        "7z x ./${PROJECT_NAME}_${PROJECT_VERSION}.zip -p${ZIP_PASSWORD}\n"
        "```\n"
)

add_custom_target(create_zip ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${FullOutputDir}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE_NAME} ${FullOutputDir}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LICENSE-GPL ${FullOutputDir}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FullOutputDir}/${PROJECT_NAME}_hashes.txt ${FullOutputDir}
        COMMAND zip -P ${ZIP_PASSWORD} ${FullOutputDir}/${PROJECT_NAME}_${PROJECT_VERSION}.zip ${FullOutputDir}/${EXECUTABLE_NAME} ${FullOutputDir}/LICENSE-GPL ${FullOutputDir}/${PROJECT_NAME}_hashes.txt
        DEPENDS ${PROJECT_NAME} ${FullOutputDir}/${PROJECT_NAME}_hashes.txt
        COMMENT "Creating zip file with executable, LICENSE, and hash file (password not included)"
)

add_custom_command(TARGET create_zip POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove ${FullOutputDir}/LICENSE-GPL
        COMMENT "Removing LICENSE and password file after adding them to zip"
)
