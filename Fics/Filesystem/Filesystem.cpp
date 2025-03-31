#include <Filesystem.hpp>

std::string Filesystem::getFilesystem(const std::string& Filesystem) {
    std::string basePath = std::filesystem::current_path().parent_path().string();
    std::string lastChar = Filesystem.empty() ? "" : std::string(1, Filesystem.front());
    if(lastChar == "/") {
        return basePath + Filesystem;
    } else {
        return basePath + std::string("/") + Filesystem;
    }    
}

bool Filesystem::createFile(const std::string& Filesystem) {
    std::ofstream file(Filesystem);
    return file.good();
}

bool Filesystem::createDirectory(const std::string& Filesystem) {
    return std::filesystem::create_directories(Filesystem);
}

bool Filesystem::removeFile(const std::string& Filesystem) {
    return std::filesystem::remove(Filesystem);
}

bool Filesystem::removeDirectory(const std::string& Filesystem) {
    return std::filesystem::remove_all(Filesystem) > 0;
}