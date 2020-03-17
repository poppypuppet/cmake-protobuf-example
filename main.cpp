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

    vector<string> data_filenames;

    DescriptorPool descriptorPool;
    DynamicMessageFactory factory;

    // add message.MSG to DescriptorPool
    FileDescriptorProto message_file_desc_proto;
    GetMessageTypeFromProtoFile(message_proto_filename, &message_file_desc_proto);
    const FileDescriptor *message_file_descriptor = descriptorPool.BuildFile(message_file_desc_proto);
    cout << "message_file_descriptor" << endl;
    cout << message_file_descriptor->DebugString() << endl;

    // add ask.MSG to DescriptorPool
    FileDescriptorProto ask_file_desc_proto;
    GetMessageTypeFromProtoFile(ask_proto_filename, &ask_file_desc_proto);
    const FileDescriptor *ask_file_descriptor = descriptorPool.BuildFile(ask_file_desc_proto);
    cout << "ask_file_descriptor" << endl;
    cout << ask_file_descriptor->DebugString() << endl;

    // verify DescriptorPool can find this type
    const Descriptor *message_desc = descriptorPool.FindMessageTypeByName(message_name);
    if (message_desc != nullptr) {
        cout << message_desc->DebugString() << endl;
    }

    // verify DescriptorPool can find this type
    const Descriptor *ask_desc = descriptorPool.FindMessageTypeByName(ask_name);
    if (message_desc != nullptr) {
        cout << ask_desc->DebugString() << endl;
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