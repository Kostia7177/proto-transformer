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
Headers			= $(shell find $1 -name "*.hpp" -o -name "*.tcc")

ProvideObj		= g++ $(CFLAGS) -o $@ -c $<
#ProvideObj += -fsanitize=address
#ProvideObj += -pg
LinkBinary		= g++ -o $@ $^ -lboost_system -lboost_chrono -lboost_thread -lpthread -lboost_program_options -latomic
#LinkBinary += -fsanitize=address
#LinkBinary += -pg

all: $(All)
install:
	cp -R include/* $(DESTDIR)

CFLAGS += -g -std=c++11 -Iinclude -Wno-varargs

.cpp.o:
	$(ProvideObj)

.SECONDEXPANSION:

%.oServer:	%.cpp \
		$$(call Headers,include/TricksAndThings/ParamPackManip/) \
		$$(call Headers,include/TricksAndThings/SignatureManip/) \
		$$(call Headers,include/TricksAndThings/ThreadPool/) \
		$$(call Headers,include/TricksAndThings/LockFree/) \
		$$(call Headers,include/TricksAndThings/Tools/) \
		include/ProtoTransformer/Server.hpp \
		include/ProtoTransformer/detail/Server.tcc \
		include/ProtoTransformer/detail/Session/* \
		$$(@D)/Proto.hpp \
		include/ProtoTransformer/detail/SessionManagers/* \
		include/ProtoTransformer/detail/Wrappers/* \
		$$(call GetExampleFeatures,$$(@D),hpp) \
		$$(call GetExampleDeps,$$(@D))
	$(ProvideObj)

%.oClient:	%.cpp \
		$$(call Headers,include/TricksAndThings/ParamPackManip/) \
		$$(call Headers,include/TricksAndThings/SignatureManip/) \
		$$(call Headers,include/TricksAndThings/Tools/) \
		include/ProtoTransformer/Client.hpp \
		include/ProtoTransformer/detail/Client.tcc \
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
