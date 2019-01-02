#ifndef EM_PORT_API
#	if defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#		else
#			define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#		endif
#	else
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype
#		else
#			define EM_PORT_API(rettype) rettype
#		endif
#	endif
#endif

double Rondom()
{
	static int seed = 1;
	static int const a = 16807, m = 2147483647, q = 127773, r = 2836;

	seed = a * (seed % q) - r * (seed / q);
	if (seed < 0) seed += m;
	return (double)seed / (double)m;
}

EM_PORT_API(double) Pi(int trials)
{
	double sum = 0.0;
	for (int j = 0; j < 100; j++)
	{
		int hits = 0;
		for (int i = 0; i < trials; i++)
		{
			double x = Rondom();
			double y = Rondom();
			if (x * x + y * y < 1.0)
				hits++;
		}
		sum += 4.0 * hits / trials;
		printf("Worker: Pi() round %d.\n", j + 1);
	}
	return sum / 100.0f;
}
