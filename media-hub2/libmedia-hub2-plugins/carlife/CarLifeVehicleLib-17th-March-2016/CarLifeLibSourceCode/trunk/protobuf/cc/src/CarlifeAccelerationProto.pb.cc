// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: CarlifeAccelerationProto.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "CarlifeAccelerationProto.pb.h"

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

const ::google::protobuf::Descriptor* CarlifeAcceleration_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  CarlifeAcceleration_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_CarlifeAccelerationProto_2eproto() {
  protobuf_AddDesc_CarlifeAccelerationProto_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "CarlifeAccelerationProto.proto");
  GOOGLE_CHECK(file != NULL);
  CarlifeAcceleration_descriptor_ = file->message_type(0);
  static const int CarlifeAcceleration_offsets_[4] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAcceleration, samplescount_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAcceleration, xaxisaccesum_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAcceleration, yaxisaccesum_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAcceleration, zaxisaccesum_),
  };
  CarlifeAcceleration_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      CarlifeAcceleration_descriptor_,
      CarlifeAcceleration::default_instance_,
      CarlifeAcceleration_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAcceleration, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CarlifeAcceleration, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(CarlifeAcceleration));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_CarlifeAccelerationProto_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    CarlifeAcceleration_descriptor_, &CarlifeAcceleration::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_CarlifeAccelerationProto_2eproto() {
  delete CarlifeAcceleration::default_instance_;
  delete CarlifeAcceleration_reflection_;
}

void protobuf_AddDesc_CarlifeAccelerationProto_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\036CarlifeAccelerationProto.proto\022\032com.ba"
    "idu.carlife.protobuf\"m\n\023CarlifeAccelerat"
    "ion\022\024\n\014samplesCount\030\001 \002(\005\022\024\n\014xAxisAcceSu"
    "m\030\002 \002(\005\022\024\n\014yAxisAcceSum\030\003 \002(\005\022\024\n\014zAxisAc"
    "ceSum\030\004 \002(\005", 171);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "CarlifeAccelerationProto.proto", &protobuf_RegisterTypes);
  CarlifeAcceleration::default_instance_ = new CarlifeAcceleration();
  CarlifeAcceleration::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_CarlifeAccelerationProto_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_CarlifeAccelerationProto_2eproto {
  StaticDescriptorInitializer_CarlifeAccelerationProto_2eproto() {
    protobuf_AddDesc_CarlifeAccelerationProto_2eproto();
  }
} static_descriptor_initializer_CarlifeAccelerationProto_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int CarlifeAcceleration::kSamplesCountFieldNumber;
const int CarlifeAcceleration::kXAxisAcceSumFieldNumber;
const int CarlifeAcceleration::kYAxisAcceSumFieldNumber;
const int CarlifeAcceleration::kZAxisAcceSumFieldNumber;
#endif  // !_MSC_VER

CarlifeAcceleration::CarlifeAcceleration()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void CarlifeAcceleration::InitAsDefaultInstance() {
}

CarlifeAcceleration::CarlifeAcceleration(const CarlifeAcceleration& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void CarlifeAcceleration::SharedCtor() {
  _cached_size_ = 0;
  samplescount_ = 0;
  xaxisaccesum_ = 0;
  yaxisaccesum_ = 0;
  zaxisaccesum_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CarlifeAcceleration::~CarlifeAcceleration() {
  SharedDtor();
}

void CarlifeAcceleration::SharedDtor() {
  if (this != default_instance_) {
  }
}

void CarlifeAcceleration::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* CarlifeAcceleration::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return CarlifeAcceleration_descriptor_;
}

const CarlifeAcceleration& CarlifeAcceleration::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_CarlifeAccelerationProto_2eproto();
  return *default_instance_;
}

CarlifeAcceleration* CarlifeAcceleration::default_instance_ = NULL;

CarlifeAcceleration* CarlifeAcceleration::New() const {
  return new CarlifeAcceleration;
}

void CarlifeAcceleration::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    samplescount_ = 0;
    xaxisaccesum_ = 0;
    yaxisaccesum_ = 0;
    zaxisaccesum_ = 0;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool CarlifeAcceleration::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 samplesCount = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &samplescount_)));
          set_has_samplescount();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_xAxisAcceSum;
        break;
      }

      // required int32 xAxisAcceSum = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_xAxisAcceSum:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &xaxisaccesum_)));
          set_has_xaxisaccesum();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_yAxisAcceSum;
        break;
      }

      // required int32 yAxisAcceSum = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_yAxisAcceSum:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &yaxisaccesum_)));
          set_has_yaxisaccesum();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(32)) goto parse_zAxisAcceSum;
        break;
      }

      // required int32 zAxisAcceSum = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_zAxisAcceSum:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &zaxisaccesum_)));
          set_has_zaxisaccesum();
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

void CarlifeAcceleration::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required int32 samplesCount = 1;
  if (has_samplescount()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->samplescount(), output);
  }

  // required int32 xAxisAcceSum = 2;
  if (has_xaxisaccesum()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(2, this->xaxisaccesum(), output);
  }

  // required int32 yAxisAcceSum = 3;
  if (has_yaxisaccesum()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->yaxisaccesum(), output);
  }

  // required int32 zAxisAcceSum = 4;
  if (has_zaxisaccesum()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(4, this->zaxisaccesum(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* CarlifeAcceleration::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required int32 samplesCount = 1;
  if (has_samplescount()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->samplescount(), target);
  }

  // required int32 xAxisAcceSum = 2;
  if (has_xaxisaccesum()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(2, this->xaxisaccesum(), target);
  }

  // required int32 yAxisAcceSum = 3;
  if (has_yaxisaccesum()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->yaxisaccesum(), target);
  }

  // required int32 zAxisAcceSum = 4;
  if (has_zaxisaccesum()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(4, this->zaxisaccesum(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int CarlifeAcceleration::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 samplesCount = 1;
    if (has_samplescount()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->samplescount());
    }

    // required int32 xAxisAcceSum = 2;
    if (has_xaxisaccesum()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->xaxisaccesum());
    }

    // required int32 yAxisAcceSum = 3;
    if (has_yaxisaccesum()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->yaxisaccesum());
    }

    // required int32 zAxisAcceSum = 4;
    if (has_zaxisaccesum()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->zaxisaccesum());
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

void CarlifeAcceleration::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const CarlifeAcceleration* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const CarlifeAcceleration*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void CarlifeAcceleration::MergeFrom(const CarlifeAcceleration& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_samplescount()) {
      set_samplescount(from.samplescount());
    }
    if (from.has_xaxisaccesum()) {
      set_xaxisaccesum(from.xaxisaccesum());
    }
    if (from.has_yaxisaccesum()) {
      set_yaxisaccesum(from.yaxisaccesum());
    }
    if (from.has_zaxisaccesum()) {
      set_zaxisaccesum(from.zaxisaccesum());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void CarlifeAcceleration::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void CarlifeAcceleration::CopyFrom(const CarlifeAcceleration& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CarlifeAcceleration::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000000f) != 0x0000000f) return false;

  return true;
}

void CarlifeAcceleration::Swap(CarlifeAcceleration* other) {
  if (other != this) {
    std::swap(samplescount_, other->samplescount_);
    std::swap(xaxisaccesum_, other->xaxisaccesum_);
    std::swap(yaxisaccesum_, other->yaxisaccesum_);
    std::swap(zaxisaccesum_, other->zaxisaccesum_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata CarlifeAcceleration::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = CarlifeAcceleration_descriptor_;
  metadata.reflection = CarlifeAcceleration_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace carlife
}  // namespace baidu
}  // namespace com

// @@protoc_insertion_point(global_scope)
