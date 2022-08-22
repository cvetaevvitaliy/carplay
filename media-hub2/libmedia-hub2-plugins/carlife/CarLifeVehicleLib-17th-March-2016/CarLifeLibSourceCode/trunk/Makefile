#Author:
#	Liu CaiQuan
#Date:
#	7th Semptember 2015
#CarLife Protocol version:
#	V0.15
#
#rule for Linux
#
#                   Copyright (C) Under BaiDu, Inc.

#TOPDIR:=~/EddyWorkSpace/CarLifeLibSourceCode/trunk
TOPDIR:=$(shell pwd)
export TOPDIR

CROSS_COMPILE=

CFLAGS:=-g -Wall -W -fPIC
CFLAGS+=-I protobuf/cc/include
CFLAGS+=-I $(TOPDIR)/include
CFLAGS+=-I $(TOPDIR)/protobuf/cc/include
CFLAGS+=-I $(SDKTARGETSYSROOT)/usr/include
CFLAGS+=-I $(SDKTARGETSYSROOT)/usr/include/google/protobuf
CFLAGS+=-I $(SDKTARGETSYSROOT)/usr/include/google/protobuf/io
CFLAGS+=-I $(SDKTARGETSYSROOT)/usr/include/google/protobuf/stubs
CFLAGS+=-I $(SDKTARGETSYSROOT)/usr/include
CFLAGS+=-I $(SDKTARGETSYSROOT)/usr/include/libxml2

#LFLAGS:=-fPIC -shared
LFLAGS+=-L$(SDKTARGETSYSROOT)/usr/lib -lprotobuf

export CFLAGS LFLAGS

OBJDIR=obj

#obj-y+=CarLifeLibTest.o
obj-y+=core/
obj-y+=libwrapper/
obj-y+=modules/
obj-y+=protobuf/
obj-y+=utility/

TARGET-LIB=libcarlifevehicle.so
#TARGET-APP=libtest

all: carlifelib
#libtest

carlifelib: 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CC) -shared -o $(TARGET-LIB) built-in.o
	cp $(TARGET-LIB) $(OBJDIR)	

#libtest:CarLifeLibTest.cpp
#	$(CC) -o $(TARGET-APP) $< -Iinclude -L./ -lcarlifevehicle -lpthread $(LFLAGS)
#	$(CP) $(TARGET-APP) $(OBJDIR)

protoBuf:
	rm -f ./protobuf/pbGen/*
	protoc --proto_path=./protobuf/proto/ --cpp_out=./protobuf/pbGen ./protobuf/proto/*.proto
	rm -f ./protobuf/cc/src/*.cc
	rm -f ./protobuf/cc/include/*.h
	mv ./protobuf/pbGen/*.cc ./protobuf/cc/src/
	mv ./protobuf/pbGen/*.h ./protobuf/cc/include/
install:
	cp $(TARGET-LIB) $(DESTDIR)/usr/lib/
	cp libcarlifevehicle.pc $(DESTDIR)/usr/lib/pkgconfig/
	cp ./include/CCarLifeLibWrapper.h $(DESTDIR)/usr/include/

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET-LIB)
	rm -f $(TARGET-APP)
	rm -f $(OBJDIR)/*

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET-LIB)
	rm -f $(TARGET-APP)
	rm -f $(OBJDIR)/*
