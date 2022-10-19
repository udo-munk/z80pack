// Main program when using the C++ frontpanel library

extern "C" int sim_main(int, char *[]);

int main(int argc, char *argv[])
{
	return sim_main(argc, argv);
}
