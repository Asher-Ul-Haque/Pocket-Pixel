cmake_minimum_required(VERSION 3.16)

add_library(rcheevos STATIC)

target_sources(rcheevos PRIVATE
        # Core runtime
        src/rcheevos/alloc.c
        src/rcheevos/condition.c
        src/rcheevos/condset.c
        src/rcheevos/consoleinfo.c
        src/rcheevos/format.c
        src/rcheevos/lboard.c
        src/rcheevos/memref.c
        src/rcheevos/operand.c
        src/rcheevos/richpresence.c
        src/rcheevos/runtime.c
        src/rcheevos/runtime_progress.c
        src/rcheevos/trigger.c
        src/rcheevos/value.c
        src/rcheevos/rc_validate.c

        # Client
        src/rc_client.c
        src/rc_client_external.c
        src/rc_compat.c
        src/rc_util.c
        src/rc_version.c

        # API
        src/rapi/rc_api_common.c
        src/rapi/rc_api_editor.c
        src/rapi/rc_api_info.c
        src/rapi/rc_api_runtime.c
        src/rapi/rc_api_user.c

        # Hashing
        src/rhash/aes.c
        src/rhash/cdreader.c
        src/rhash/hash.c
        src/rhash/hash_disc.c
        src/rhash/hash_encrypted.c
        src/rhash/hash_rom.c
        src/rhash/hash_zip.c
        src/rhash/md5.c
)

target_include_directories(rcheevos
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rcheevos
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rapi
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rhash
)

target_compile_features(rcheevos PUBLIC c_std_99)
target_compile_definitions(rcheevos PUBLIC RC_CLIENT_SUPPORTS_HASH=1)
