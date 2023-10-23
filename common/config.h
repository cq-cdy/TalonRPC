//
// Created by cdy on 23-9-27.
//

#ifndef TALONRPC_CONFIG_H
#define TALONRPC_CONFIG_H
#include "map"
#include "unordered_map"
#include "string"
#include <tinyxml/tinyxml.h>
#include "tcp/net_addr.h"
namespace talon {
    struct RpcStub {
        std::string name;
        NetAddr::s_ptr addr;
        int timeout{2000};
    };

    class Config {
    public:


        Config(const char *xmlfile);

        Config();

        ~Config();

    public:
        static Config *GetGlobalConfig();

        static std::unordered_map<std::string,std::string> getServiceCenterMap();
        static void setServiceCenterMap(const std::string& conf_path);

        static void SetGlobalConfig(const char *xmlfile);

        static std::unordered_map<std::string,std::string> service_center_map ;

    public:

        std::string m_log_level;
        std::string m_log_file_name;
        std::string m_log_file_path;
        std::string m_local_ip;

        int m_log_max_file_size{0};
        int m_log_sync_inteval{0};   // 日志同步间隔，ms

        int m_port{0};
        int m_io_threads{0};

        TiXmlDocument *m_xml_document{nullptr};

        std::map<std::string, RpcStub> m_rpc_stubs;
    };
}


#endif //TALONRPC_CONFIG_H
