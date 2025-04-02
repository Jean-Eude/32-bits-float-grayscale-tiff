#pragma once

// Classe qui permet gérer des fichiers/répertoires

#include <HeadersBase.hpp>

class Filesystem {
    public :
        Filesystem() = delete;
        ~Filesystem() = default;

        static std::string getFilesystem(const std::string& Filesystem);
        static bool createFile(const std::string& Filesystem);
        static bool createDirectory(const std::string& Filesystem);
        static bool removeFile(const std::string& Filesystem);
        static bool removeDirectory(const std::string& Filesystem);
};