cmake_minimum_required(VERSION 3.25)
project(ShowClient VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(ShowClient
    ./ShowClient/main.cpp
    
    ./ShowClient/PRUConfig.cpp
    ./ShowClient/PRUConfig.hpp
    ./ShowClient/Configuration.cpp
    ./ShowClient/Configuration.hpp
    ./ShowClient/MusicController.cpp
    ./ShowClient/MusicController.hpp
    ./ShowClient/LightController.cpp
    ./ShowClient/LightController.hpp
    ./ShowClient/StatusController.cpp
    ./ShowClient/StatusController.hpp

    ./ShowClient/wavfile/chunkheader.cpp
    ./ShowClient/wavfile/chunkheader.hpp
    ./ShowClient/wavfile/fileheader.cpp
    ./ShowClient/wavfile/fileheader.hpp
    ./ShowClient/wavfile/mwavfile.cpp
    ./ShowClient/wavfile/mwavfile.hpp
    ./ShowClient/wavfile/wavfmtchunk.cpp
    ./ShowClient/wavfile/wavfmtchunk.hpp

    ./ShowClient/lightfile/lightfile.cpp
    ./ShowClient/lightfile/lightfile.hpp
    ./ShowClient/lightfile/lightheader.cpp
    ./ShowClient/lightfile/lightheader.hpp

    ./common/utility/buffer.cpp
    ./common/utility/buffer.hpp
    ./common/utility/dbgutil.hpp
    ./common/utility/mapfile.cpp
    ./common/utility/mapfile.hpp
    ./common/utility/strutil.hpp
    ./common/utility/timeutil.cpp
    ./common/utility/timeutil.hpp
    

    ./common/network/Connection.cpp
    ./common/network/Connection.hpp
    ./common/network/Timer.cpp
    ./common/network/Timer.hpp
    ./common/network/packets/allpackets.hpp
    ./common/network/packets/ErrorPacket.cpp
    ./common/network/packets/ErrorPacket.hpp
    ./common/network/packets/IdentPacket.cpp
    ./common/network/packets/IdentPacket.hpp
    ./common/network/packets/LoadPacket.cpp
    ./common/network/packets/LoadPacket.hpp
    ./common/network/packets/NopPacket.cpp
    ./common/network/packets/NopPacket.hpp
    ./common/network/packets/Packet.cpp
    ./common/network/packets/Packet.hpp
    ./common/network/packets/PacketType.cpp
    ./common/network/packets/PacketType.hpp
    ./common/network/packets/PlayPacket.cpp
    ./common/network/packets/PlayPacket.hpp
    ./common/network/packets/ShowPacket.cpp
    ./common/network/packets/ShowPacket.hpp
    ./common/network/packets/SyncPacket.cpp
    ./common/network/packets/SyncPacket.hpp

    ./thirdparty/rtaudio/RtAudio.cpp
    ./thirdparty/rtaudio/RtAudio.h

)
target_compile_definitions(ShowClient PRIVATE
        ASIO_STANDALONE
)

if (WIN32)
    target_compile_definitions(ShowClient PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Debug>:_DEBUG=1>
        __WINDOWS_DS__
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        _WIN32_WINNT=0x0A00
    )

    target_compile_options(ShowClient PRIVATE
        $<$<CONFIG:Release>:/O2>
    )

    target_link_libraries(ShowClient
        PUBLIC
            ole32
            dsound
    )
else()
    target_compile_definitions(ShowClient PRIVATE
        $<$<CONFIG:Debug>:DEBUG=1>
    )

    target_compile_options(ShowClient PRIVATE
        $<$<CONFIG:Release>:-O2>
        -Wno-deprecated-declarations
    )
    if (APPLE)
        target_compile_definitions(ShowClient PRIVATE
            __MACOSX_CORE__
        )

        FIND_LIBRARY(COREAUDIO CoreAudio)
        FIND_LIBRARY(COREFOUNDATION CoreFoundation )
        MARK_AS_ADVANCED ( COREAUDIO COREFOUNDATION )

        target_link_libraries(ShowClient
            PUBLIC
                pthread
                ${COREAUDIO}
                ${COREFOUNDATION}
        )

    else ()
        target_compile_definitions(ShowClient PRIVATE
            __LINUX_ALSA__
        )

        if (BEAGLE)
            target_compile_definitions(ShowClient PRIVATE
                BEAGLE=1
                STATUSLED=1
            )
        endif (BEAGLE)
    
        target_link_libraries(ShowClient
            PUBLIC
                asound
                pthread
        )

    endif (APPLE)
    
endif (WIN32)

target_include_directories(ShowClient
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/common/
        ${PROJECT_SOURCE_DIR}/thirdparty/
        ${PROJECT_SOURCE_DIR}/thirdparty/asio/
)


