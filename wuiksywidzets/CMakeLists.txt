cmake_minimum_required(VERSION 3.0)

project(wuiksywidzets)

set(CMAKE_CXX_STANDARD 17)

set(wxWidgets_ROOT_DIR "C:/Users/EmKa/Documents/wxWidgets")
find_package(wxWidgets REQUIRED net gl core base)
find_package(cryptopp CONFIG REQUIRED)
include(${wxWidgets_USE_FILE})

set(SOURCES
  src/main_frame.cpp
  src/user_panel.cpp
  src/bot_config.cpp
  src/add_buff_dialog.cpp
  src/add_item_dialog.cpp
  src/add_spell_dialog.cpp
  src/bounding_box_selection_panel.cpp
  src/manor_data.cpp
  src/add_manor_dialog.cpp
  src/add_craft_dialog.cpp
  src/add_dos_dialog.cpp
  src/craft_data.cpp
  src/main_notebook.cpp
  src/targeting_tab.cpp  
  src/farming_tab.cpp
  src/spells_tab.cpp
  src/items_tab.cpp
  src/party_tab.cpp
  src/manor_tab.cpp
  src/craft_tab.cpp
  src/dos_and_donts_tab.cpp
  src/item_data.cpp
  src/npc_data.cpp
  src/skill_data.cpp
  src/alarm_tab.cpp
  src/follow_tab.cpp
  src/pick_tab.cpp
  src/relax_tab.cpp
  src/out_of_party_tab.cpp
  src/special_tricks_tab.cpp
  src/map_tab.cpp
  src/add_oop_dialog.cpp
  src/beastfarm_tab.cpp
  src/beastfarm_data.cpp
  src/add_beast_dialog.cpp
  src/aes_boy.cpp
  src/connector.cpp
)

set(INCLUDES
  inc/event_identifiers.hpp
  inc/main_frame.hpp
  inc/user_panel.hpp
  inc/bot_config.hpp
  inc/binary_serializer.hpp
  inc/binary_deserializer.hpp
  inc/add_buff_dialog.hpp
  inc/add_item_dialog.hpp
  inc/add_spell_dialog.hpp
  inc/bounding_box_selection_panel.hpp
  inc/manor_data.hpp
  inc/add_manor_dialog.hpp
  inc/add_craft_dialog.hpp
  inc/add_dos_dialog.hpp
  inc/craft_data.hpp
  inc/main_notebook.hpp
  inc/base_tab.hpp
  inc/targeting_tab.hpp
  inc/farming_tab.hpp
  inc/spells_tab.hpp
  inc/items_tab.hpp
  inc/party_tab.hpp
  inc/manor_tab.hpp
  inc/craft_tab.hpp
  inc/dos_and_donts_tab.hpp
  inc/current_character_data.hpp
  inc/item_data.hpp
  inc/npc_data.hpp
  inc/skill_data.hpp
  inc/alarm_tab.hpp
  inc/follow_tab.hpp
  inc/pick_tab.hpp
  inc/relax_tab.hpp
  inc/out_of_party_tab.hpp
  inc/special_tricks_tab.hpp
  inc/map_tab.hpp
  inc/add_oop_dialog.hpp
  inc/beastfarm_tab.hpp
  inc/beastfarm_data.hpp
  inc/add_beast_dialog.hpp
  inc/aes_boy.hpp
  inc/connector.hpp
)

add_executable(${PROJECT_NAME} WIN32 src/main.cpp ${SOURCES} ${INCLUDES})
target_include_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_INCLUDE_DIRS} inc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} cryptopp-static)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/rsrc
  ${CMAKE_CURRENT_BINARY_DIR}/rsrc
)