//
// Created by cdy on 23-9-27.
//
#include "tinyxml/tinyxml.h"
#include "config.h"

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

static talon::Config *g_config = nullptr;

talon::Config::Config(const char *xmlfile) {
    auto *xml_document = new TiXmlDocument();

    bool rt = xml_document->LoadFile(xmlfile);
    if (!rt) {
        printf("Start talon server error, failed to read config file %s, error info[%s] \n", xmlfile,
               xml_document->ErrorDesc());
        exit(0);
    }

    TiXmlElement *root_node = xml_document->FirstChildElement("root");
    if (!root_node) {
        printf("Start talon server error, failed to read node [%s]\n", "root");
        exit(0);
    }

    TiXmlElement *log_node = root_node->FirstChildElement("log");
    if (!log_node) {
        printf("Start talon server error, failed to read node [%s]\n", "log");
        exit(0);
    }



    TiXmlElement *log_level_node = log_node->FirstChildElement("log_level");
    if (!log_level_node || !log_level_node->GetText()) {
        printf("Start talon server error, failed to read config file %s\n", "log_level");
        exit(0);
    }
    std::string log_level_str = std::string(log_level_node->GetText());

    m_log_level = log_level_str;
}

talon::Config*  talon::Config::GetGlobalConfig() {
    return g_config;;
}

void talon::Config::SetGlobalConfig(const char *xmlfile) {
    if (g_config == nullptr) {
        g_config = new Config(xmlfile);
    }
}
