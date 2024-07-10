
TARGET := sms_libretro.so
SHARED := -shared
CFLAGS += -O3 -fPIC

OBJ = sms.o z80.o mem.o vdp.o

.DEFAULT_GOAL := $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(LD) $(SHARED) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
