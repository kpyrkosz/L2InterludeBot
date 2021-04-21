cmake_minimum_required(VERSION 3.11)

project(libowybot)

set(CMAKE_CXX_STANDARD 17)

find_package(cryptopp CONFIG REQUIRED)

set(SOURCES
  src/action_executor.cpp
  src/autoattack_module.cpp
  src/bot_action_penalties.cpp
  src/bot_confiuration.cpp
  src/configuration_setter.cpp
  src/game_data_facade.cpp
  src/grinder.cpp
  src/item_use_module.cpp
  src/main.cpp
  src/mainloop_hook.cpp
  src/manor_crops.cpp
  src/movement_module.cpp
  src/network_table_hook.cpp
  src/party.cpp
  src/party_buff_guard.cpp
  src/player_character.cpp
  src/spell_casting_module.cpp
  src/targeting_module.cpp
  src/craft_module.cpp
  src/alarm_module.cpp
  src/blacklist.cpp
  src/picking_module.cpp
  src/antistuck_module.cpp
  src/oop_module.cpp
  src/fishing_module.cpp
  src/beastfarm_data.cpp
)

set(INCLUDES
  inc/action_executor.hpp
  inc/autoattack_module.hpp
  inc/binary_deserializer.hpp
  inc/binary_serializer.hpp
  inc/bot_action_penalties.hpp
  inc/bot_confiuration.hpp
  inc/configuration_setter.hpp
  inc/game_data_facade.hpp
  inc/grinder.hpp
  inc/item_use_module.hpp
  inc/mainloop_hook.hpp
  inc/manor_crops.hpp
  inc/movement_module.hpp
  inc/native_lineage_classes.hpp
  inc/network_table_hook.hpp
  inc/party.hpp
  inc/party_buff_guard.hpp
  inc/player_character.hpp
  inc/protocol.hpp
  inc/spell_casting_module.hpp
  inc/targeting_module.hpp
  inc/craft_module.hpp
  inc/alarm_module.hpp
  inc/blacklist.hpp
  inc/picking_module.hpp
  inc/antistuck_module.hpp
  inc/oop_module.hpp
  inc/fishing_module.hpp
  inc/beastfarm_data.hpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${INCLUDES})
target_include_directories(${PROJECT_NAME} PRIVATE inc)
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/native/core.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/native/engine.lib
  ws2_32
  Winmm
  cryptopp-static
)
