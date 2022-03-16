CC = mpicc
CFLAGS += -g -Wall -std=c99 -Iinclude
SRCDIR = src
BUILDDIR = build

SRCS=$(shell find $(SRCDIR) -name *.c)
OBJS=$(SRCS:%.c=$(BUILDDIR)/%.o)
DEPS=$(OBJS:%.o=%.d)

$(BUILDDIR)/main : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

.PHONY : clean
clean :
	rm -rf $(BUILDDIR)

run :
	mpiexec $(BUILDDIR)/main
