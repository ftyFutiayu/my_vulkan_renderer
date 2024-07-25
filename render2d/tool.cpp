#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

namespace render_2d {

    std::string ReadWholeFile(const std::string &filename) {
        std::ifstream input_file(filename, std::ios::binary);

        if (!input_file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            throw std::runtime_error("Failed to open file: " + filename);
        }

        input_file.seekg(0, std::ios::end);
        auto size = input_file.tellg();
        input_file.seekg(0, std::ios::beg); // 重置文件指针到开始位置

        std::string content;
        content.resize(static_cast<size_t>(size));
        input_file.read(&content[0], size);

        if (input_file.fail()) {
            std::cerr << "Failed to read file: " << filename << std::endl;
            throw std::runtime_error("Failed to read file: " + filename);
        }

        std::cout << "Success Read " << size << " bytes from file: " << filename << std::endl;
        return content;
    }
}