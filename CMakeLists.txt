cmake_minimum_required(VERSION 3.16)
project(SightCityUnderworld LANGUAGES C CXX)

option(BUILD_VITA "Build PS Vita target with VitaSDK + vita2d" OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CORE_SOURCES
    src/core/Math.cpp
    src/game/Player.cpp
    src/game/Vehicle.cpp
    src/game/VehiclePhysics.cpp
    src/game/AircraftSystem.cpp
    src/game/Camera.cpp
    src/game/World.cpp
    src/game/SightCityMap.cpp
    src/game/EnvironmentSystem.cpp
    src/game/WorldCollisionSystem.cpp
    src/game/TimeOfDaySystem.cpp
    src/game/WeatherSystem.cpp
    src/game/WantedSystem.cpp
    src/game/LawSystem.cpp
    src/game/Settings.cpp
    src/game/AimAssist.cpp
    src/game/MapDiscovery.cpp
    src/game/Properties.cpp
    src/game/WorldMarkers.cpp
    src/game/Traffic.cpp
    src/game/RoadNetwork.cpp
    src/game/NPC.cpp
    src/game/Economy.cpp
    src/game/MissionSystem.cpp
    src/game/CharacterProfile.cpp
    src/game/CharacterCreator.cpp
    src/game/PhoneSystem.cpp
    src/game/HeistRoom.cpp
    src/game/GarageSystem.cpp
    src/game/VehicleCustomization.cpp
    src/game/DealershipSystem.cpp
    src/game/WeaponSystem.cpp
    src/game/GunStoreSystem.cpp
    src/game/CharacterAI.cpp
    src/game/WorldAI.cpp
    src/game/FireExplosionSystem.cpp
    src/game/EmergencyResponseSystem.cpp
    src/game/StoryGameplay.cpp
    src/game/DynamicIncidentSystem.cpp
    src/game/CharacterMeetingSystem.cpp
    src/game/WildlifeSystem.cpp
    src/game/TrophySystem.cpp
    src/game/CombatSystem.cpp
    src/game/StoryHeistSystem.cpp
    src/game/VitaStoryPolish.cpp
)

if(BUILD_VITA)
    if(NOT DEFINED ENV{VITASDK})
        message(FATAL_ERROR "VITASDK environment variable is required for BUILD_VITA=ON")
    endif()
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

    add_executable(city_limits_vita
        ${CORE_SOURCES}
        src/platform/vita/VitaInput.cpp
        src/platform/vita/VitaRenderer.cpp
        src/platform/vita/main_vita.cpp
    )
    target_include_directories(city_limits_vita PRIVATE src)
    target_compile_definitions(city_limits_vita PRIVATE PLATFORM_VITA=1)
    target_compile_options(city_limits_vita PRIVATE -O2 -ffast-math)
    target_link_libraries(city_limits_vita
        vita2d

        SceDisplay_stub
        SceGxm_stub
        SceCtrl_stub
        SceTouch_stub
        SceSysmodule_stub
        SceKernelDmacMgr_stub

        SceCommonDialog_stub
        SceSharedFb_stub
        SceAppMgr_stub
        SceSysmem_stub
        SceKernelThreadMgr_stub
        SceLibKernel_stub
        ScePgf_stub
        ScePvf_stub

        m
    )

    set(VITA_APP_NAME "Sight City: Underworld")
    set(VITA_TITLEID "SCUW00001")
    set(VITA_VERSION "00.61")
    vita_create_self(city_limits_vita.self city_limits_vita UNSAFE)
    vita_create_vpk(sight_city_underworld.vpk ${VITA_TITLEID} city_limits_vita.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        FILE sce_sys/icon0.png sce_sys/icon0.png
        FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    )
else()
    add_executable(city_limits src/main.cpp ${CORE_SOURCES})
    target_include_directories(city_limits PRIVATE src)
    target_compile_definitions(city_limits PRIVATE PLATFORM_DESKTOP=1)
endif()
