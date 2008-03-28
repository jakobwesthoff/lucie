CC= /usr/bin/gcc
STRIP= /usr/bin/strip
MAKE= /usr/bin/make

CFLAGS= -Os -DDEBUG

LUA_DIR= lib/lua-5.1.2
LUA_A= $(LUA_DIR)/src/liblua.a
LUA_INCLUDE= $(LUA_DIR)/src
LUA_LIB= $(LUA_DIR)/src

LUCIE_C= src/lucie.c src/inireader.c src/btree.c src/reader.c src/superglobals.c src/util.c src/lucieinfo.c
LUCIE_H= src/lucie.h src/inireader.h src/btree.h src/reader.h src/superglobals.h src/util.h src/lucieinfo.h
LUCIE= src/lucie

EXTENSIONS= src/ext/core.so

.PHONY: all clean

all: $(LUCIE) $(EXT_CORE)

clean:
	cd "$(LUA_DIR)" && $(MAKE) clean
	rm -f $(LUCIE) $(EXTENSIONS)
	
$(LUA_A):
	cd "$(LUA_DIR)/src" && $(MAKE) a MYCFLAGS="-DLUA_USE_LINUX $(CFLAGS)" MYLIBS="-Wl,-E -ldl"

$(LUCIE): $(LUCIE_C) $(LUCIE_H) $(LUA_A) $(EXTENSIONS)
	$(CC) -Wall -Wl,-E -I"$(LUA_INCLUDE)" -L"$(LUA_LIB)" -o $(LUCIE) $(LUCIE_C) -lm -ldl -llua $(CFLAGS)
	$(STRIP) $(LUCIE)

$(EXTENSIONS): $(EXTENSIONS:.so=.c) $(LUCIE_H) 
	$(CC) -Wall -shared -Wl,-soname,$(@).1 -fPIC -I"$(LUA_INCLUDE)" -L"$(LUA_LIB)" -o $(@) $(@:.so=.c) $(CFLAGS)
	$(STRIP) $(@)
