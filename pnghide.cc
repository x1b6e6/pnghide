#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <docopt/docopt.h>
#include <png++/image.hpp>
#include <png++/rgb_pixel.hpp>

#ifndef PROJECT_VERSION
#warning PROJECT_VERSION not found
#define PROJECT_VERSION "(unknown)"
#endif

auto get_version() {
	return "pnghide " PROJECT_VERSION;
}

auto get_usage() {
	return get_version() + std::string(R"(
Usage:
    pnghide hide [--input=FILE] [--output=FILE] [--not-fill-null] <image>
    pnghide unhide [--output=FILE] [--limit=LIMIT] <image>

Options:
    -h, --help            print this message
    -v, --version         print version and exit
    -i, --input=FILE      set input file [default: /dev/stdin]
    -o, --output=FILE     set output file [default: /dev/stdout]
    -l, --limit=LIMIT     set limit reading bytes [default: -1]
    -n, --not-fill-null   do not write zeroes after input
)");
}
template <typename TPixel>
void proccess_hide(png::image<TPixel>& image,
				   std::istream& input,
				   std::ostream& output,
				   bool not_fill_null) {
	auto width = image.get_width();
	auto height = image.get_height();

	int row = 0;
	int cell = 0;

	uint16_t buf = 0;
	uint8_t buf_size = 0;

	size_t writed = 0;

	auto proccess_buf = [&] {
		while (buf_size >= 3) {
			char current_bits = (buf >> (buf_size - 3)) & 0b111;
			auto pixel = image.get_pixel(cell, row);

			pixel.red &= ~1;
			pixel.green &= ~1;
			pixel.blue &= ~1;

			pixel.red |= (current_bits >> 2) & 1;
			pixel.green |= (current_bits >> 1) & 1;
			pixel.blue |= (current_bits >> 0) & 1;

			buf_size -= 3;

			image.set_pixel(cell, row, pixel);

			++cell;
			if (cell == width) {
				++row;
				cell = 0;
				if (row == height) {
					buf_size = 0;
					throw std::runtime_error(
						"data size is bigger then image can store\n");
				}
			}
		}
	};

	try {
		while (std::cin.good()) {
			uint8_t buf_in;
			std::cin.read((char*)&buf_in, 1);
			if (std::cin.gcount() == 0)
				break;
			++writed;
			buf <<= 8;
			buf |= buf_in;
			buf_size += 8;
			proccess_buf();
		}

		if (buf_size != 0) {
			auto mod = buf_size % 3;
			if (mod != 0) {
				buf <<= 1;
				++buf_size;
				if (mod == 2) {
					buf <<= 1;
					++buf_size;
				}
			}
			proccess_buf();
		}
	} catch (const std::runtime_error& e) {
		std::cerr << "Error: size of input is very large\n";
	}

	if (not_fill_null == false) {
		try {
			for (;;) {
				buf = 0;
				buf_size = 8;
				proccess_buf();
			}
		} catch (const std::runtime_error&) {
		}
	}

	std::cerr << "Writed " << writed << " bytes\n";
}

template <typename TPixel>
void proccess_unhide(const png::image<TPixel>& image,
					 std::ostream& output,
					 long limit) {
	auto width = image.get_width();
	auto height = image.get_height();

	uint16_t buf = 0;
	uint8_t buf_size = 0;
	size_t readed = 0;
	for (size_t row = 0; row < height; ++row) {
		for (size_t cell = 0; cell < width; ++cell) {
			buf <<= 3;
			buf_size += 3;
			auto pixel = image.get_pixel(cell, row);
			buf |= (pixel.red & 1) << 2;
			buf |= (pixel.green & 1) << 1;
			buf |= (pixel.blue & 1) << 0;

			if (buf_size > 8) {
				uint8_t out_char = (buf >> (buf_size - 8)) & 0xff;
				std::cout.write((const char*)&out_char, 1);
				if (limit != -1) {
					++readed;
					if (readed >= limit)
						return;
				}

				buf_size -= 8;
			}
		}
	}
}

int main(int argc, const char** argv) {
	auto args = docopt::docopt(get_usage(), {argv + 1, argv + argc}, true,
							   get_version());

	auto hide = args.at("hide").asBool();
	auto unhide = args.at("unhide").asBool();
	auto image_file = args.at("<image>").asString();
	auto input = args.at("--input").asString();
	auto output = args.at("--output").asString();
	auto limit = args.at("--limit").asLong();
	auto not_fill_null = args.at("--not-fill-null").asBool();

	std::ifstream fin;
	std::ofstream fout;

	png::image<png::rgb_pixel> image(image_file);

	if (output != "/dev/stdout") {
		fout.open(output);
		std::cout.rdbuf(fout.rdbuf());
	}

	if (hide) {
		if (input != "/dev/stdin") {
			fin.open(input);
			std::cin.rdbuf(fin.rdbuf());
		}
		proccess_hide(image, std::cin, std::cout, not_fill_null);
		image.write_stream(std::cout);
	} else if (unhide) {
		proccess_unhide(image, std::cout, limit);
	}

	return 0;
}

// vim: set ts=4 sw=4 :
