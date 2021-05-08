/**
 * https://zzqcn.github.io/perf/simd_demo.html
 */

#ifdef ORIGINAL1
//正常代码如下, 直接使用基本算法计算2个矩阵a和b, 结果放在c中:
void multiply(void) {
    unsigned i;
    for(i=0; i<N; i++) {
        c[i] = a[i] * b[i];
    }
}

#elif defined(ORIGINAL2)

//一次循环计算4次
//可以将正常代码改为一次循环内计算4次乘法, 某种情况下可以提升性能:
void multiply(void) {
    unsigned i;
    for(i=0; i<(N & ((~(unsigned)0x3))); i+=4) {
        c[i]   = a[i]   * b[i];
        c[i+1] = a[i+1] * b[i+1];
        c[i+2] = a[i+2] * b[i+2];
        c[i+3] = a[i+3] * b[i+3];
    }
    for(; i<N; i++) {
        c[i] = a[i] * b[i];
    }
}

#elif defined(SSE)
//使用SSE指令
//Intel SSE指令通过128bit位宽的专用寄存器, 支持一次操作128bit数据. float是单精度浮点数, 占32bit, 那么可以使用一条SSE指令一次计算4个float数:

void multiply(void) {
    unsigned i;
    __m128 A, B, C;

    for(i=0; i<(N & ((~(unsigned)0x3))); i+=4) {
        A = _mm_load_ps(&a[i]);
        B = _mm_load_ps(&b[i]);
        C = _mm_mul_ps(A, B);
        _mm_store_ps(&c[i], C);
    }
    for(; i<N; i++) {
        c[i] = a[i] * b[i];
    }
}
//注意这些SSE指令要求参数中的内存地址必须对齐于16字节边界, 所以可以用以下函数分配内存:

a = (float*) _mm_malloc(N*sizeof(float), 16);
b = (float*) _mm_malloc(N*sizeof(float), 16);
c = (float*) _mm_malloc(N*sizeof(float), 16);

#elif defined(AVX)

void multiply(void) {
    unsigned i;
    __m256 A, B, C;

    for(i=0; i<(N & ((~(unsigned)0x7))); i+=8) {
        A = _mm256_load_ps(&a[i]);
        B = _mm256_load_ps(&b[i]);
        C = _mm256_mul_ps(A, B);
        _mm256_store_ps(&c[i], C);
    }
    for(; i<N; i++) {
        c[i] = a[i] * b[i];
    }
}
//AVX指令要求内存地址对齐于32字节边界, 所以内存分配代码改为:

a = (float*) _mm_malloc(N*sizeof(float), 32);
b = (float*) _mm_malloc(N*sizeof(float), 32);
c = (float*) _mm_malloc(N*sizeof(float), 32);
#endif
