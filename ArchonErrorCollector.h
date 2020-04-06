#ifndef RPC_ARCHONERRORCOLLECTOR_H
#define RPC_ARCHONERRORCOLLECTOR_H

#include <google/protobuf/compiler/importer.h>

class ArchonErrorCollector;
class ArchonErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
    virtual void AddError(const std::string &filename, int line, int column, const std::string &message) {
        printf("%s, %d, %d, %s\n", filename.c_str(), line, column, message.c_str());
    }
};

#endif //RPC_ARCHONERRORCOLLECTOR_H
