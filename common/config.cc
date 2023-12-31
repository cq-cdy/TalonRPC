//
// Created by cdy on 23-9-27.
//
#include "tinyxml/tinyxml.h"
#include "config.h"
#include "config_reader.h"

#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
if (!name##_node) { \
  printf("Start talon server error, failed to read node [%s]\n", #name); \
  exit(0); \
} \



#define READ_STR_FROM_XML_NODE(name, parent) \
  TiXmlElement* name##_node = parent->FirstChildElement(#name); \
  if (!name##_node|| !name##_node->GetText()) { \
    printf("Start talon server error, failed to read config file %s\n", #name); \
    exit(0); \
  } \
  std::string name##_str = std::string(name##_node->GetText()); \



namespace talon {


    static Config *g_config = nullptr;
    std::unordered_map<std::string, std::string> Config::service_center_map = {};

    Config *Config::GetGlobalConfig() {
        return g_config;
    }

    void Config::SetGlobalConfig(const char *xmlfile) {
        if (g_config == nullptr) {
            if (xmlfile != nullptr) {
                g_config = new Config(xmlfile);
            } else {
                g_config = new Config();
            }

        }
    }

    Config::~Config() {
        if (m_xml_document) {
            delete m_xml_document;
            m_xml_document = nullptr;
        }
    }

    Config::Config() {
        m_log_level = "DEBUG";

    }

    Config::Config(const char *xmlfile) {
        m_xml_document = new TiXmlDocument();

        bool rt = m_xml_document->LoadFile(xmlfile);
        if (!rt) {
            printf("Start talon server error, failed to read config file %s, error info[%s] \n", xmlfile,
                   m_xml_document->ErrorDesc());
            exit(0);
        }

        READ_XML_NODE(root, m_xml_document);
        READ_XML_NODE(log, root_node);
        READ_XML_NODE(server, root_node);

        READ_STR_FROM_XML_NODE(log_level, log_node);
        READ_STR_FROM_XML_NODE(log_file_name, log_node);
        READ_STR_FROM_XML_NODE(log_file_path, log_node);
        READ_STR_FROM_XML_NODE(log_max_file_size, log_node);
        READ_STR_FROM_XML_NODE(log_sync_interval, log_node);

        m_log_level = log_level_str;
        m_log_file_name = log_file_name_str;
        m_log_file_path = log_file_path_str;
        m_log_max_file_size = std::atoi(log_max_file_size_str.c_str());
        m_log_sync_inteval = std::atoi(log_sync_interval_str.c_str());

        printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s],FILE_PATH[%s] MAX_FILE_SIZE[%d B], SYNC_INTEVAL[%d ms]\n",
               m_log_level.c_str(), m_log_file_name.c_str(), m_log_file_path.c_str(), m_log_max_file_size,
               m_log_sync_inteval);

        TiXmlElement *port_node = server_node->FirstChildElement("port");
        if (!port_node || !port_node->GetText()) {
            printf("Start talon server error, failed to read config file %s\n", "port");
            exit(0);
        }
        std::string port_str = std::string(port_node->GetText());

        TiXmlElement *io_threads_node = server_node->FirstChildElement("io_threads");
        if (!io_threads_node || !io_threads_node->GetText()) {
            printf("Start talon server error, failed to read config file %s\n", "io_threads");
            exit(0);
        }
        std::string io_threads_str = std::string(io_threads_node->GetText());

        TiXmlElement *local_ip_node = server_node->FirstChildElement("local_ip");
        if (!local_ip_node || !local_ip_node->GetText()) {
            printf("Start talon server error, failed to read config file %s\n", "local_ip");
            exit(0);
        }
        std::string local_ip_str = std::string(local_ip_node->GetText());

        m_port = std::atoi(port_str.c_str());
        m_io_threads = std::atoi(io_threads_str.c_str());
        m_local_ip = local_ip_str;

        TiXmlElement *stubs_node = root_node->FirstChildElement("stubs");

        if (stubs_node) {
            for (TiXmlElement *node = stubs_node->FirstChildElement(
                    "rpc_server"); node; node = node->NextSiblingElement("rpc_server")) {
                RpcStub stub;
                stub.name = std::string(node->FirstChildElement("name")->GetText());
                stub.timeout = std::atoi(node->FirstChildElement("timeout")->GetText());

                std::string ip = std::string(node->FirstChildElement("ip")->GetText());
                uint16_t port = std::atoi(node->FirstChildElement("port")->GetText());
                stub.addr = std::make_shared<IPNetAddr>(ip, port);
                m_rpc_stubs.insert(std::make_pair(stub.name, stub));
            }
        }


        printf("Server -- PORT[%d], IO Threads[%d]\n", m_port, m_io_threads);

    }

    void Config::setServiceCenterMap(const std::string &conf_path) {
        ConfigReader reader(conf_path);
        service_center_map = reader.getMap();
    }

    std::unordered_map<std::string, std::string> Config::getServiceCenterMap() {
        return service_center_map;
    }


}