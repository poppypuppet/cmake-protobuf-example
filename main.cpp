#include <iostream>
#include <fstream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/dynamic_message.h>

#include "proto/ask.pb.h"

#include "ArchonErrorCollector.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;

const FileDescriptor *addProtoFileToDescriptorPool(string proto_filename, DescriptorPool *dp);

bool descriptorContainsMessageOfType(const Descriptor *des, const char *messageType);

void getMessageTypeFromProtoFile(const string &proto_filename, FileDescriptorProto *file_desc_proto);

bool binFileToDescriptorPool(DescriptorPool *pool, const char *filePath);

const FileDescriptor *importProtoFileToDescriptorPool(SourceTreeDescriptorDatabase *stdd, const string &proto_filename, DescriptorPool *dp);

const FileDescriptor *importProtoFile(DiskSourceTree *sourceTree, const string &proto_filename);

void EncodeStaticProtoSchema();

void DecodeStaticProtoSchema();

void EncodeDynamicProtoSchema(DynamicMessageFactory *dmf, DescriptorPool *dp, const std::string &name);

void DecodeDynamicProtoSchema(DiskSourceTree *sourceTree);

void DecodeDynamicProtoSchemaInOrder(DescriptorPool *descriptorPool, DiskSourceTree *diskSourceTree);

string ask_name = "message.ASK";
string p1ask_name = "p1.Ask";
string p2ask_name = "p2.Ask";
string ask_proto_filename = "../proto/ask.proto";
string p1ask_proto_filename = "../proto/p1Ask.proto";
string p2ask_proto_filename = "../proto/p2Ask.proto";
string ask_data_filename = "ask.log";

int main(int argc, char **argv) {

    //EncodeStaticProtoSchema();
    //DecodeStaticProtoSchema();


    DescriptorPool descriptorPool;
    DynamicMessageFactory factory(&descriptorPool);

    // add mapping directories
    DiskSourceTree sourceTree;
    sourceTree.MapPath("", "../archon/");
    sourceTree.MapPath("", "../archon-example/");

    DecodeDynamicProtoSchema(&sourceTree);
    DecodeDynamicProtoSchemaInOrder(&descriptorPool, &sourceTree);

    EncodeDynamicProtoSchema(&factory, &descriptorPool, "archon_video.VideoFrame");

    cout << "Hello Proto!" << endl;

    return 0;
}

const Descriptor *verifyDescriptorPoolCanFind(DescriptorPool *descriptorPool, const string &message_name) {
    cout << "Verify DescriptorPool can find this " << message_name << endl;
    const Descriptor *descriptor = descriptorPool->FindMessageTypeByName(message_name);
    if (descriptor != nullptr) {
        cout << descriptor->DebugString() << endl;
    }
    return descriptor;
}

void DecodeStaticProtoSchema() {}

void EncodeDynamicProtoSchema(DynamicMessageFactory *dmf, DescriptorPool *dp, const std::string &name) {
    // find descriptor from descriptor pool
    const Descriptor *descriptor = dp->FindMessageTypeByName(name);
    // create a const message ptr by factory method and message descriptor
    const google::protobuf::Message *tmp = dmf->GetPrototype(descriptor);
    // create a real msg
    google::protobuf::Message *msg = tmp->New();
    cout << "dynamic msg " << msg->DebugString() << endl;
}

void DecodeDynamicProtoSchema(DiskSourceTree *sourceTree) {
    const FileDescriptor *fd = importProtoFile(sourceTree, "archon_video.proto");
    if (fd != nullptr) {
        const Descriptor *descriptor = fd->FindMessageTypeByName("archon_video.VideoFrame");
        if (descriptor != nullptr) {
            cout << descriptor->DebugString() << endl;
        }
    }
}

void DecodeDynamicProtoSchemaInOrder(DescriptorPool *descriptorPool, DiskSourceTree *sourceTree) {
    FileDescriptorProto fdp;

    SourceTreeDescriptorDatabase stdd(sourceTree);
    stdd.FindFileByName("archon.proto", &fdp);
    descriptorPool->BuildFile(fdp);
    cout << "archon.StateValue" << descriptorPool->FindMessageTypeByName("archon.StateValue")->DebugString() << endl;

    stdd.FindFileByName("archon_video.proto", &fdp);
    descriptorPool->BuildFile(fdp);
    cout << "archon_video.VideoFrame" << descriptorPool->FindMessageTypeByName("archon_video.VideoFrame")->DebugString() << endl;
}

const FileDescriptor *importProtoFileToDescriptorPool(SourceTreeDescriptorDatabase *stdd, const string &proto_filename, DescriptorPool *dp) {
    FileDescriptorProto fdp;
    cout << "FileDescriptorProto" << proto_filename << endl;
    if (stdd->FindFileByName(proto_filename, &fdp)) {
        cout << fdp.DebugString() << endl;
    }
    fdp.descriptor();
    const FileDescriptor *fd = dp->BuildFile(fdp);
    cout << "DescriptorPool FileDescriptor" << endl;
    if (fd != nullptr) {
        cout << fd->DebugString() << endl;
    }
    return fd;
}

const FileDescriptor *importProtoFile(DiskSourceTree *sourceTree, const string &proto_filename) {
    ArchonErrorCollector aec;
    Importer importer(sourceTree, &aec);
    const FileDescriptor *fd = importer.Import(proto_filename);
    cout << "importer FileDescriptor" << endl;
    if (fd != nullptr) {
        cout << fd->DebugString() << endl;
    }
    return fd;
}

const FileDescriptor *addProtoFileToDescriptorPool(string proto_filename, DescriptorPool *dp) {
    FileDescriptorProto fdp;
    getMessageTypeFromProtoFile(proto_filename, &fdp);
    const FileDescriptor *fd = dp->BuildFile(fdp);
    cout << "file_descriptor" << endl;
    if (fd != nullptr) {
        cout << fd->DebugString() << endl;
    }
    return fd;
}


void EncodeStaticProtoSchema() {
    // write to log files
    message::Ask ask;
    ask.set_id(1234);
    fstream ask_output(ask_data_filename, ios::out | ios::trunc | ios::binary);
    if (!ask.SerializeToOstream(&ask_output)) {
        cerr << "Failed to write msg." << endl;
    }
}

void getMessageTypeFromProtoFile(const string &proto_filename, FileDescriptorProto *file_desc_proto) {
    FILE *proto_file = fopen(proto_filename.c_str(), "r");
    {
        if (proto_file == nullptr) {
            cout << "Cannot open .proto file: " << proto_filename << endl;
        }

        FileInputStream proto_input_stream(fileno(proto_file));
        Tokenizer tokenizer(&proto_input_stream, nullptr);
        Parser parser;
        if (!parser.Parse(&tokenizer, file_desc_proto)) {
            cout << "Cannot parse .proto file:" << proto_filename << endl;
        }
    }
    fclose(proto_file);

    // Here we walk around a bug in protocol buffers that
    // |Parser::Parse| does not set name (.proto filename) in
    // file_desc_proto.
    if (!file_desc_proto->has_name()) {
        file_desc_proto->set_name(proto_filename);
    }

    //cout << file_desc_proto->DebugString() << endl;
}


bool descriptorContainsMessageOfType(const Descriptor *des, const char *messageType) {
    for (int i = 0; i < des->field_count(); i++) {
        auto fieldDes = des->field(i);

        if (fieldDes->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            auto typeDes = fieldDes->message_type();
            if (typeDes->full_name() == messageType) {
                return true;
            }

            if (descriptorContainsMessageOfType(typeDes, messageType)) {
                return true;
            }
        }
    }
    return false;
}

bool binFileToDescriptorPool(DescriptorPool *pool, const char *filePath) {
    std::ifstream bproto;
    bproto.open(filePath, std::ifstream::in);

    FileDescriptorSet fds{};
    if (!fds.ParseFromIstream(&bproto)) {
        cout << "Failed to parse binary schema file %s" << filePath << endl;
        return false;
    }

    for (auto &file : fds.file()) {
        pool->BuildFile(file);
    }

    return true;
}
