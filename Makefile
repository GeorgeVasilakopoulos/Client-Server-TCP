CC = gcc
EXEC = poller



DIRS = . structures
SOURCE_DIRS = $(foreach D, $(DIRS),$(wildcard source/$(D)))
INCLUDE_DIRS = $(foreach D, $(DIRS),$(wildcard include/$(D)))

DEPFLAGS = -MP -MD
CFLAGS = $(foreach D, $(INCLUDE_DIRS), -I$(D)) $(DEPFLAGS)

CFILES = $(foreach D,$(SOURCE_DIRS),$(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c,%.o,$(CFILES))
DEPFILES = $(patsubst %.c,%.d,$(CFILES))




all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ -lpthread


%.o: %.c
	$(CC) -g -Wall $(CFLAGS) -c -o $@ $<

clean:
	rm $(EXEC) $(OBJECTS) $(DEPFILES)

-include $(DEPFILES)




