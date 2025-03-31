#include <Parser.hpp>

void Parser::readConfigFile(const char* m_configFile) {
    this->m_configfile = m_configFile;
    std::ifstream cFile (this->m_configfile);

    if (cFile.is_open()) {

        std::string line;
        while(getline(cFile, line)) {
            // Enlèvement des "espaces blancs" (ex: ab=cd''' / ab''=''cd  == ab=cd)
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);       
                 
            if(line[0] == '#' || line.empty()) {
                continue;
            }

            // Equivalence (ex: ab=cd  ==  ab=cd)
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);

            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            try {
                if (!value.empty() && std::all_of(value.begin(), value.end(), [](char c) { return std::isdigit(c) || c == '.'; })) {
                    if (value.find('.') != std::string::npos) {
                        if (value.length() - value.find('.') > 2) {
                            m_cVars[name] = std::stod(value); // Double
                        } else {
                            m_cVars[name] = std::stof(value); // Float
                        }
                    } else {
                        m_cVars[name] = std::stoi(value); // Int
                    }
                } else {
                    m_cVars[name] = value; // String
                }
            } catch (const std::invalid_argument&) {
                m_cVars[name] = value;
            } catch (const std::out_of_range&) {
                m_cVars[name] = value;
            }
        }
    }
}