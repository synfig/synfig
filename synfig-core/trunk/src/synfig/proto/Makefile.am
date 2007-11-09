MAINTAINERCLEANFILES=nodebase.h
EXTRA_DIST=nodebase.h nodebase.px proto.m4

M4=m4

PROTO_PP=$(M4) proto.m4

FILES=nodebase.h

all: $(FILES)

clean:
	$(RM) $(FILES)

SUFFIXES=.px .h

.px.h: proto.m4
	$(PROTO_PP) $< > $@
