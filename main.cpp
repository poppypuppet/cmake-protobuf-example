#include <iostream>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/compiler/parser.h>
#include <fstream>

#include "proto/message.pb.h"
#include "proto/ask.pb.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;

const FileDescriptor *AddProtoFileToDescriptorPool(string proto_filename,
                                                   DescriptorPool *descriptorPool);

bool DescriptorContainsMessageOfType(const Descriptor *des,
                                     const char *messageType);

void GetMessageTypeFromProtoFile(const string &proto_filename,
                                 FileDescriptorProto *file_desc_proto);

int main(int argc, char **argv) {
    string ask_data_filename = "ask.log";
    string message_data_filename = "message.log";

    string ask_proto_filename = "../proto/ask.proto";
    string message_proto_filename = "../proto/message.proto";

    string message_name = "message.MSG";
    string ask_name = "message.ASK";

    string p1ask_name = "p1.Ask";
    string p2ask_name = "p2.Ask";
    string p1ask_proto_filename = "../proto/p1Ask.proto";
    string p2ask_proto_filename = "../proto/p2Ask.proto";

    vector<string> data_filenames;

    DescriptorPool descriptorPool;
    DynamicMessageFactory factory;

    cout << "Add message.MSG to DescriptorPool" << endl;
    AddProtoFileToDescriptorPool(message_proto_filename, &descriptorPool);
    cout << "Add p1.Ask to DescriptorPool" << endl;
    AddProtoFileToDescriptorPool(p1ask_proto_filename, &descriptorPool);
    cout << "Add p2.Ask to DescriptorPool" << endl;
    AddProtoFileToDescriptorPool(p2ask_proto_filename, &descriptorPool);

    try {
        cout << "Verify DescriptorPool can find this " << message_name << endl;
        const Descriptor *message_desc = descriptorPool.FindMessageTypeByName(message_name);
        if (message_desc != nullptr) {
            cout << message_desc->DebugString() << endl;
        }

        cout << "Verify DescriptorPool can find this " << p1ask_name << endl;
        const Descriptor *ask_desc = descriptorPool.FindMessageTypeByName(p1ask_name);
        if (ask_desc != nullptr) {
            cout << ask_desc->DebugString() << endl;
        }

        cout << "Verify DescriptorPool can find this " << p2ask_name << endl;
        const Descriptor *ask2_desc = descriptorPool.FindMessageTypeByName(p2ask_name);
        if (ask_desc != nullptr) {
            cout << ask2_desc->DebugString() << endl;
        }
    } catch (const std::exception &e) {
        std::cout << e.what();
    }

    // write to log files
    message::MSG msg;
    msg.set_id(2);
    msg.set_str("msg_string");
    fstream message_output(message_data_filename, ios::out | ios::trunc | ios::binary);
    if (!msg.SerializeToOstream(&message_output)) {
        cerr << "Failed to write msg." << endl;
    }

    // write to log files
    message::ASK ask;
    ask.set_id(1234);
    fstream ask_output(ask_data_filename, ios::out | ios::trunc | ios::binary);
    if (!msg.SerializeToOstream(&ask_output)) {
        cerr << "Failed to write msg." << endl;
    }

    // TODO read from files and decode

    cout << "Hello Proto!" << endl;

    return 0;
}

const FileDescriptor *AddProtoFileToDescriptorPool(string proto_filename, DescriptorPool *descriptorPool) {
    FileDescriptorProto file_desc_proto;
    GetMessageTypeFromProtoFile(proto_filename, &file_desc_proto);
    const FileDescriptor *file_descriptor = descriptorPool->BuildFile(file_desc_proto);
    cout << "file_descriptor" << endl;
    cout << file_descriptor->DebugString() << endl;
    return file_descriptor;
}

void GetMessageTypeFromProtoFile(const string &proto_filename, FileDescriptorProto *file_desc_proto) {
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

bool DescriptorContainsMessageOfType(const Descriptor *des, const char *messageType) {
    for (int i = 0; i < des->field_count(); i++) {
        auto fieldDes = des->field(i);

        if (fieldDes->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            auto typeDes = fieldDes->message_type();
            if (typeDes->full_name() == messageType) {
                return true;
            }

            if (DescriptorContainsMessageOfType(typeDes, messageType)) {
                return true;
            }
        }
    }

    return false;
}