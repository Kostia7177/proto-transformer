.SUFFIXES:	.oServer .oClient

.SECONDARY:

DESTDIR	?= /usr/local/include/

Examples		= $(shell for D in `find examples/ -mindepth 1 -type d -print`; do echo "$$D/$1"; done)
GetExampleFeatures	= $(shell if test -f $1/ExampleFeatures.cpp; then echo "$1/ExampleFeatures.$2"; fi)
GetExampleDeps		= $(shell if test -f $1/ExampleFeatures.hpp; \
				  then \
					for File in `grep -E "^\#include \"" $1/ExampleFeatures.hpp \
							| awk -F'"' '{ print $$2; }'`; \
					do \
						echo $1/$$File; \
					done; \
				  fi)
All			= $(call Examples,Server) $(call Examples,Client)
Objs			= $(shell find examples/ -name "*.o" -o -name "*.oServer" -o -name "*.oClient")

ProvideObj		= g++ $(CFLAGS) -o $@ -c $<
LinkBinary		= g++ -o $@ $^ -lboost_system -lboost_chrono -lboost_thread -lpthread -lboost_program_options -latomic

all: $(All)
install:
	cp -R include/* $(DESTDIR)

CFLAGS += -g -std=c++11 -Iinclude -Wno-varargs

.cpp.o:
	$(ProvideObj)

.SECONDEXPANSION:

%.oServer:	%.cpp \
		include/ProtoTransformer/Server.hpp \
		include/ProtoTransformer/detail/Server.tcc \
		include/ProtoTransformer/detail/Session/* \
		include/TricksAndThings/ParamPackManip/* \
		include/TricksAndThings/ParamPackManip/Binders/* \
		$$(@D)/Proto.hpp \
		$$(shell ls include/ProtoTransformer/detail/SessionManagers/*) \
		$$(shell ls include/ProtoTransformer/detail/Wrappers/*) \
		$$(call GetExampleFeatures,$$(@D),hpp) \
		$$(call GetExampleDeps,$$(@D))
	$(ProvideObj)

%.oClient:	%.cpp \
		include/ProtoTransformer/Client.hpp \
		include/ProtoTransformer/detail/Client.tcc \
		include/TricksAndThings/ParamPackManip/* \
		include/TricksAndThings/ParamPackManip/Binders/* \
		$$(@D)/Proto.hpp \
		$$(call GetExampleFeatures,$$(@D),hpp) \
		$$(call GetExampleDeps,$$(@D))
	$(ProvideObj)

%/Server:	$$(@D)/Server.oServer \
		$$(call GetExampleFeatures,$$(@D),o)
	$(LinkBinary)

%/Client:	$$(@D)/Client.oClient \
		$$(call GetExampleFeatures,$$(@D),o)
	$(LinkBinary)

clean:
	rm $(Objs) $(All)
