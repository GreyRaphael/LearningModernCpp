#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void visit_directory_1(fs::path const& dir) {
    if (fs::exists(dir) && fs::is_directory(dir)) {
        for (auto const& entry : fs::directory_iterator(dir)) {
            auto filename = entry.path().filename();
            if (fs::is_directory(entry.status()))
                std::cout << "[+]" << filename << '\n';
            else if (fs::is_symlink(entry.status()))
                std::cout << "[>]" << filename << '\n';
            else if (fs::is_regular_file(entry.status()))
                std::cout << "   " << filename << '\n';
            else
                std::cout << "[?]" << filename << '\n';
        }
    } else {
        std::cout << "Directory " << std::quoted(dir.string()) << " does not exit" << '\n';
    }
}

void visit_directory_2(fs::path const& dir) {
    if (fs::exists(dir) && fs::is_directory(dir)) {
        for (auto const& entry : fs::recursive_directory_iterator(dir)) {
            auto filename = entry.path().filename();
            if (fs::is_directory(entry.status()))
                std::cout << "[+]" << filename << '\n';
            else if (fs::is_symlink(entry.status()))
                std::cout << "[>]" << filename << '\n';
            else if (fs::is_regular_file(entry.status()))
                std::cout << "   " << filename << '\n';
            else
                std::cout << "[?]" << filename << '\n';
        }
    } else {
        std::cout << "Directory " << std::quoted(dir.string()) << " does not exit" << '\n';
    }
}

void visit_directory(fs::path const& dir, bool const recursive = false, unsigned int const level = 0) {
    if (fs::exists(dir) && fs::is_directory(dir)) {
        auto lead = std::string(level * 3, ' ');
        for (auto const& entry : fs::directory_iterator(dir)) {
            auto filename = entry.path().filename();
            if (fs::is_directory(entry.status())) {
                std::cout << lead << "[+]" << filename << '\n';
                if (recursive)
                    visit_directory(entry, recursive, level + 1);
            } else if (fs::is_symlink(entry.status()))
                std::cout << lead << "[>]" << filename << '\n';
            else if (fs::is_regular_file(entry.status()))
                std::cout << lead << " " << filename << '\n';
            else
                std::cout << lead << "[?]" << filename << '\n';
        }
    }
}

std::uintmax_t dir_size(fs::path const& path) {
    auto size = 0;

    if (fs::exists(path) && fs::is_directory(path)) {
        for (auto const& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status()) || fs::is_symlink(entry.status())) {
                [[maybe_unused]] auto err = std::error_code{};
                auto filesize = fs::file_size(entry);
                if (filesize != static_cast<uintmax_t>(-1))
                    size += filesize;
            }
        }
    }

    return size;
}

int main() {
    auto path = fs::current_path() / "test";
    visit_directory_1(path);
    visit_directory_2(path);
    visit_directory(path);
    visit_directory(path, true);
    std::cout << dir_size(path) << '\n';
}