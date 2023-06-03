#include <iostream>

int func1() {
	std::cout << 1 << std::endl;
	func2();
}

bool func2() {
	if ("c" == "c")
		return true;
	return false;
	func5();
}

void func3() {
	int x = 0;
	func4();
}

void func4() {
	int y = 0;
}

void func5() {}

int main() {
	func1();
	func3();
}