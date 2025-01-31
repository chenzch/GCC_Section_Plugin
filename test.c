#pragma GCC section text ".sec1"
#pragma GCC section data ".data1"
#pragma GCC section bss ".bss1"
#pragma GCC section rodata ".rodata1"

int fun1(int a, int b) {

	const int base1 = 3; // ? put into data section?

	return base1 + a - b;
}

static int ga;
static int gb = 4;
static const int gc = 3;

#pragma GCC section text
#pragma GCC section data
#pragma GCC section bss
#pragma GCC section rodata

int fun2(int a, int b) {
	const int base2 = 3;
	return base2 + a + b;
}

static int gx;
static int gy = 4;
static const int gz = 3;

int main() {
	ga = fun1(gb, gc);
	gx = fun2(gy, gz);
	return 0;
}

void exit() {}
