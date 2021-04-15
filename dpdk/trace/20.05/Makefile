

all:
	@echo -e "Compile demostration."
	@gcc eal_common_errno.c \
		eal_common_log.c \
		eal_common_string_fns.c \
		eal_common_trace.c \
		eal_common_trace_ctf.c \
		eal_common_trace_point.c \
		eal_common_trace_utils.c \
		eal_common_uuid.c \
		eal_thread.c \
		test-0.c -I. -pthread -g

clean:
	@echo "Clean"
	@rm -f a.out
	@rm -rf /home/rongtao/dpdk-traces/*
