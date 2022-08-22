// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: CarlifeAuthenResultProto.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "CarlifeAuthenResultProto.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace baidu {
namespace carlife {
namespace protobuf {

namespace {

const ::google::protobuf::Descriptor* CarlifeAuthenResult_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  CarlifeAuthenResult_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_CarlifeAuthenResultProto_2eproto() {
  protobuf_AddDesc_CarlifeAuthenResultProto_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "CarlifeAuthenResultProto.proto");
  GOOGLE_CHECK(file != NULL);
  CarlifeAuthenResult_descriptor_ = file->message_type(0);
  static const int CarlifeAuthenResult_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAuthenResult, authenresult_),
  };
  CarlifeAuthenResult_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      CarlifeAuthenResult_descriptor_,
      CarlifeAuthenResult::default_instance_,
      CarlifeAuthenResult_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAuthenResult, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAuthenResult, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(CarlifeAuthenResult));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_CarlifeAuthenResultProto_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    CarlifeAuthenResult_descriptor_, &CarlifeAuthenResult::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_CarlifeAuthenResultProto_2eproto() {
  delete CarlifeAuthenResult::default_instance_;
  delete CarlifeAuthenResult_reflection_;
}

void protobuf_AddDesc_CarlifeAuthenResultProto_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\036CarlifeAuthenResultProto.proto\022\032com.ba"
    "idu.carlife.protobuf\"+\n\023CarlifeAuthenRes"
    "ult\022\024\n\014authenResult\030\001 \002(\010", 105);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "CarlifeAuthenResultProto.proto", &protobuf_RegisterTypes);
  CarlifeAuthenResult::default_instance_ = new CarlifeAuthenResult();
  CarlifeAuthenResult::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_CarlifeAuthenResultProto_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_CarlifeAuthenResultProto_2eproto {
  StaticDescriptorInitializer_CarlifeAuthenResultProto_2eproto() {
    protobuf_AddDesc_CarlifeAuthenResultProto_2eproto();
  }
} static_descriptor_initializer_CarlifeAuthenResultProto_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int CarlifeAuthenResult::kAuthenResultFieldNumber;
#endif  // !_MSC_VER

CarlifeAuthenResult::CarlifeAuthenResult()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void CarlifeAuthenResult::InitAsDefaultInstance() {
}

CarlifeAuthenResult::CarlifeAuthenResult(const CarlifeAuthenResult& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void CarlifeAuthenResult::SharedCtor() {
  _cached_size_ = 0;
  authenresult_ = false;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CarlifeAuthenResult::~CarlifeAuthenResult() {
  SharedDtor();
}

void CarlifeAuthenResult::SharedDtor() {
  if (this != default_instance_) {
  }
}

void CarlifeAuthenResult::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* CarlifeAuthenResult::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return CarlifeAuthenResult_descriptor_;
}

const CarlifeAuthenResult& CarlifeAuthenResult::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_CarlifeAuthenResultProto_2eproto();
  return *default_instance_;
}

CarlifeAuthenResult* CarlifeAuthenResult::default_instance_ = NULL;

CarlifeAuthenResult* CarlifeAuthenResult::New() const {
  return new CarlifeAuthenResult;
}

void CarlifeAuthenResult::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    authenresult_ = false;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool CarlifeAuthenResult::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required bool authenResult = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   bool, ::google::protobuf::internal::WireFormatLite::TYPE_BOOL>(
                 input, &authenresult_)));
          set_has_authenresult();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CarlifeAuthenResult::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required bool authenResult = 1;
  if (has_authenresult()) {
    ::google::protobuf::internal::WireFormatLite::WriteBool(1, this->authenresult(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* CarlifeAuthenResult::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required bool authenResult = 1;
  if (has_authenresult()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteBoolToArray(1, this->authenresult(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int CarlifeAuthenResult::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required bool authenResult = 1;
    if (has_authenresult()) {
      total_size += 1 + 1;
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CarlifeAuthenResult::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const CarlifeAuthenResult* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const CarlifeAuthenResult*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void CarlifeAuthenResult::MergeFrom(const CarlifeAuthenResult& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_authenresult()) {
      set_authenresult(from.authenresult());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void CarlifeAuthenResult::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void CarlifeAuthenResult::CopyFrom(const CarlifeAuthenResult& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CarlifeAuthenResult::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;

  return true;
}

void CarlifeAuthenResult::Swap(CarlifeAuthenResult* other) {
  if (other != this) {
    std::swap(authenresult_, other->authenresult_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata CarlifeAuthenResult::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = CarlifeAuthenResult_descriptor_;
  metadata.reflection = CarlifeAuthenResult_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace carlife
}  // namespace baidu
}  // namespace com

// @@protoc_insertion_point(global_scope)
