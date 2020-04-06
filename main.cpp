#include <iostream>
#include <fstream>

#include<time.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/parser.h>

#include "proto/ask.pb.h"

#include "ArchonErrorCollector.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;

bool binFileToDescriptorPool(DescriptorPool *pool, const char *protoBinFilePath);

unsigned long EncodeStaticProtoSchema();

unsigned long DecodeStaticProtoSchema(DescriptorPool *descriptorPool);

unsigned long EncodeDynamicProtoSchema(DynamicMessageFactory *dmf, DescriptorPool *dp, const std::string &name);

unsigned long DecodeDynamicProtoSchema(DiskSourceTree *sourceTree);

unsigned long DecodeDynamicProtoSchemaInOrder(DescriptorPool *descriptorPool, DiskSourceTree *sourceTree);

string ask_data_filename = "ask.log";

int main(int argc, char **argv) {
    DescriptorPool staticPool;
    cout << "EncodeStaticProtoSchema=" << EncodeStaticProtoSchema() << endl;
    cout << "DecodeStaticProtoSchema=" << DecodeStaticProtoSchema(&staticPool) << endl;

    DescriptorPool dynamicPool;
    DynamicMessageFactory factory(&dynamicPool);

    // add mapping directories
    DiskSourceTree sourceTree;
    sourceTree.MapPath("", "../archon/");
    sourceTree.MapPath("", "../archon-video/");

    DecodeDynamicProtoSchema(&sourceTree);
    cout << "DecodeDynamicProtoSchemaInOrder=" << DecodeDynamicProtoSchemaInOrder(&dynamicPool, &sourceTree) << endl;
    cout << "EncodeDynamicProtoSchema=" << EncodeDynamicProtoSchema(&factory, &dynamicPool, "diagnostics.NodePin");

    cout << "Hello Proto!" << endl;

    return 0;
}

unsigned long DecodeStaticProtoSchema(DescriptorPool *descriptorPool) {
    clock_t startTime, endTime;
    startTime = clock();
    binFileToDescriptorPool(descriptorPool, "../archon-bin/archon_video.pb.bin");
    endTime = clock();
    return endTime - startTime;
}

unsigned long EncodeDynamicProtoSchema(DynamicMessageFactory *dmf, DescriptorPool *dp, const std::string &name) {
    clock_t startTime, endTime;
    startTime = clock();
    // find descriptor from descriptor pool
    const Descriptor *descriptor = dp->FindMessageTypeByName(name);
    // create a const message ptr by factory method and message descriptor
    const google::protobuf::Message *tmp = dmf->GetPrototype(descriptor);
    // create a real msg
    google::protobuf::Message *msg = tmp->New();
    endTime = clock();
    cout << "dynamic msg " << msg->DebugString() << endl;
    return endTime - startTime;
}

unsigned long DecodeDynamicProtoSchema(DiskSourceTree *sourceTree) {
    string top = "archon_internal.proto";
    string type = "archon_internal.SingleEdgeIo";

    ArchonErrorCollector aec;
    Importer importer(sourceTree, &aec);

    clock_t startTime, endTime;
    startTime = clock();
    const FileDescriptor *fd = importer.Import(top);
    endTime = clock();

    cout << "importer FileDescriptor" << endl;
    if (fd != nullptr) {
        cout << fd->DebugString() << endl;

        const Descriptor *descriptor = fd->FindMessageTypeByName(type);
        if (descriptor != nullptr) {
            cout << descriptor->DebugString() << endl;
        }
    }

    return endTime - startTime;
}

unsigned long DecodeDynamicProtoSchemaInOrder(DescriptorPool *descriptorPool, DiskSourceTree *sourceTree) {
    FileDescriptorProto fdp;

    string base = "archon.proto";
    string top = "archon_graph.proto";
    string type = "diagnostics.NodePin";

    unsigned long time = 0;
    clock_t startTime, endTime;
    startTime = clock();

    SourceTreeDescriptorDatabase stdd(sourceTree);
    stdd.FindFileByName(base, &fdp);
    descriptorPool->BuildFile(fdp);

    endTime = clock();
    time += endTime - startTime;

    startTime = clock();
    stdd.FindFileByName(top, &fdp);
    descriptorPool->BuildFile(fdp);
    endTime = clock();
    time += endTime - startTime;
    auto d = descriptorPool->FindMessageTypeByName(type);
    cout << type << d->DebugString() << endl;

    return time;
}

unsigned long EncodeStaticProtoSchema() {
    clock_t startTime, endTime;
    startTime = clock();

    message::Ask ask;
    ask.set_id(1234);

    endTime = clock();

    fstream ask_output(ask_data_filename, ios::out | ios::trunc | ios::binary);
    if (!ask.SerializeToOstream(&ask_output)) {
        cerr << "Failed to write msg." << endl;
    }

    return endTime - startTime;
}

bool binFileToDescriptorPool(DescriptorPool *pool, const char *protoBinFilePath) {
    std::ifstream bproto;
    bproto.open(protoBinFilePath, std::ifstream::in);

    FileDescriptorSet fds{};
    if (!fds.ParseFromIstream(&bproto)) {
        cout << "Failed to parse binary schema file %s" << protoBinFilePath << endl;
        return false;
    }

    for (auto &file : fds.file()) {
        pool->BuildFile(file);
    }

    return true;
}
