#pragma once

// Classe qui permet gérer des fichiers/répertoires

#include <HeadersBase.hpp>

class Filesystem {
    public:
        static std::string getFilesystem(const std::string& path);
        static bool createFile(const std::string& path);
        static bool createDirectory(const std::string& path);
        static bool removeFile(const std::string& path);
        static bool removeDirectory(const std::string& path);
        static std::vector<std::string> listFiles(const std::string& path);
        static std::string getExtension(const std::string& filename);
};