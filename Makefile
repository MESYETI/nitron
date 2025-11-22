SOURCES := $(wildcard source/*.c) $(wildcard source/**/*.c)
OBJECTS := $(patsubst source/%.c,bin/%.o,$(SOURCES))
OUT     := nitron
LD      := $(CC)

override CFLAGS += -std=c99 -Wall -Wextra -Wuninitialized -Wundef -pedantic -Ilib -m32
override LDLIBS += -lm -m32

override CFLAGS += -DAE_BACKEND_GL11 -DAE_AUDIO_PSRC

ifeq ($(BUILD),release)
	override CFLAGS += -O3
	#override CPPFLAGS += -NDEBUG
else
	override CFLAGS += -Og -g
	ifeq ($(ASAN),y)
		override CFLAGS += -fno-omit-frame-pointer -fsanitize=address
		override LDFLAGS += -fsanitize=address
	endif
endif

.SECONDEXPANSION:

deps.filter := %.c %.h
deps.option := -MM
define deps
$$(filter $$(deps.filter),,$$(shell $(CC) $(CFLAGS) $(CPPFLAGS) -E $(deps.option) $(1)))
endef

all: $(OUT)
	@:

run: $(OUT)
	'$(dir $<)$(notdir $<)' $(RUNFLAGS)

$(OUT): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/:
	mkdir -p bin

bin/fs:
	mkdir -p bin/fs

bin/disk:
	mkdir -p bin/disk

bin/%.o: source/%.c $(call deps,source/%.c) romfs.h | bin/ bin/fs bin/disk
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

clean:
	rm -r bin

distclean: clean
	rm $(OUT)

.PHONY: all run clean distclean
