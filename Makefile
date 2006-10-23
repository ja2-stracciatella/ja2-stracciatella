ICONV = iconv

SDL_CONFIG = sdl-config
CFLAGS_SDL := $(shell $(SDL_CONFIG) --cflags)
LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)

CFLAGS += -g

CFLAGS += $(CFLAGS_SDL)
CFLAGS += -I ja2/Build
CFLAGS += -I ja2/Build/Tactical
CFLAGS += -I ja2/Build/Strategic
CFLAGS += -I ja2/Build/Editor
CFLAGS += -I ja2/Build/Res
CFLAGS += -I ja2/Build/Laptop
CFLAGS += -I ja2/Build/Utils
CFLAGS += -I ja2/Build/TileEngine
CFLAGS += -I ja2/Build/TacticalAI
CFLAGS += -I sgp

#CFLAGS += -Wall
#CFLAGS += -W
CFLAGS += -Werror
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings

CFLAGS += -DJA2
#CFLAGS += -D_DEBUG

CCFLAGS += $(CFLAGS)
CCFLAGS += -std=gnu99
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wsequence-point

CXXFLAGS += $(CFLAGS)

LDFLAGS += $(LDFLAGS_SDL)
LDFLAGS += -lm
LDFLAGS += -lz

SRCS :=
SRCS += ja2/Build/AniViewScreen.c
SRCS += ja2/Build/Credits.c
SRCS += ja2/Build/Editor/Cursor_Modes.c
SRCS += ja2/Build/Editor/EditScreen.c
SRCS += ja2/Build/Editor/Edit_Sys.c
SRCS += ja2/Build/Editor/EditorBuildings.c
SRCS += ja2/Build/Editor/EditorItems.c
SRCS += ja2/Build/Editor/EditorMapInfo.c
SRCS += ja2/Build/Editor/EditorMercs.c
SRCS += ja2/Build/Editor/EditorTerrain.c
SRCS += ja2/Build/Editor/Editor_Callbacks.c
SRCS += ja2/Build/Editor/Editor_Modes.c
SRCS += ja2/Build/Editor/Editor_Taskbar_Creation.c
SRCS += ja2/Build/Editor/Editor_Taskbar_Utils.c
SRCS += ja2/Build/Editor/Editor_Undo.c
SRCS += ja2/Build/Editor/Item_Statistics.c
SRCS += ja2/Build/Editor/LoadScreen.c
SRCS += ja2/Build/Editor/MessageBox.c
SRCS += ja2/Build/Editor/NewSmooth.c
SRCS += ja2/Build/Editor/PopupMenu.c
SRCS += ja2/Build/Editor/Road_Smoothing.c
SRCS += ja2/Build/Editor/Sector_Summary.c
SRCS += ja2/Build/Editor/SelectWin.c
SRCS += ja2/Build/Editor/SmartMethod.c
SRCS += ja2/Build/Editor/Smooth.c
SRCS += ja2/Build/Editor/Smoothing_Utils.c
SRCS += ja2/Build/Fade_Screen.c
SRCS += ja2/Build/GameInitOptionsScreen.c
SRCS += ja2/Build/GameLoop.c
SRCS += ja2/Build/GameScreen.c
SRCS += ja2/Build/GameSettings.c
SRCS += ja2/Build/GameVersion.c
SRCS += ja2/Build/HelpScreen.c
SRCS += ja2/Build/Init.c
SRCS += ja2/Build/Intro.c
SRCS += ja2/Build/JA2_Splash.c
SRCS += ja2/Build/JAScreens.c
SRCS += ja2/Build/Laptop/AIM.c
SRCS += ja2/Build/Laptop/AIMArchives.c
SRCS += ja2/Build/Laptop/AIMFacialIndex.c
SRCS += ja2/Build/Laptop/AIMHistory.c
SRCS += ja2/Build/Laptop/AIMPolicies.c
SRCS += ja2/Build/Laptop/AIMSort.c
SRCS += ja2/Build/Laptop/AimLinks.c
SRCS += ja2/Build/Laptop/AimMembers.c
SRCS += ja2/Build/Laptop/BobbyR.c
SRCS += ja2/Build/Laptop/BobbyRAmmo.c
SRCS += ja2/Build/Laptop/BobbyRArmour.c
SRCS += ja2/Build/Laptop/BobbyRGuns.c
SRCS += ja2/Build/Laptop/BobbyRMailOrder.c
SRCS += ja2/Build/Laptop/BobbyRMisc.c
SRCS += ja2/Build/Laptop/BobbyRShipments.c
SRCS += ja2/Build/Laptop/BobbyRUsed.c
SRCS += ja2/Build/Laptop/BrokenLink.c
SRCS += ja2/Build/Laptop/CharProfile.c
SRCS += ja2/Build/Laptop/EMail.c
SRCS += ja2/Build/Laptop/Files.c
SRCS += ja2/Build/Laptop/Finances.c
SRCS += ja2/Build/Laptop/Florist.c
SRCS += ja2/Build/Laptop/Florist_Cards.c
SRCS += ja2/Build/Laptop/Florist_Gallery.c
SRCS += ja2/Build/Laptop/Florist_Order_Form.c
SRCS += ja2/Build/Laptop/Funeral.c
SRCS += ja2/Build/Laptop/History.c
SRCS += ja2/Build/Laptop/IMPVideoObjects.c
SRCS += ja2/Build/Laptop/IMP_AboutUs.c
SRCS += ja2/Build/Laptop/IMP_Attribute_Entrance.c
SRCS += ja2/Build/Laptop/IMP_Attribute_Finish.c
SRCS += ja2/Build/Laptop/IMP_Attribute_Selection.c
SRCS += ja2/Build/Laptop/IMP_Begin_Screen.c
SRCS += ja2/Build/Laptop/IMP_Compile_Character.c
SRCS += ja2/Build/Laptop/IMP_Confirm.c
SRCS += ja2/Build/Laptop/IMP_Finish.c
SRCS += ja2/Build/Laptop/IMP_HomePage.c
SRCS += ja2/Build/Laptop/IMP_MainPage.c
SRCS += ja2/Build/Laptop/IMP_Personality_Entrance.c
SRCS += ja2/Build/Laptop/IMP_Personality_Finish.c
SRCS += ja2/Build/Laptop/IMP_Personality_Quiz.c
SRCS += ja2/Build/Laptop/IMP_Portraits.c
SRCS += ja2/Build/Laptop/IMP_Text_System.c
SRCS += ja2/Build/Laptop/IMP_Voices.c
SRCS += ja2/Build/Laptop/Insurance.c
SRCS += ja2/Build/Laptop/Insurance_Comments.c
SRCS += ja2/Build/Laptop/Insurance_Contract.c
SRCS += ja2/Build/Laptop/Insurance_Info.c
SRCS += ja2/Build/Laptop/Laptop.c
SRCS += ja2/Build/Laptop/Mercs.c
SRCS += ja2/Build/Laptop/Mercs_Account.c
SRCS += ja2/Build/Laptop/Mercs_Files.c
SRCS += ja2/Build/Laptop/Mercs_No_Account.c
SRCS += ja2/Build/Laptop/Personnel.c
SRCS += ja2/Build/Laptop/Sir-Tech.c
SRCS += ja2/Build/Laptop/Store_Inventory.c
SRCS += ja2/Build/Loading_Screen.c
SRCS += ja2/Build/MainMenuScreen.c
SRCS += ja2/Build/MessageBoxScreen.c
SRCS += ja2/Build/Options_Screen.c
SRCS += ja2/Build/SaveLoadGame.c
SRCS += ja2/Build/SaveLoadScreen.c
SRCS += ja2/Build/Screens.c
SRCS += ja2/Build/Strategic/AI_Viewer.c
SRCS += ja2/Build/Strategic/Assignments.c
SRCS += ja2/Build/Strategic/Auto_Resolve.c
SRCS += ja2/Build/Strategic/Campaign_Init.c
SRCS += ja2/Build/Strategic/Creature_Spreading.c
SRCS += ja2/Build/Strategic/Game_Clock.c
SRCS += ja2/Build/Strategic/Game_Event_Hook.c
SRCS += ja2/Build/Strategic/Game_Events.c
SRCS += ja2/Build/Strategic/Game_Init.c
SRCS += ja2/Build/Strategic/Hourly_Update.c
SRCS += ja2/Build/Strategic/MapScreen.c
SRCS += ja2/Build/Strategic/Map_Screen_Helicopter.c
SRCS += ja2/Build/Strategic/Map_Screen_Interface.c
SRCS += ja2/Build/Strategic/Map_Screen_Interface_Border.c
SRCS += ja2/Build/Strategic/Map_Screen_Interface_Bottom.c
SRCS += ja2/Build/Strategic/Map_Screen_Interface_Map.c
SRCS += ja2/Build/Strategic/Map_Screen_Interface_Map_Inventory.c
SRCS += ja2/Build/Strategic/Map_Screen_Interface_TownMine_Info.c
SRCS += ja2/Build/Strategic/Meanwhile.c
SRCS += ja2/Build/Strategic/Merc_Contract.c
SRCS += ja2/Build/Strategic/Player_Command.c
SRCS += ja2/Build/Strategic/PreBattle_Interface.c
SRCS += ja2/Build/Strategic/Queen_Command.c
SRCS += ja2/Build/Strategic/QuestText.c
SRCS += ja2/Build/Strategic/Quest_Debug_System.c
SRCS += ja2/Build/Strategic/Quests.c
SRCS += ja2/Build/Strategic/Scheduling.c
SRCS += ja2/Build/Strategic/Strategic.c
SRCS += ja2/Build/Strategic/StrategicMap.c
SRCS += ja2/Build/Strategic/Strategic_AI.c
SRCS += ja2/Build/Strategic/Strategic_Event_Handler.c
SRCS += ja2/Build/Strategic/Strategic_Merc_Handler.c
SRCS += ja2/Build/Strategic/Strategic_Mines.c
SRCS += ja2/Build/Strategic/Strategic_Movement.c
SRCS += ja2/Build/Strategic/Strategic_Movement_Costs.c
SRCS += ja2/Build/Strategic/Strategic_Pathing.c
SRCS += ja2/Build/Strategic/Strategic_Status.c
SRCS += ja2/Build/Strategic/Strategic_Town_Loyalty.c
SRCS += ja2/Build/Strategic/Strategic_Town_Reputation.c
SRCS += ja2/Build/Strategic/Strategic_Turns.c
SRCS += ja2/Build/Strategic/Town_Militia.c
SRCS += ja2/Build/Sys_Globals.c
SRCS += ja2/Build/Tactical/Air_Raid.c
SRCS += ja2/Build/Tactical/Animation_Cache.c
SRCS += ja2/Build/Tactical/Animation_Control.c
SRCS += ja2/Build/Tactical/Animation_Data.c
SRCS += ja2/Build/Tactical/ArmsDealerInvInit.c
SRCS += ja2/Build/Tactical/Arms_Dealer_Init.c
SRCS += ja2/Build/Tactical/Auto_Bandage.c
SRCS += ja2/Build/Tactical/Boxing.c
SRCS += ja2/Build/Tactical/Bullets.c
SRCS += ja2/Build/Tactical/Campaign.c
SRCS += ja2/Build/Tactical/Civ_Quotes.c
SRCS += ja2/Build/Tactical/Dialogue_Control.c
SRCS += ja2/Build/Tactical/DisplayCover.c
SRCS += ja2/Build/Tactical/Drugs_And_Alcohol.c
SRCS += ja2/Build/Tactical/End_Game.c
SRCS += ja2/Build/Tactical/Enemy_Soldier_Save.c
SRCS += ja2/Build/Tactical/FOV.c
SRCS += ja2/Build/Tactical/Faces.c
SRCS += ja2/Build/Tactical/Gap.c
SRCS += ja2/Build/Tactical/Handle_Doors.c
SRCS += ja2/Build/Tactical/Handle_Items.c
SRCS += ja2/Build/Tactical/Handle_UI.c
SRCS += ja2/Build/Tactical/Handle_UI_Plan.c
SRCS += ja2/Build/Tactical/Interface.c
SRCS += ja2/Build/Tactical/Interface_Control.c
SRCS += ja2/Build/Tactical/Interface_Cursors.c
SRCS += ja2/Build/Tactical/Interface_Dialogue.c
SRCS += ja2/Build/Tactical/Interface_Items.c
SRCS += ja2/Build/Tactical/Interface_Panels.c
SRCS += ja2/Build/Tactical/Interface_Utils.c
SRCS += ja2/Build/Tactical/Inventory_Choosing.c
SRCS += ja2/Build/Tactical/Items.c
SRCS += ja2/Build/Tactical/Keys.c
SRCS += ja2/Build/Tactical/LOS.c
SRCS += ja2/Build/Tactical/Map_Information.c
SRCS += ja2/Build/Tactical/Merc_Entering.c
SRCS += ja2/Build/Tactical/Merc_Hiring.c
SRCS += ja2/Build/Tactical/Militia_Control.c
SRCS += ja2/Build/Tactical/Morale.c
SRCS += ja2/Build/Tactical/OppList.c
SRCS += ja2/Build/Tactical/Overhead.c
SRCS += ja2/Build/Tactical/PathAI.c
SRCS += ja2/Build/Tactical/Points.c
SRCS += ja2/Build/Tactical/QArray.c
SRCS += ja2/Build/Tactical/Real_Time_Input.c
SRCS += ja2/Build/Tactical/Rotting_Corpses.c
SRCS += ja2/Build/Tactical/ShopKeeper_Interface.c
SRCS += ja2/Build/Tactical/SkillCheck.c
SRCS += ja2/Build/Tactical/Soldier_Add.c
SRCS += ja2/Build/Tactical/Soldier_Ani.c
SRCS += ja2/Build/Tactical/Soldier_Control.c
SRCS += ja2/Build/Tactical/Soldier_Create.c
SRCS += ja2/Build/Tactical/Soldier_Find.c
SRCS += ja2/Build/Tactical/Soldier_Init_List.c
SRCS += ja2/Build/Tactical/Soldier_Profile.c
SRCS += ja2/Build/Tactical/Soldier_Tile.c
SRCS += ja2/Build/Tactical/Spread_Burst.c
SRCS += ja2/Build/Tactical/Squads.c
SRCS += ja2/Build/Tactical/Strategic_Exit_GUI.c
SRCS += ja2/Build/Tactical/Structure_Wrap.c
SRCS += ja2/Build/Tactical/Tactical_Save.c
SRCS += ja2/Build/Tactical/Tactical_Turns.c
SRCS += ja2/Build/Tactical/TeamTurns.c
SRCS += ja2/Build/Tactical/Turn_Based_Input.c
SRCS += ja2/Build/Tactical/UI_Cursors.c
SRCS += ja2/Build/Tactical/Vehicles.c
SRCS += ja2/Build/Tactical/Weapons.c
SRCS += ja2/Build/Tactical/World_Items.c
SRCS += ja2/Build/TacticalAI/AIList.c
SRCS += ja2/Build/TacticalAI/AIMain.c
SRCS += ja2/Build/TacticalAI/AIUtils.c
SRCS += ja2/Build/TacticalAI/Attacks.c
SRCS += ja2/Build/TacticalAI/CreatureDecideAction.c
SRCS += ja2/Build/TacticalAI/DecideAction.c
SRCS += ja2/Build/TacticalAI/FindLocations.c
SRCS += ja2/Build/TacticalAI/Knowledge.c
SRCS += ja2/Build/TacticalAI/Medical.c
SRCS += ja2/Build/TacticalAI/Movement.c
SRCS += ja2/Build/TacticalAI/NPC.c
SRCS += ja2/Build/TacticalAI/PanicButtons.c
SRCS += ja2/Build/TacticalAI/QuestDebug.c
SRCS += ja2/Build/TacticalAI/Realtime.c
SRCS += ja2/Build/TileEngine/Ambient_Control.c
SRCS += ja2/Build/TileEngine/Buildings.c
SRCS += ja2/Build/TileEngine/Environment.c
SRCS += ja2/Build/TileEngine/Exit_Grids.c
SRCS += ja2/Build/TileEngine/Explosion_Control.c
SRCS += ja2/Build/TileEngine/Fog_Of_War.c
SRCS += ja2/Build/TileEngine/Interactive_Tiles.c
SRCS += ja2/Build/TileEngine/Isometric_Utils.c
SRCS += ja2/Build/TileEngine/LightEffects.c
SRCS += ja2/Build/TileEngine/Lighting.c
SRCS += ja2/Build/TileEngine/Map_Edgepoints.c
SRCS += ja2/Build/TileEngine/Overhead_Map.c
SRCS += ja2/Build/TileEngine/Phys_Math.c
SRCS += ja2/Build/TileEngine/Physics.c
SRCS += ja2/Build/TileEngine/Pits.c
SRCS += ja2/Build/TileEngine/Radar_Screen.c
SRCS += ja2/Build/TileEngine/RenderWorld.c # inline assembler
SRCS += ja2/Build/TileEngine/Render_Dirty.c
SRCS += ja2/Build/TileEngine/Render_Fun.c
SRCS += ja2/Build/TileEngine/SaveLoadMap.c
SRCS += ja2/Build/TileEngine/Shade_Table_Util.c
SRCS += ja2/Build/TileEngine/Simple_Render_Utils.c
SRCS += ja2/Build/TileEngine/Smell.c
SRCS += ja2/Build/TileEngine/SmokeEffects.c
SRCS += ja2/Build/TileEngine/Structure.c
SRCS += ja2/Build/TileEngine/SysUtil.c
SRCS += ja2/Build/TileEngine/Tactical_Placement_GUI.c
SRCS += ja2/Build/TileEngine/TileDat.c
SRCS += ja2/Build/TileEngine/TileDef.c
SRCS += ja2/Build/TileEngine/Tile_Animation.c
SRCS += ja2/Build/TileEngine/Tile_Cache.c
SRCS += ja2/Build/TileEngine/Tile_Surface.c
SRCS += ja2/Build/TileEngine/WorldDat.c
SRCS += ja2/Build/TileEngine/WorldDef.c
SRCS += ja2/Build/TileEngine/WorldMan.c
SRCS += ja2/Build/Utils/Animated_ProgressBar.c
SRCS += ja2/Build/Utils/Cinematics.c
SRCS += ja2/Build/Utils/Cursors.c
SRCS += ja2/Build/Utils/Debug_Control.c
SRCS += ja2/Build/Utils/Encrypted_File.c
SRCS += ja2/Build/Utils/Event_Manager.c
SRCS += ja2/Build/Utils/Event_Pump.c
SRCS += ja2/Build/Utils/Font_Control.c
SRCS += ja2/Build/Utils/MapUtility.c
SRCS += ja2/Build/Utils/MercTextBox.c
SRCS += ja2/Build/Utils/Message.c
SRCS += ja2/Build/Utils/Multi_Language_Graphic_Utils.c
SRCS += ja2/Build/Utils/Multilingual_Text_Code_Generator.c
SRCS += ja2/Build/Utils/Music_Control.c
SRCS += ja2/Build/Utils/PopUpBox.c
SRCS += ja2/Build/Utils/Quantize.cc
SRCS += ja2/Build/Utils/Quantize_Wrap.cc
SRCS += ja2/Build/Utils/STIConvert.c
SRCS += ja2/Build/Utils/Slider.c
SRCS += ja2/Build/Utils/Sound_Control.c
SRCS += ja2/Build/Utils/Text_Input.c
SRCS += ja2/Build/Utils/Text_Utils.c
SRCS += ja2/Build/Utils/Timer_Control.c
SRCS += ja2/Build/Utils/Utilities.c
SRCS += ja2/Build/Utils/WordWrap.c
SRCS += ja2/Build/Utils/_DutchText.c
SRCS += ja2/Build/Utils/_EnglishText.c
SRCS += ja2/Build/Utils/_FrenchText.c
SRCS += ja2/Build/Utils/GermanText.c
SRCS += ja2/Build/Utils/_ItalianText.c
SRCS += ja2/Build/Utils/_JA25EnglishText.c
SRCS += ja2/Build/Utils/JA25GermanText.c
SRCS += ja2/Build/Utils/_PolishText.c
SRCS += ja2/Build/Utils/_RussianText.c
SRCS += sgp/Button_Sound_Control.c
SRCS += sgp/Button_System.c
SRCS += sgp/Compression.c
SRCS += sgp/Container.c
SRCS += sgp/Cursor_Control.c
SRCS += sgp/Debug.c
SRCS += sgp/English.c
SRCS += sgp/FileMan.c
SRCS += sgp/Font.c
SRCS += sgp/HImage.c
SRCS += sgp/ImpTGA.c
SRCS += sgp/Input.c
#SRCS += sgp/JA2_Libs.c # XXX is included in sgp/LibraryDataBase.c
SRCS += sgp/LibraryDataBase.c
SRCS += sgp/Line.c
SRCS += sgp/MemMan.c
SRCS += sgp/MouseSystem.c
SRCS += sgp/PCX.c
SRCS += sgp/Random.c
SRCS += sgp/SGP.c
SRCS += sgp/STCI.c
SRCS += sgp/Shading.c # inline assembler
SRCS += sgp/Smack_Stub.c
SRCS += sgp/SoundMan.c
SRCS += sgp/Stubs.c
SRCS += sgp/VObject.c
SRCS += sgp/VObject_Blitters.c # inline assembler
SRCS += sgp/VSurface.c
SRCS += sgp/Video.c

DEPS = $(filter %.d, $(SRCS:.c=.d) $(SRCS:.cc=.d))
OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cc=.o))

.SUFFIXES:
.SUFFIXES: .c .cc .d .o

all: ja

depend: $(DEPS)

ifeq ($(findstring $(MAKECMDGOALS), clean depend Data),)
-include $(DEPS)
endif

ja: $(OBJS)
	@echo '===> LD $@'
	@$(CXX) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	@echo '===> CC $<'
	@$(CC) $(CCFLAGS) -c $< -o $@

.cc.o:
	@echo '===> CXX $<'
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.c.d:
	@echo '===> DEP $<'
	@$(CC) $(CCFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

.cc.d:
	@echo '===> DEP $<'
	@$(CXX) $(CXXFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

clean:
	@echo '===> CLEAN'
	@rm -fr $(DEPS) $(OBJS) ja2/Build/Utils/JA25GermanText.c ja2/Build/Utils/GermanText.c

ja2/Build/Utils/JA25GermanText.c: ja2/Build/Utils/_JA25GermanText.c
ja2/Build/Utils/GermanText.c: ja2/Build/Utils/_GermanText.c

ja2/Build/Utils/JA25GermanText.c ja2/Build/Utils/GermanText.c:
	@echo '===> ICONV $<'
	@$(ICONV) -f ISO8859-15 -t UTF-8 < $< > $@


Data:
	@echo "===> LN $(DATA)"
	@mkdir Data Data/TILECACHE
	@ln -s $(DATA)/AMBIENT.SLF                Data/Ambient.slf
	@ln -s $(DATA)/ANIMS.SLF                  Data/Anims.slf
	@ln -s $(DATA)/BATTLESNDS.SLF             Data/BattleSnds.slf
	@ln -s $(DATA)/BIGITEMS.SLF               Data/BigItems.slf
	@ln -s $(DATA)/BINARYDATA.SLF             Data/BinaryData.slf
	@ln -s $(DATA)/CURSORS.SLF                Data/Cursors.slf
	@ln -s $(DATA)/DATA.SLF                   Data/Data.slf
	@ln -s $(DATA)/FACES.SLF                  Data/Faces.slf
	@ln -s $(DATA)/FONTS.SLF                  Data/Fonts.slf
	@ln -s $(DATA)/GERMAN.SLF                 Data/German.slf
	@ln -s $(DATA)/INTERFACE.SLF              Data/InterFace.slf
	@ln -s $(DATA)/INTRO.SLF                  Data/Intro.slf
	@ln -s $(DATA)/LAPTOP.SLF                 Data/Laptop.slf
	@ln -s $(DATA)/LOADSCREENS.SLF            Data/LoadScreens.slf
	@ln -s $(DATA)/MAPS.SLF                   Data/Maps.slf
	@ln -s $(DATA)/MERCEDT.SLF                Data/MercEdt.slf
	@ln -s $(DATA)/MUSIC.SLF                  Data/Music.slf
	@ln -s $(DATA)/NPCDATA.SLF                Data/NpcData.slf
	@ln -s $(DATA)/NPC_SPEECH.SLF             Data/Npc_Speech.slf
	@ln -s $(DATA)/RADARMAPS.SLF              Data/RadarMaps.slf
	@ln -s $(DATA)/SOUNDS.SLF                 Data/Sounds.slf
	@ln -s $(DATA)/SPEECH.SLF                 Data/Speech.slf
	@ln -s $(DATA)/TILESETS.SLF               Data/TileSets.slf
	@ln -s $(DATA)/TILECACHE/CP_DEAD1.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CP_DEAD1_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CT_DEAD.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CT_DEAD1.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CT_DEAD1_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CT_DEAD_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CW_DEAD1.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/CW_DEAD1_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/C_DEAD1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/C_DEAD1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/FLY_SHDW.sti     Data/TILECACHE/FLY_SHDW.STI
	@ln -s $(DATA)/TILECACHE/FT_DEAD1.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/FT_DEAD1_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_D_BCK.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_D_BCK_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_dhd.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_dhd_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_fall.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_fall_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_fallf.JSD    Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_fallf_nb.jsd Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_fwd.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_fwd_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_jfk.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_jfk_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_prn.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_prn_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_wtr.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/F_d_wtr_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/GEN_BLOW.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/HELI_SH.sti      Data/TILECACHE/HELI_SH.STI
	@ln -s $(DATA)/TILECACHE/HM_WREK.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/H_dead2.jsd      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/H_dead2_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/IC_WREK.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/I_DEAD1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/I_DEAD1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/KNIFING.sti      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/K_dead2.jsd      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/K_dead2_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/L_dead1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/L_dead1_nb.JSD   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/MINIBOOM.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/MSLE_SMK.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/MSLE_SPT.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/MUSTCHZE.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/MUSTRD.sti       Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/MUST_EXP.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_BCK.JSD        Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_BCK_nb.jsd     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_DEAD1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_DEAD1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_D_BCK.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_D_BCK_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_JFK.JSD        Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_JFK_nb.jsd     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_dhd.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_dhd_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_fall.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_fall_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_fallf.JSD    Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_fallf_nb.jsd Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_fwd.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_fwd_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_jfk.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_jfk_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_prn.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_prn_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_wtr.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/M_d_wtr_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/Mustard2.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/P_DECOMP2.jsd    Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SHCKWAVE.sti     Data/TILECACHE/shckwave.STI
	@ln -s $(DATA)/TILECACHE/SMALMUST.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SMALSMKE.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SMALTEAR.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SMKECHZE.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SMOKE.sti        Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SPIT2.sti        Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/SPIT_GAS.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/S_DEAD1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/S_DEAD1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/S_D_BCK.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/S_D_BCK_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/TEARCHZE.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/TEAR_EXP.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/Teargas.sti      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/Tk2_wrek.jsd     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/Tk2_wrek_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/WAT_EXP.sti      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/W_DEAD1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/W_DEAD1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/W_DEAD2.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/W_DEAD2_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/ZGRAV_A.sti      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/ZGRAV_B.sti      Data/TILECACHE/ZGRAV_B.STI
	@ln -s $(DATA)/TILECACHE/ZGRAV_C.sti      Data/TILECACHE/ZGRAV_C.STI
	@ln -s $(DATA)/TILECACHE/ZGRAV_D.sti      Data/TILECACHE/ZGRAV_D.STI
	@ln -s $(DATA)/TILECACHE/f_dead.JSD       Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/f_dead_nb.jsd    Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/ft_dead2.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/ft_dead2_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/h_dead1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/h_dead1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/j_dead.jsd       Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/k_dead1.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/k_dead1_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/m_dead2.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/m_dead2_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/mn_dead1.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/mn_dead1_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/mn_dead2.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/mn_dead2_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/msle_sma.sti     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/q_dead.JSD       Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/q_dead_nb.jsd    Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/qn_dead.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_burnt.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_dhd.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_dhd_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_fall.JSD     Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_fall_nb.jsd  Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_fallf.JSD    Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_fallf_nb.jsd Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_fwd.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_fwd_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_jfk.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_jfk_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_prn.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_prn_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_wtr.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_d_wtr_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_dead2.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_dead2_nb.jsd   Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/s_expld.JSD      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/tk_wrek.jsd      Data/TILECACHE/
	@ln -s $(DATA)/TILECACHE/tk_wrek_nb.jsd   Data/TILECACHE/
