#CFLAGS += -I ../../ja2/trunk/Build
#CFLAGS += -I ../../ja2/trunk/Build/Tactical
#CFLAGS += -I ../../ja2/trunk/Build/Strategic
#CFLAGS += -I ../../ja2/trunk/Build/Editor
#CFLAGS += -I ../../ja2/trunk/Build/Res
#CFLAGS += -I ../../ja2/trunk/Build/Laptop
#CFLAGS += -I ../../ja2/trunk/Build/Utils
#CFLAGS += -I ../../ja2/trunk/Build/TileEngine
#CFLAGS += -I ../../ja2/trunk/Build/TacticalAI
CFLAGS += -I .

#CFLAGS += -Wall -W -std=c99
CFLAGS += -Werror-implicit-function-declaration
CFLAGS += -Wimplicit-int
CFLAGS += -Wsequence-point
CFLAGS += -Wreturn-type
#CFLAGS += -Wwrite-strings
CFLAGS += -Werror

CFLAGS += -DJA2

SRCS += ./Button_Sound_Control.c
SRCS += ./Button_System.c
SRCS += ./Compression.c
SRCS += ./Container.c
SRCS += ./Cursor_Control.c
SRCS += ./DbMan.c
#SRCS += ./Debug.c
#SRCS += ./DirectDraw_Calls.c
#SRCS += ./DirectX_Common.c
SRCS += ./English.c
#SRCS += ./FileMan.c
#SRCS += ./Flic.c
SRCS += ./Font.c
SRCS += ./HImage.c
SRCS += ./ImpTGA.c
#SRCS += ./Input.c
#SRCS += ./Install.c
SRCS += ./JA2_Libs.c
#SRCS += ./LibraryDataBase.c
SRCS += ./Line.c
#SRCS += ./MemMan.c
SRCS += ./MouseSystem.c
#SRCS += ./Mutex_Manager.c
SRCS += ./PCX.c
SRCS += ./Random.c
#SRCS += ./RegInst.c
#SRCS += ./SGP.c
SRCS += ./STCI.c
#SRCS += ./Shading.c # XXX Inline Assembler
#SRCS += ./SoundMan.c
#SRCS += ./Timer.c
SRCS += ./VObject.c
#SRCS += ./VObject_Blitters.c # XXX Inline Assembler
#SRCS += ./VSurface.c
#SRCS += ./Video.c
#SRCS += ./WinFont.c
OBJS = $(SRCS:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

all: $(OBJS)

.c.o:
	@echo '===> Übersetze $<'
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo '===> Räume auf'
	@rm -fr $(OBJS)
