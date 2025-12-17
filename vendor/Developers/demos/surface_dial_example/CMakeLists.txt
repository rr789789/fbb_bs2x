    set(SOURCES_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/encoder.c
        ${CMAKE_CURRENT_SOURCE_DIR}/encoder_setting.c
        ${CMAKE_CURRENT_SOURCE_DIR}/surface_dial_hid.c
    )

    set(PUBLIC_HEADER_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set(SOURCES "${SOURCES_LIST}" PARENT_SCOPE)
    set(PUBLIC_HEADER "${PUBLIC_HEADER_LIST}" PARENT_SCOPE)