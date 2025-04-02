#pragma once

#include <TiFF.hpp>
#include <Filesystem.hpp>
#include <Parser.hpp>

class Traitements : public Parser {
public:
    Traitements();
    ~Traitements() = default;

    void OnReadConfigFile(const char* cfg);
    bool Conversion(const std::string& fichier, TiFF& img);
    void Process();

    void CorrectionPOintsBlancs(TiFF& img);
    void CorrectionNoirs(TiFF& img);
    void CorrectionTrameCCD(TiFF& img);
    void CorrectionHomog(TiFF& img);
    void Reduction(TiFF& img, int index);

private:
    std::string folderEntrees;
    std::vector<std::string> fichiersEntree;

    std::string imgNoirPath;
    std::string imgTramePath;
    std::string imgHomogPath;
};
