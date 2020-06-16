#ifndef RPC_DRAFT_H
#define RPC_DRAFT_H

#include <iostream>
#include <fstream>


#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/dynamic_message.h>

#include "ArchonErrorCollector.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;


const FileDescriptor *addProtoFileToDescriptorPool(string proto_filename, DescriptorPool *dp);

bool descriptorContainsMessageOfType(const Descriptor *des, const char *messageType);

void getMessageTypeFromProtoFile(const string &proto_filename, FileDescriptorProto *file_desc_proto);

const FileDescriptor *importProtoFileToDescriptorPool(SourceTreeDescriptorDatabase *stdd, const string &proto_filename, DescriptorPool *dp);

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

const Descriptor *verifyDescriptorPoolCanFind(DescriptorPool *descriptorPool, const string &message_name) {
    cout << "Verify DescriptorPool can find this " << message_name << endl;
    const Descriptor *descriptor = descriptorPool->FindMessageTypeByName(message_name);
    if (descriptor != nullptr) {
        cout << descriptor->DebugString() << endl;
    }
    return descriptor;
}

#endif //RPC_DRAFT_H
