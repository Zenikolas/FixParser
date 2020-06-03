#include <FixParser.h>

static void show_usage(const std::string &name)
{
    std::cerr << "Usage: " << name << " -f data/fix_test_data.txt\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-f,--file\t\tFile to parse"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        show_usage(argv[0]);
        return 1;
    }

    std::string filename{};

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-f") || (arg == "--file")) {
            if (i + 1 < argc) {
                filename = argv[++i];
            } else {
                std::cerr << "--file option requires one argument." << std::endl;
                return 1;
            }
        }
    }

    const size_t orderBookDepth = 5;
    fix_parser::FixParser parser{orderBookDepth};

    try {
        parser.process(filename);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << parser.market_data_to_s(orderBookDepth) << std::endl;

    return 0;
}
