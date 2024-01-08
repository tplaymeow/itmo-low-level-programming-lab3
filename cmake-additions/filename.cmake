function(define_file_info targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        get_filename_component(basename "${sourcefile}" NAME)
        set_property(
                SOURCE "${sourcefile}" APPEND
                PROPERTY COMPILE_DEFINITIONS "FILENAME=\"${basename}\"")
        set_property(
                SOURCE "${sourcefile}" APPEND
                PROPERTY COMPILE_DEFINITIONS "TARGETNAME=\"${targetname}\"")
    endforeach()
endfunction()