/* Copyright (C) Rong Tao @Sylincom Beijing, 2021年 02月 25日 星期四 12:53:26 CST. */
/**
 * 分别使用下面的编译命令，对比生成的汇编文件
 * gcc fcvt.c -S -mavx2
 * gcc fcvt.c -S -mavx
 * gcc fcvt.c -S -mavx2
 */

double fcvt(int i, float *fp, double *dp, long *lp)
{
	float f = *fp;
	double d = *dp;
	long l = *lp;

	*lp = (long)d;
	*fp = (float)i;
	*dp = (double)l;

	return (double)f;
}

