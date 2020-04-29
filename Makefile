C_FILES_TX :=	\
		./mqueue_test/tx.c

C_FILES_RX :=	\
		./mqueue_test/rx.c

RX_EXE :=	\
		./mqueue_test/rx

TX_EXE :=	\
		./mqueue_test/tx


ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -g -lrt -lpthread -lm
endif

all: _rx _tx

_rx: ${C_FILES_RX}
	$(CC) $? -o ${RX_EXE} $(LDFLAGS)

_tx: ${C_FILES_TX}
	$(CC) $? -o ${TX_EXE} $(LDFLAGS)

clean:
	rm ${RX_EXE} ${TX_EXE}