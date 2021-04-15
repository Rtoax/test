

#define __rte_experimental \
__attribute__((section(".text.experimental")))



#define __rte_internal \
__attribute__((section(".text.internal")))


#define __rte_always_inline inline __attribute__((always_inline))

