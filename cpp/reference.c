#include <iostream>

int add(int &v) {
	v++;
	return v;
}

int main() {
	int a;
	add(a);
	std::cout<<a<<std::endl;
}
