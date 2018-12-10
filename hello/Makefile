##############################################################################################
CC := arm-hisiv200-linux-gcc
##############################################################################################
WORK_PATH := ../
SUBDIR   := common his extdev task server sys it68051 it6615 ti1297 

##############################################################################################
SOFT_VER  := "1.4.2.0"
SOFT_NAME := "DEC-MINE"

##############################################################################################
COMM_SRC := $(foreach n, $(SUBDIR), $(wildcard $(WORK_PATH)$(n)/*.c))
COMM_OBJ := $(COMM_SRC:%.c=%.o)

##############################################################################################
DEFINES := -DCHIP_TYPE_hi3798mv100 -DCFG_HI_SDK_VERSION=HiSTBLinuxV100R003C00SPC062


INC_DIR   := $(foreach n, $(SUBDIR), -I$(WORK_PATH)$(n)) 
INCLUDE   := -I$(WORK_PATH)include -I ./ $(INC_DIR) 
SYS_LIBS := -lpthread -lrt -lm -ldl -lsqlite3 -liconv

CFLAGS := -Wall -g  -march=armv7-a -mcpu=cortex-a9 $(SYS_LIBS)  $(INCLUDE) $(DEFINES)

##############################################################################################
HI_LIBDIR := $(WORK_PATH)lib/static
HI_LIBS :=  $(wildcard $(HI_LIBDIR)/*.a) 

#HI_LIBS := ../lib/static/libeffect.a ../lib/static/libfreetype.a ../lib/static/libhi_cc.a 
#HI_LIBS += ../lib/static/libhi_charsetMgr.a ../lib/static/libhi_common.a ../lib/static/libhigo.a 
#HI_LIBS += ../lib/static/libhigoadp.a ../lib/static/libhimount.a ../lib/static/libhi_msp.a ../lib/static/libhi_pes.a 
#HI_LIBS += ../lib/static/libhi_resampler.a  ../lib/static/libhi_subtitle.a ../lib/static/libhi_ttx.a 
#HI_LIBS += ../lib/static/libhi_vp.a ../lib/static/libjpeg6b.a ../lib/static/libjpeg.a 
#HI_LIBS += ../lib/static/libpng14.a ../lib/static/libpng.a ../lib/static/librtp.a ../lib/static/libsubdec.a ../lib/static/libz.a   
#HI_LIBS += ../lib/static/libhi_so.a ../lib/static/libplayer.a 
#HI_LIBDIR := -L$(WORK_PATH)lib/static  -L $(WORK_PATH)lib/share -L $(WORK_PATH)lib/extern 

#HI_LIBS := -lhi_common -lpng -lhigo -lhigoadp -ljpeg -lhi_resampler -lhi_msp -lz -lfreetype 
#HI_LIBS += -lhi_pes -lhi_subtitle -lhi_so -lhi_ttx -lhi_cc -lhi_vp -lrtp 
#HI_LIBS += -lhi_charsetMgr -lsubdec -lplayer -lHiDrmEngine

LDFLAGS := -L$(WORK_PATH)lib/static  -L $(WORK_PATH)lib/share -L $(WORK_PATH)lib/extern  -Wl,--start-group $(HI_LIBS) -Wl,--end-group

##############################################################################################
# target source
SRC  := $(wildcard *.c) 
OBJ  := $(SRC:%.c=%.o)
TARGET := $(OBJ:%.o=%)

##############################################################################################
%.o:%.h

.PHONY : clean all test ver

all: ver $(TARGET) 

$(TARGET):%:%.o $(COMM_OBJ)
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(LDFLAGS) 

ver:
	@echo "generate date and ver"
	@date +'#define SOFTVER_DATE "%Y:%m:%d"' > ../common/wv_ver.h
	@date +'#define SOFTVER_TIME "%T"' >> ../common/wv_ver.h
	@echo '#define SOFTVER_MAIN $(SOFT_VER)' >> ../common/wv_ver.h
	@echo  '#define SOFTVER_NAME $(SOFT_NAME)'  >> ../common/wv_ver.h
	
test:
	@echo $(COMM_OBJ)
	@echo $(TARGET)
	@echo $(CFLAGS)
	@echo $(LDFLAGS)
	@echo $(CC)
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(COMM_OBJ) 
