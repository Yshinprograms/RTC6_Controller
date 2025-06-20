// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: ovf_lut.proto
// Protobuf C++ Version: 5.29.3

#include "ovf_lut.pb.h"

#include <algorithm>
#include <type_traits>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/generated_message_tctable_impl.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/generated_message_util.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace open_vector_format {

inline constexpr WorkPlaneLUT::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : vectorblockspositions_{},
        _vectorblockspositions_cached_byte_size_{0},
        workplaneshellposition_{::int64_t{0}},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR WorkPlaneLUT::WorkPlaneLUT(::_pbi::ConstantInitialized)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(_class_data_.base()),
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(),
#endif  // PROTOBUF_CUSTOM_VTABLE
      _impl_(::_pbi::ConstantInitialized()) {
}
struct WorkPlaneLUTDefaultTypeInternal {
  PROTOBUF_CONSTEXPR WorkPlaneLUTDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~WorkPlaneLUTDefaultTypeInternal() {}
  union {
    WorkPlaneLUT _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 WorkPlaneLUTDefaultTypeInternal _WorkPlaneLUT_default_instance_;

inline constexpr JobLUT::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : workplanepositions_{},
        _workplanepositions_cached_byte_size_{0},
        jobshellposition_{::int64_t{0}},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR JobLUT::JobLUT(::_pbi::ConstantInitialized)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(_class_data_.base()),
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(),
#endif  // PROTOBUF_CUSTOM_VTABLE
      _impl_(::_pbi::ConstantInitialized()) {
}
struct JobLUTDefaultTypeInternal {
  PROTOBUF_CONSTEXPR JobLUTDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~JobLUTDefaultTypeInternal() {}
  union {
    JobLUT _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 JobLUTDefaultTypeInternal _JobLUT_default_instance_;
}  // namespace open_vector_format
static constexpr const ::_pb::EnumDescriptor**
    file_level_enum_descriptors_ovf_5flut_2eproto = nullptr;
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_ovf_5flut_2eproto = nullptr;
const ::uint32_t
    TableStruct_ovf_5flut_2eproto::offsets[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
        protodesc_cold) = {
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::open_vector_format::JobLUT, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::open_vector_format::JobLUT, _impl_.jobshellposition_),
        PROTOBUF_FIELD_OFFSET(::open_vector_format::JobLUT, _impl_.workplanepositions_),
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::open_vector_format::WorkPlaneLUT, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::open_vector_format::WorkPlaneLUT, _impl_.workplaneshellposition_),
        PROTOBUF_FIELD_OFFSET(::open_vector_format::WorkPlaneLUT, _impl_.vectorblockspositions_),
};

static const ::_pbi::MigrationSchema
    schemas[] ABSL_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::open_vector_format::JobLUT)},
        {10, -1, -1, sizeof(::open_vector_format::WorkPlaneLUT)},
};
static const ::_pb::Message* const file_default_instances[] = {
    &::open_vector_format::_JobLUT_default_instance_._instance,
    &::open_vector_format::_WorkPlaneLUT_default_instance_._instance,
};
const char descriptor_table_protodef_ovf_5flut_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\rovf_lut.proto\022\022open_vector_format\">\n\006J"
    "obLUT\022\030\n\020jobShellPosition\030\001 \001(\003\022\032\n\022workP"
    "lanePositions\030\002 \003(\003\"M\n\014WorkPlaneLUT\022\036\n\026w"
    "orkPlaneShellPosition\030\001 \001(\003\022\035\n\025vectorBlo"
    "cksPositions\030\002 \003(\003B=Z;github.com/digital"
    "-production-aachen/openvectorformat/prot"
    "ob\006proto3"
};
static ::absl::once_flag descriptor_table_ovf_5flut_2eproto_once;
PROTOBUF_CONSTINIT const ::_pbi::DescriptorTable descriptor_table_ovf_5flut_2eproto = {
    false,
    false,
    249,
    descriptor_table_protodef_ovf_5flut_2eproto,
    "ovf_lut.proto",
    &descriptor_table_ovf_5flut_2eproto_once,
    nullptr,
    0,
    2,
    schemas,
    file_default_instances,
    TableStruct_ovf_5flut_2eproto::offsets,
    file_level_enum_descriptors_ovf_5flut_2eproto,
    file_level_service_descriptors_ovf_5flut_2eproto,
};
namespace open_vector_format {
// ===================================================================

class JobLUT::_Internal {
 public:
};

JobLUT::JobLUT(::google::protobuf::Arena* arena)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(arena, _class_data_.base()) {
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(arena) {
#endif  // PROTOBUF_CUSTOM_VTABLE
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:open_vector_format.JobLUT)
}
inline PROTOBUF_NDEBUG_INLINE JobLUT::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from, const ::open_vector_format::JobLUT& from_msg)
      : workplanepositions_{visibility, arena, from.workplanepositions_},
        _workplanepositions_cached_byte_size_{0},
        _cached_size_{0} {}

JobLUT::JobLUT(
    ::google::protobuf::Arena* arena,
    const JobLUT& from)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(arena, _class_data_.base()) {
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(arena) {
#endif  // PROTOBUF_CUSTOM_VTABLE
  JobLUT* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_, from);
  _impl_.jobshellposition_ = from._impl_.jobshellposition_;

  // @@protoc_insertion_point(copy_constructor:open_vector_format.JobLUT)
}
inline PROTOBUF_NDEBUG_INLINE JobLUT::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : workplanepositions_{visibility, arena},
        _workplanepositions_cached_byte_size_{0},
        _cached_size_{0} {}

inline void JobLUT::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  _impl_.jobshellposition_ = {};
}
JobLUT::~JobLUT() {
  // @@protoc_insertion_point(destructor:open_vector_format.JobLUT)
  SharedDtor(*this);
}
inline void JobLUT::SharedDtor(MessageLite& self) {
  JobLUT& this_ = static_cast<JobLUT&>(self);
  this_._internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  ABSL_DCHECK(this_.GetArena() == nullptr);
  this_._impl_.~Impl_();
}

inline void* JobLUT::PlacementNew_(const void*, void* mem,
                                        ::google::protobuf::Arena* arena) {
  return ::new (mem) JobLUT(arena);
}
constexpr auto JobLUT::InternalNewImpl_() {
  constexpr auto arena_bits = ::google::protobuf::internal::EncodePlacementArenaOffsets({
      PROTOBUF_FIELD_OFFSET(JobLUT, _impl_.workplanepositions_) +
          decltype(JobLUT::_impl_.workplanepositions_)::
              InternalGetArenaOffset(
                  ::google::protobuf::Message::internal_visibility()),
  });
  if (arena_bits.has_value()) {
    return ::google::protobuf::internal::MessageCreator::ZeroInit(
        sizeof(JobLUT), alignof(JobLUT), *arena_bits);
  } else {
    return ::google::protobuf::internal::MessageCreator(&JobLUT::PlacementNew_,
                                 sizeof(JobLUT),
                                 alignof(JobLUT));
  }
}
PROTOBUF_CONSTINIT
PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::google::protobuf::internal::ClassDataFull JobLUT::_class_data_ = {
    ::google::protobuf::internal::ClassData{
        &_JobLUT_default_instance_._instance,
        &_table_.header,
        nullptr,  // OnDemandRegisterArenaDtor
        nullptr,  // IsInitialized
        &JobLUT::MergeImpl,
        ::google::protobuf::Message::GetNewImpl<JobLUT>(),
#if defined(PROTOBUF_CUSTOM_VTABLE)
        &JobLUT::SharedDtor,
        ::google::protobuf::Message::GetClearImpl<JobLUT>(), &JobLUT::ByteSizeLong,
            &JobLUT::_InternalSerialize,
#endif  // PROTOBUF_CUSTOM_VTABLE
        PROTOBUF_FIELD_OFFSET(JobLUT, _impl_._cached_size_),
        false,
    },
    &JobLUT::kDescriptorMethods,
    &descriptor_table_ovf_5flut_2eproto,
    nullptr,  // tracker
};
const ::google::protobuf::internal::ClassData* JobLUT::GetClassData() const {
  ::google::protobuf::internal::PrefetchToLocalCache(&_class_data_);
  ::google::protobuf::internal::PrefetchToLocalCache(_class_data_.tc_table);
  return _class_data_.base();
}
PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<1, 2, 0, 0, 2> JobLUT::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    2, 8,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967292,  // skipmap
    offsetof(decltype(_table_), field_entries),
    2,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    _class_data_.base(),
    nullptr,  // post_loop_handler
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::open_vector_format::JobLUT>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    // repeated int64 workPlanePositions = 2;
    {::_pbi::TcParser::FastV64P1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(JobLUT, _impl_.workplanepositions_)}},
    // int64 jobShellPosition = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<::uint64_t, offsetof(JobLUT, _impl_.jobshellposition_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(JobLUT, _impl_.jobshellposition_)}},
  }}, {{
    65535, 65535
  }}, {{
    // int64 jobShellPosition = 1;
    {PROTOBUF_FIELD_OFFSET(JobLUT, _impl_.jobshellposition_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kInt64)},
    // repeated int64 workPlanePositions = 2;
    {PROTOBUF_FIELD_OFFSET(JobLUT, _impl_.workplanepositions_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kPackedInt64)},
  }},
  // no aux_entries
  {{
  }},
};

PROTOBUF_NOINLINE void JobLUT::Clear() {
// @@protoc_insertion_point(message_clear_start:open_vector_format.JobLUT)
  ::google::protobuf::internal::TSanWrite(&_impl_);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.workplanepositions_.Clear();
  _impl_.jobshellposition_ = ::int64_t{0};
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

#if defined(PROTOBUF_CUSTOM_VTABLE)
        ::uint8_t* JobLUT::_InternalSerialize(
            const MessageLite& base, ::uint8_t* target,
            ::google::protobuf::io::EpsCopyOutputStream* stream) {
          const JobLUT& this_ = static_cast<const JobLUT&>(base);
#else   // PROTOBUF_CUSTOM_VTABLE
        ::uint8_t* JobLUT::_InternalSerialize(
            ::uint8_t* target,
            ::google::protobuf::io::EpsCopyOutputStream* stream) const {
          const JobLUT& this_ = *this;
#endif  // PROTOBUF_CUSTOM_VTABLE
          // @@protoc_insertion_point(serialize_to_array_start:open_vector_format.JobLUT)
          ::uint32_t cached_has_bits = 0;
          (void)cached_has_bits;

          // int64 jobShellPosition = 1;
          if (this_._internal_jobshellposition() != 0) {
            target = ::google::protobuf::internal::WireFormatLite::
                WriteInt64ToArrayWithField<1>(
                    stream, this_._internal_jobshellposition(), target);
          }

          // repeated int64 workPlanePositions = 2;
          {
            int byte_size = this_._impl_._workplanepositions_cached_byte_size_.Get();
            if (byte_size > 0) {
              target = stream->WriteInt64Packed(
                  2, this_._internal_workplanepositions(), byte_size, target);
            }
          }

          if (PROTOBUF_PREDICT_FALSE(this_._internal_metadata_.have_unknown_fields())) {
            target =
                ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
                    this_._internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
          }
          // @@protoc_insertion_point(serialize_to_array_end:open_vector_format.JobLUT)
          return target;
        }

#if defined(PROTOBUF_CUSTOM_VTABLE)
        ::size_t JobLUT::ByteSizeLong(const MessageLite& base) {
          const JobLUT& this_ = static_cast<const JobLUT&>(base);
#else   // PROTOBUF_CUSTOM_VTABLE
        ::size_t JobLUT::ByteSizeLong() const {
          const JobLUT& this_ = *this;
#endif  // PROTOBUF_CUSTOM_VTABLE
          // @@protoc_insertion_point(message_byte_size_start:open_vector_format.JobLUT)
          ::size_t total_size = 0;

          ::uint32_t cached_has_bits = 0;
          // Prevent compiler warnings about cached_has_bits being unused
          (void)cached_has_bits;

          ::_pbi::Prefetch5LinesFrom7Lines(&this_);
           {
            // repeated int64 workPlanePositions = 2;
            {
              total_size +=
                  ::_pbi::WireFormatLite::Int64SizeWithPackedTagSize(
                      this_._internal_workplanepositions(), 1,
                      this_._impl_._workplanepositions_cached_byte_size_);
            }
          }
           {
            // int64 jobShellPosition = 1;
            if (this_._internal_jobshellposition() != 0) {
              total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(
                  this_._internal_jobshellposition());
            }
          }
          return this_.MaybeComputeUnknownFieldsSize(total_size,
                                                     &this_._impl_._cached_size_);
        }

void JobLUT::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<JobLUT*>(&to_msg);
  auto& from = static_cast<const JobLUT&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:open_vector_format.JobLUT)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_internal_mutable_workplanepositions()->MergeFrom(from._internal_workplanepositions());
  if (from._internal_jobshellposition() != 0) {
    _this->_impl_.jobshellposition_ = from._impl_.jobshellposition_;
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void JobLUT::CopyFrom(const JobLUT& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:open_vector_format.JobLUT)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}


void JobLUT::InternalSwap(JobLUT* PROTOBUF_RESTRICT other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.workplanepositions_.InternalSwap(&other->_impl_.workplanepositions_);
        swap(_impl_.jobshellposition_, other->_impl_.jobshellposition_);
}

::google::protobuf::Metadata JobLUT::GetMetadata() const {
  return ::google::protobuf::Message::GetMetadataImpl(GetClassData()->full());
}
// ===================================================================

class WorkPlaneLUT::_Internal {
 public:
};

WorkPlaneLUT::WorkPlaneLUT(::google::protobuf::Arena* arena)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(arena, _class_data_.base()) {
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(arena) {
#endif  // PROTOBUF_CUSTOM_VTABLE
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:open_vector_format.WorkPlaneLUT)
}
inline PROTOBUF_NDEBUG_INLINE WorkPlaneLUT::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from, const ::open_vector_format::WorkPlaneLUT& from_msg)
      : vectorblockspositions_{visibility, arena, from.vectorblockspositions_},
        _vectorblockspositions_cached_byte_size_{0},
        _cached_size_{0} {}

WorkPlaneLUT::WorkPlaneLUT(
    ::google::protobuf::Arena* arena,
    const WorkPlaneLUT& from)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(arena, _class_data_.base()) {
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(arena) {
#endif  // PROTOBUF_CUSTOM_VTABLE
  WorkPlaneLUT* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_, from);
  _impl_.workplaneshellposition_ = from._impl_.workplaneshellposition_;

  // @@protoc_insertion_point(copy_constructor:open_vector_format.WorkPlaneLUT)
}
inline PROTOBUF_NDEBUG_INLINE WorkPlaneLUT::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : vectorblockspositions_{visibility, arena},
        _vectorblockspositions_cached_byte_size_{0},
        _cached_size_{0} {}

inline void WorkPlaneLUT::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  _impl_.workplaneshellposition_ = {};
}
WorkPlaneLUT::~WorkPlaneLUT() {
  // @@protoc_insertion_point(destructor:open_vector_format.WorkPlaneLUT)
  SharedDtor(*this);
}
inline void WorkPlaneLUT::SharedDtor(MessageLite& self) {
  WorkPlaneLUT& this_ = static_cast<WorkPlaneLUT&>(self);
  this_._internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  ABSL_DCHECK(this_.GetArena() == nullptr);
  this_._impl_.~Impl_();
}

inline void* WorkPlaneLUT::PlacementNew_(const void*, void* mem,
                                        ::google::protobuf::Arena* arena) {
  return ::new (mem) WorkPlaneLUT(arena);
}
constexpr auto WorkPlaneLUT::InternalNewImpl_() {
  constexpr auto arena_bits = ::google::protobuf::internal::EncodePlacementArenaOffsets({
      PROTOBUF_FIELD_OFFSET(WorkPlaneLUT, _impl_.vectorblockspositions_) +
          decltype(WorkPlaneLUT::_impl_.vectorblockspositions_)::
              InternalGetArenaOffset(
                  ::google::protobuf::Message::internal_visibility()),
  });
  if (arena_bits.has_value()) {
    return ::google::protobuf::internal::MessageCreator::ZeroInit(
        sizeof(WorkPlaneLUT), alignof(WorkPlaneLUT), *arena_bits);
  } else {
    return ::google::protobuf::internal::MessageCreator(&WorkPlaneLUT::PlacementNew_,
                                 sizeof(WorkPlaneLUT),
                                 alignof(WorkPlaneLUT));
  }
}
PROTOBUF_CONSTINIT
PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::google::protobuf::internal::ClassDataFull WorkPlaneLUT::_class_data_ = {
    ::google::protobuf::internal::ClassData{
        &_WorkPlaneLUT_default_instance_._instance,
        &_table_.header,
        nullptr,  // OnDemandRegisterArenaDtor
        nullptr,  // IsInitialized
        &WorkPlaneLUT::MergeImpl,
        ::google::protobuf::Message::GetNewImpl<WorkPlaneLUT>(),
#if defined(PROTOBUF_CUSTOM_VTABLE)
        &WorkPlaneLUT::SharedDtor,
        ::google::protobuf::Message::GetClearImpl<WorkPlaneLUT>(), &WorkPlaneLUT::ByteSizeLong,
            &WorkPlaneLUT::_InternalSerialize,
#endif  // PROTOBUF_CUSTOM_VTABLE
        PROTOBUF_FIELD_OFFSET(WorkPlaneLUT, _impl_._cached_size_),
        false,
    },
    &WorkPlaneLUT::kDescriptorMethods,
    &descriptor_table_ovf_5flut_2eproto,
    nullptr,  // tracker
};
const ::google::protobuf::internal::ClassData* WorkPlaneLUT::GetClassData() const {
  ::google::protobuf::internal::PrefetchToLocalCache(&_class_data_);
  ::google::protobuf::internal::PrefetchToLocalCache(_class_data_.tc_table);
  return _class_data_.base();
}
PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<1, 2, 0, 0, 2> WorkPlaneLUT::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    2, 8,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967292,  // skipmap
    offsetof(decltype(_table_), field_entries),
    2,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    _class_data_.base(),
    nullptr,  // post_loop_handler
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::open_vector_format::WorkPlaneLUT>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    // repeated int64 vectorBlocksPositions = 2;
    {::_pbi::TcParser::FastV64P1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(WorkPlaneLUT, _impl_.vectorblockspositions_)}},
    // int64 workPlaneShellPosition = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<::uint64_t, offsetof(WorkPlaneLUT, _impl_.workplaneshellposition_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(WorkPlaneLUT, _impl_.workplaneshellposition_)}},
  }}, {{
    65535, 65535
  }}, {{
    // int64 workPlaneShellPosition = 1;
    {PROTOBUF_FIELD_OFFSET(WorkPlaneLUT, _impl_.workplaneshellposition_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kInt64)},
    // repeated int64 vectorBlocksPositions = 2;
    {PROTOBUF_FIELD_OFFSET(WorkPlaneLUT, _impl_.vectorblockspositions_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kPackedInt64)},
  }},
  // no aux_entries
  {{
  }},
};

PROTOBUF_NOINLINE void WorkPlaneLUT::Clear() {
// @@protoc_insertion_point(message_clear_start:open_vector_format.WorkPlaneLUT)
  ::google::protobuf::internal::TSanWrite(&_impl_);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.vectorblockspositions_.Clear();
  _impl_.workplaneshellposition_ = ::int64_t{0};
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

#if defined(PROTOBUF_CUSTOM_VTABLE)
        ::uint8_t* WorkPlaneLUT::_InternalSerialize(
            const MessageLite& base, ::uint8_t* target,
            ::google::protobuf::io::EpsCopyOutputStream* stream) {
          const WorkPlaneLUT& this_ = static_cast<const WorkPlaneLUT&>(base);
#else   // PROTOBUF_CUSTOM_VTABLE
        ::uint8_t* WorkPlaneLUT::_InternalSerialize(
            ::uint8_t* target,
            ::google::protobuf::io::EpsCopyOutputStream* stream) const {
          const WorkPlaneLUT& this_ = *this;
#endif  // PROTOBUF_CUSTOM_VTABLE
          // @@protoc_insertion_point(serialize_to_array_start:open_vector_format.WorkPlaneLUT)
          ::uint32_t cached_has_bits = 0;
          (void)cached_has_bits;

          // int64 workPlaneShellPosition = 1;
          if (this_._internal_workplaneshellposition() != 0) {
            target = ::google::protobuf::internal::WireFormatLite::
                WriteInt64ToArrayWithField<1>(
                    stream, this_._internal_workplaneshellposition(), target);
          }

          // repeated int64 vectorBlocksPositions = 2;
          {
            int byte_size = this_._impl_._vectorblockspositions_cached_byte_size_.Get();
            if (byte_size > 0) {
              target = stream->WriteInt64Packed(
                  2, this_._internal_vectorblockspositions(), byte_size, target);
            }
          }

          if (PROTOBUF_PREDICT_FALSE(this_._internal_metadata_.have_unknown_fields())) {
            target =
                ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
                    this_._internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
          }
          // @@protoc_insertion_point(serialize_to_array_end:open_vector_format.WorkPlaneLUT)
          return target;
        }

#if defined(PROTOBUF_CUSTOM_VTABLE)
        ::size_t WorkPlaneLUT::ByteSizeLong(const MessageLite& base) {
          const WorkPlaneLUT& this_ = static_cast<const WorkPlaneLUT&>(base);
#else   // PROTOBUF_CUSTOM_VTABLE
        ::size_t WorkPlaneLUT::ByteSizeLong() const {
          const WorkPlaneLUT& this_ = *this;
#endif  // PROTOBUF_CUSTOM_VTABLE
          // @@protoc_insertion_point(message_byte_size_start:open_vector_format.WorkPlaneLUT)
          ::size_t total_size = 0;

          ::uint32_t cached_has_bits = 0;
          // Prevent compiler warnings about cached_has_bits being unused
          (void)cached_has_bits;

          ::_pbi::Prefetch5LinesFrom7Lines(&this_);
           {
            // repeated int64 vectorBlocksPositions = 2;
            {
              total_size +=
                  ::_pbi::WireFormatLite::Int64SizeWithPackedTagSize(
                      this_._internal_vectorblockspositions(), 1,
                      this_._impl_._vectorblockspositions_cached_byte_size_);
            }
          }
           {
            // int64 workPlaneShellPosition = 1;
            if (this_._internal_workplaneshellposition() != 0) {
              total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(
                  this_._internal_workplaneshellposition());
            }
          }
          return this_.MaybeComputeUnknownFieldsSize(total_size,
                                                     &this_._impl_._cached_size_);
        }

void WorkPlaneLUT::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<WorkPlaneLUT*>(&to_msg);
  auto& from = static_cast<const WorkPlaneLUT&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:open_vector_format.WorkPlaneLUT)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_internal_mutable_vectorblockspositions()->MergeFrom(from._internal_vectorblockspositions());
  if (from._internal_workplaneshellposition() != 0) {
    _this->_impl_.workplaneshellposition_ = from._impl_.workplaneshellposition_;
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void WorkPlaneLUT::CopyFrom(const WorkPlaneLUT& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:open_vector_format.WorkPlaneLUT)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}


void WorkPlaneLUT::InternalSwap(WorkPlaneLUT* PROTOBUF_RESTRICT other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.vectorblockspositions_.InternalSwap(&other->_impl_.vectorblockspositions_);
        swap(_impl_.workplaneshellposition_, other->_impl_.workplaneshellposition_);
}

::google::protobuf::Metadata WorkPlaneLUT::GetMetadata() const {
  return ::google::protobuf::Message::GetMetadataImpl(GetClassData()->full());
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace open_vector_format
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::std::false_type
    _static_init2_ PROTOBUF_UNUSED =
        (::_pbi::AddDescriptors(&descriptor_table_ovf_5flut_2eproto),
         ::std::false_type{});
#include "google/protobuf/port_undef.inc"
