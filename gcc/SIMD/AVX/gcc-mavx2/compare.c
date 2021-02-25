/**
 * 分别使用下面的编译命令，对比生成的汇编文件
 * gcc fcvt.c -S -mavx2
 * gcc fcvt.c -S -mavx
 * gcc fcvt.c -S -mavx2
 */

int compare(float f1, float f2)
{
	if (f1 > f2) return 0;
	else return 1;
}

