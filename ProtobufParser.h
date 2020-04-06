#ifndef RPC_PROTOBUFPARSER_H
#define RPC_PROTOBUFPARSER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <google/protobuf/descriptor.h>

class ProtobufParser {
public:
    // load single bin file to DescriptorPool, dedupe inside.
    bool loadProtoSchema(const char *protoBinFileName);

    // get Descriptor from pool by TypeName
    const google::protobuf::Descriptor *findProtoDescriptor(const char *typeName);

    // inspect buffer byte be Descriptor
    std::vector<std::string> inspectMessage(google::protobuf::Descriptor *descritpro, const void *buffer, size_t size);

    // find shared buffer handler inside message
    std::vector<std::string> findSharedBufferHandles(const google::protobuf::Message &message, google::protobuf::Descriptor *descritpro);

private:
    // internal DescriptorPool
    google::protobuf::DescriptorPool descriptorPool;
};

#endif //RPC_PROTOBUFPARSER_H
