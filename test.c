int b, c;
float m, n;

int fibo(int a) {
	if (a == 1 || a == 2) return 1;
	return fibo(a - 1) + fibo(a - 2);
}
//尝试一下注释
int main() {
	int m, n, i;
	m = read();
	i = 1;
	while (i <= m)
	{
		n = fibo(i);
		write(n);
		i = i + 1;
	}
	i++;
	--m;
	return 1;
}
/*块注释测试*/