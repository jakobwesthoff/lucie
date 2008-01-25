CC= /usr/bin/gcc
STRIP= /usr/bin/strip
MAKE= /usr/bin/make


LUA_DIR= lib/lua-5.1.2
LUA_A= $(LUA_DIR)/src/liblua.a
LUA_INCLUDE= $(LUA_DIR)/src
LUA_LIB= $(LUA_DIR)/src

LUCIE_C= src/lucie.c
LUCIE_H= src/lucie.h
LUCIE= src/lucie

EXT_CORE_C= src/ext/core.c
EXT_CORE_H= src/ext/core.h
EXT_CORE= src/ext/core.so

.PHONY: all clean

all: $(LUCIE) $(EXT_CORE)

clean:
	cd "$(LUA_DIR)" && $(MAKE) clean
	rm -f $(LUCIE) $(EXT_CORE)
	
$(LUA_A):
	cd "$(LUA_DIR)/src" && $(MAKE) a MYCFLAGS=-DLUA_USE_LINUX MYLIBS="-Wl,-E -ldl"

$(LUCIE): $(LUCIE_C) $(LUCIE_H) $(LUA_A)
	   $(CC) -Os -Wall -DLUA_USE_LINUX -Wl,-E -I"$(LUA_INCLUDE)" -L"$(LUA_LIB)" -o $(LUCIE) $(LUCIE_C) -lm -ldl -llua 
	   $(STRIP) $(LUCIE)

