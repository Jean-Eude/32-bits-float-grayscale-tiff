#include <Filesystem.hpp>

std::string Filesystem::getFilesystem(const std::string& path) {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        std::cerr << "Erreur getcwd: " << strerror(errno) << std::endl;
        return "";
    }

    std::string basePath(cwd);
    basePath = basePath.substr(0, basePath.find_last_of("/\\")); // simule parent_path()

    if (!path.empty() && path.front() == '/') {
        return basePath + path;
    } else {
        return basePath + "/" + path;
    }
}

bool Filesystem::createFile(const std::string& path) {
    std::ofstream file(path);
    return file.good();
}

bool Filesystem::createDirectory(const std::string& path) {
    // 0777 : permissions rwxrwxrwx (modifiables)
    return mkdir(path.c_str(), 0777) == 0 || errno == EEXIST;
}

bool Filesystem::removeFile(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

bool Filesystem::removeDirectory(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        // Ignore "." and ".."
        if (name == "." || name == "..") continue;

        std::string fullPath = path + "/" + name;

        struct stat st;
        if (stat(fullPath.c_str(), &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            removeDirectory(fullPath);
        } else {
            std::remove(fullPath.c_str());
        }
    }

    closedir(dir);
    return rmdir(path.c_str()) == 0;
}

std::vector<std::string> Filesystem::listFiles(const std::string& path) {
    std::vector<std::string> files;
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        std::cerr << "Erreur : impossible d'ouvrir le dossier '" << path << "'." << std::endl;
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name != "." && name != "..") {
            files.push_back(name);
        }
    }

    closedir(dir);
    return files;
}

std::string Filesystem::getExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == filename.length() - 1)
        return "";
    return filename.substr(dotPos + 1);
}