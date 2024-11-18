CC = gcc
CFLAGS += -Wall -g -ggdb

SRCDIR = .

OBJS = \
	$(SRCDIR)/pd_cmd.o \
	$(SRCDIR)/pd_event.o \
	$(SRCDIR)/pd_hash.o \
	$(SRCDIR)/pd_hdlr.o \
	$(SRCDIR)/pd_main.o \
	$(SRCDIR)/pd_net.o \
	$(SRCDIR)/pd_server.o \
	$(SRCDIR)/pd_smbuf.o \
	$(SRCDIR)/pd_tree.o

all: pdkv

clean:
	rm -rf pdkv *.o

pdkv: $(OBJS)
	$(CC) $(CFLAGS) -o pdkv $(OBJS)

$(SRCDIR)/pd_cmd.o: $(SRCDIR)/pd_cmd.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_event.o: $(SRCDIR)/pd_event.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_hash.o: $(SRCDIR)/pd_hash.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_hdlr.o: $(SRCDIR)/pd_hdlr.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_main.o: $(SRCDIR)/pd_main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_net.o: $(SRCDIR)/pd_net.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_server.o: $(SRCDIR)/pd_server.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_smbuf.o: $(SRCDIR)/pd_smbuf.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/pd_tree.o: $(SRCDIR)/pd_tree.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean
