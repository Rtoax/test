#include <stdio.h>
struct rb_node {    /* 节点 */
	unsigned long  __rb_parent_color;   /* 父节点的颜色 */
	struct rb_node *rb_right;   /* 右孩子 */
	struct rb_node *rb_left;    /* 左孩子 */
} __attribute__((aligned(sizeof(long))));   /* 对齐方式 */

struct rb_node_noaligned {    /* 节点 */
	unsigned long  __rb_parent_color;   /* 父节点的颜色 */
	struct rb_node *rb_right;   /* 右孩子 */
	struct rb_node *rb_left;    /* 左孩子 */
};   /* 对齐方式 */

int main() {
	struct rb_node rb1, rb2;
	struct rb_node_noaligned rb3, rb4;

	printf("%p\n", &rb1);
	printf("%p\n", &rb2);
	printf("%p\n", &rb3);
	printf("%p\n", &rb4);
}
