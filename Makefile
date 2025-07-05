
TARGET := sms_libretro.so
SHARED := -shared
CFLAGS += -g -O3 -fPIC -flto
LDFLAGS += -flto

OBJ = sms.o z80.o mem.o vdp.o psg.o

.DEFAULT_GOAL := $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) $(SHARED) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
