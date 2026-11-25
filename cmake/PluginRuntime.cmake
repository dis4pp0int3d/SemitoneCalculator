# Stage precisely the SoundTouch sources used for this build, including a standalone rebuild recipe.
function(add_soundtouch_runtime runtime headers sources licence)
    set(stage "${CMAKE_CURRENT_BINARY_DIR}/${runtime}-source")
    file(MAKE_DIRECTORY "${stage}")
    file(GLOB files CONFIGURE_DEPENDS "${headers}/*.h" "${sources}/*.cpp" "${sources}/*.h")
    foreach(src IN LISTS files)
        get_filename_component(name "${src}" NAME)
        configure_file("${src}" "${stage}/${name}" COPYONLY)
    endforeach()
    if(NOT EXISTS "${headers}/soundtouch_config.h")
        file(WRITE "${stage}/soundtouch_config.h" "// Default float samples.\n")
    endif()
    configure_file("${licence}" "${stage}/COPYING.TXT" COPYONLY)
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/SoundTouchLibrary.cmake.in" "${stage}/CMakeLists.txt" @ONLY)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar cfvz "${stage}.tar.gz" --format=gnutar .
        WORKING_DIRECTORY "${stage}" OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY)
    add_subdirectory("${stage}" "${runtime}-build")
endfunction()

function(package_plugin target format runtime)
    set(wrapper "${target}_${format}")
    set(bin "$<TARGET_FILE_DIR:${wrapper}>")
    if(APPLE)
        set(libdir "$<TARGET_BUNDLE_CONTENT_DIR:${wrapper}>/Frameworks")
        set(notices "$<TARGET_BUNDLE_CONTENT_DIR:${wrapper}>/Resources/Licenses")
        set_target_properties(${wrapper} PROPERTIES BUILD_WITH_INSTALL_RPATH ON INSTALL_RPATH "@loader_path/../Frameworks")
    elseif(format STREQUAL "VST3")
        set(libdir "${bin}")
        set(notices "${bin}/../Resources/Licenses")
    else()
        set(libdir "${bin}")
        set(notices "${bin}/Licenses")
    endif()
    if(runtime)
        if(MSVC)
            target_sources(${wrapper} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/LoadRuntime.cpp")
            target_compile_definitions(${wrapper} PRIVATE PLUGIN_RUNTIME_NAME="${runtime}.dll")
            target_link_options(${wrapper} PRIVATE "/DELAYLOAD:${runtime}.dll")
            target_link_libraries(${wrapper} PRIVATE delayimp)
        endif()
        add_custom_command(TARGET ${wrapper} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${libdir}" "${notices}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${runtime}>" "${libdir}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${runtime}-source.tar.gz" "${CMAKE_CURRENT_BINARY_DIR}/${runtime}-source/COPYING.TXT" "${notices}" VERBATIM)
    endif()
    # Include the JUCE notices for the SDKs and codecs distributed with these plug-ins.
    set(notice_files ${ARGN})
    set(modules "${JUCE_PATH}/modules")
    set(dependency_licences
        "JUCE|${JUCE_PATH}/LICENSE.md"
        "FLAC|${modules}/juce_audio_formats/codecs/flac/Flac Licence.txt"
        "OggVorbis|${modules}/juce_audio_formats/codecs/oggvorbis/Ogg Vorbis Licence.txt"
        "JPEG|${modules}/juce_graphics/image_formats/jpglib/README"
        "PNG|${modules}/juce_graphics/image_formats/pnglib/LICENSE"
        "zlib|${modules}/juce_core/zip/zlib/README"
        "HarfBuzz|${modules}/juce_graphics/fonts/harfbuzz/COPYING"
        "SheenBidi|${modules}/juce_graphics/unicode/sheenbidi/LICENSE")
    if(format STREQUAL "AU")
        list(APPEND dependency_licences "AudioUnitSDK|${modules}/juce_audio_plugin_client/AU/AudioUnitSDK/LICENSE.txt")
    elseif(format STREQUAL "VST3")
        list(APPEND dependency_licences "VST3SDK|${modules}/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt")
    endif()
    foreach(entry IN LISTS dependency_licences)
        string(REPLACE "|" ";" parts "${entry}")
        list(GET parts 0 name)
        list(GET parts 1 file)
        if(EXISTS "${file}")
            set(dest "${CMAKE_CURRENT_BINARY_DIR}/notices/${name}.txt")
            configure_file("${file}" "${dest}" COPYONLY)
            list(APPEND notice_files "${dest}")
        endif()
    endforeach()
    if(notice_files)
        add_custom_command(TARGET ${wrapper} POST_BUILD COMMAND "${CMAKE_COMMAND}" -E make_directory "${notices}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${notice_files} "${notices}" VERBATIM)
    endif()
    if(APPLE)
        if(runtime)
            add_custom_command(TARGET ${wrapper} POST_BUILD
                COMMAND /usr/bin/codesign --force --sign - "${libdir}/$<TARGET_FILE_NAME:${runtime}>" VERBATIM)
        endif()
        # Seal the finished bundle after its resources and nested library have been copied.
        add_custom_command(TARGET ${wrapper} POST_BUILD
            COMMAND /usr/bin/codesign --force --sign - "$<TARGET_BUNDLE_DIR:${wrapper}>" VERBATIM)
    endif()
endfunction()
