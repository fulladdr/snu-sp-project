targets := $(patsubst %.c,%,$(wildcard *.c))

% : %.c
	$(CC) -o $@ -O0 $<

all: $(targets)

clean:
	rm -rf $(targets)
