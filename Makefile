
TARGET := sms_libretro.so
SHARED := -shared
CFLAGS += -g -O3 -fPIC -flto
LDFLAGS += -flto

OBJ = sms.o z80.o mem.o vdp.o psg.o
$(TARGET): $(OBJ) ; $(CC) $(SHARED) -o $@ $^ $(LDFLAGS)
clean:            ; rm -f $(OBJ) $(TARGET)
