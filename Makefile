CC = gcc



DIRS = . structures votersRecord
SOURCE_DIRS = $(foreach D, $(DIRS),$(wildcard server/source/$(D)))
INCLUDE_DIRS = $(foreach D, $(DIRS),$(wildcard server/include/$(D)))

DEPFLAGS = -MP -MD
CFLAGS = $(foreach D, $(INCLUDE_DIRS), -I$(D)) $(DEPFLAGS)

CFILES = $(foreach D,$(SOURCE_DIRS),$(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c,%.o,$(CFILES))
DEPFILES = $(patsubst %.c,%.d,$(CFILES))


all: poller pollSwayer

pollSwayer: client/pollSwayer.c 
	$(CC) client/pollSwayer.c -lpthread -o pollSwayer 

poller: $(OBJECTS)
	$(CC) -o $@ $^ -lpthread


%.o: %.c
	$(CC) -g -Wall $(CFLAGS) -c -o $@ $<

clean:
	rm poller pollSwayer $(OBJECTS) $(DEPFILES) *.txt

-include $(DEPFILES)




