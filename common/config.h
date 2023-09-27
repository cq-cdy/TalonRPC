//
// Created by cdy on 23-9-27.
//

#ifndef TALONRPC_CONFIG_H
#define TALONRPC_CONFIG_H
#include "map"
#include "string"

namespace talon{
    class Config {
    public:

        Config(const char* xmlfile);

    public:
        static Config* GetGlobalConfig();
        static void SetGlobalConfig(const char* xmlfile);

    public:
        std::string m_log_level;

    };
}



#endif //TALONRPC_CONFIG_H
