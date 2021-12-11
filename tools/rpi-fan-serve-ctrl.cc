#include <argparse/argparse.hpp>

int main(int argc, const char* argv[]) {
	argparse::ArgumentParser program("rpi-fan-serve-ctrl");
	program.add_argument("-c", "--set-cache-life-expectancy")
		.help("The new cache life expectancy in seconds");
	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		return 1;
	}
	std::string cleStr("");
	{
		if (auto fn = program.present("-c"); fn) {
			cleStr = *fn;
		}
	}
	size_t cle = 0;
	try {
		cle = std::stoul(cleStr, nullptr, 10);
	} catch (std::invalid_argument&) {
		std::cerr << "[error] invalid port: `" << cleStr << "`\n";
		return 1;
	} catch (std::out_of_range&) {
		std::cerr << "[error] cache life expectancy should be"
				  << " in range [100;100'000] `" << cleStr << "`\n";
		return 1;
	}
	if (cle < 100 || cle > 100'000) {
		std::cerr << "[error] cache life expectancy should be"
				  << " in range [100;100'000] `" << cleStr << "`\n";
		return 1;
	}

    return 0;
}
