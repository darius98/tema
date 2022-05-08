#pragma once

#include <filesystem>
#include <fstream>
#include <random>

namespace util {

struct temp_file {
    std::filesystem::path file_path;
    std::ofstream writer;

    static std::filesystem::path make_name(std::string_view extension = "") {
        static std::random_device rng;
        static const auto temp_dir_path = std::filesystem::temp_directory_path();
        std::uniform_int_distribution<char> letter_distribution{'a', 'z'};
        std::string stem(16, ' ');
        for (std::size_t i = 0; i < 16; i++) {
            stem[i] = letter_distribution(rng);
        }
        std::filesystem::path file_name = temp_dir_path / stem;
        file_name.replace_extension(extension);
        return file_name;
    }

    explicit temp_file(std::string_view extension = "")
        : file_path{make_name(extension)}, writer{file_path} {
    }

    ~temp_file() {
        std::filesystem::remove(file_path);
    }
};

}  // namespace util
