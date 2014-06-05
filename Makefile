.SUFFIXES:		.oServer .oClient

.SECONDARY:

DESTDIR			?= /usr/local/include/

Examples		= $(shell for D in `find examples/ -mindepth 1 -type d -print`; do echo "$$D/$1"; done)
GetExampleFeatures	= $(shell if test -f $1/ExampleFeatures.cpp; then echo "$1/ExampleFeatures.$2"; fi)
All			= $(call Examples,Server) $(call Examples,Client)
Objs			= $(shell find examples/ -name "*.o" -o -name "*.oServer" -o -name "*.oClient")

ProvideObj		= g++ $(CFLAGS) -o $@ -c $<
LinkBinary		= g++ -o $@ $^ -lboost_system -lboost_thread -lpthread -lboost_program_options

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
			include/ProtoTransformer/detail/Session.tcc \
			include/ProtoTransformer/detail/Session.hpp \
		include/ProtoTransformer/detail/filteringAdapter.hpp \
			$$(@D)/Proto.hpp \
			$$(shell ls include/ProtoTransformer/detail/SessionManagers/*) \
			$$(call GetExampleFeatures,$$(@D),hpp)
	$(ProvideObj)

%.oClient:		%.cpp \
			include/ProtoTransformer/Client.hpp \
			include/ProtoTransformer/detail/Client.tcc \
			$$(@D)/Proto.hpp \
		$$(call GetExampleFeatures,$$(@D),hpp)
	$(ProvideObj)

%/Server:		$$(@D)/Server.oServer \
			$$(call GetExampleFeatures,$$(@D),o)
	$(LinkBinary)

%/Client:		$$(@D)/Client.oClient \
		$$(call GetExampleFeatures,$$(@D),o)
	$(LinkBinary)

clean:
	rm $(Objs) $(All)
