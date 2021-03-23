    /**
     * Return the current value of the fine-grain CPU cycle counter
     * (accessed via the RDTSC instruction).
     */
    static __inline __attribute__((always_inline))
    uint64_t
    rdtsc()
    {
#if TESTING
        if (mockTscValue)
            return mockTscValue;
#endif
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
//        __asm__ __volatile__("rdtscp" : "=a" (lo), "=d" (hi) : : "%rcx");
        return (((uint64_t)hi << 32) | lo);
    }
