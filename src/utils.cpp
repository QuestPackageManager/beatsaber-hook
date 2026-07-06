#include "utils.hpp"

#include <sys/stat.h>

void i2c::reset_ss(std::stringstream& ss) {
    ss.str("");
    ss.clear();  // Clear state flags.
}

void i2c::tabs(std::ostream& os, int tabs, int spacesPerTab) {
    for (int i = 0; i < tabs * spacesPerTab; i++) {
        os << " ";
    }
}

int mkpath(std::string_view file_path) {
    std::string command = fmt::format("mkdir -p -m +rw {}", file_path);
    return system(command.c_str());
}

std::string readfile(std::string_view filename) {
    std::ifstream t(filename.data());
    if (!t.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

std::vector<char> readbytes(std::string_view filename) {
    std::ifstream infile(filename.data(), std::ios_base::binary);
    if (!infile.is_open()) {
        return {};
    }
    return std::vector<char>(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());
}

bool writefile(std::string_view filename, std::string_view text) {
    std::ofstream t(filename.data());
    if (t.is_open()) {
        t << text;
        return true;
    }
    return false;
}

bool deletefile(std::string_view filename) {
    if (fileexists(filename)) {
        return remove(filename.data()) == 0;
    }
    return false;
}

bool fileexists(std::string_view filename) {
    return access(filename.data(), W_OK | R_OK) != -1;
}

bool direxists(std::string_view dirname) {
    struct stat info;
    if (stat(dirname.data(), &info) != 0) {
        return false;
    }
    return (bool) info.st_mode & S_IFDIR;
}

static std::optional<std::string> data_dir;
static std::optional<std::string> config_dir;

std::string get_data_dir(std::string_view id) {
    if (!data_dir) {
        data_dir = fmt::format(PERSISTENT_DIR, modloader_get_application_id());
    }
    if (!direxists(*data_dir)) {
        mkpath(*data_dir);
    }
    return fmt::format("{}{}/", *data_dir, id);
}

std::string get_config_path(std::string_view id) {
    if (!config_dir) {
        config_dir = fmt::format(CONFIG_PATH_FORMAT, modloader_get_application_id());
    }
    if (!direxists(*config_dir)) {
        mkpath(*config_dir);
    }
    return fmt::format("{}{}.json", *config_dir, id);
}
