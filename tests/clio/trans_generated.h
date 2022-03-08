// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_TRANS_CLIOFB_H_
#define FLATBUFFERS_GENERATED_TRANS_CLIOFB_H_

#include "flatbuffers/flatbuffers.h"

namespace cliofb {

struct action;
struct actionBuilder;
struct actionT;

struct transaction;
struct transactionBuilder;
struct transactionT;

struct actionT : public flatbuffers::NativeTable {
  typedef action TableType;
  uint32_t sender = 0;
  uint32_t contract = 0;
  uint32_t act = 0;
  std::vector<uint8_t> data{};
};

struct action FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef actionT NativeTableType;
  typedef actionBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SENDER = 4,
    VT_CONTRACT = 6,
    VT_ACT = 8,
    VT_DATA = 10
  };
  uint32_t sender() const {
    return GetField<uint32_t>(VT_SENDER, 0);
  }
  uint32_t contract() const {
    return GetField<uint32_t>(VT_CONTRACT, 0);
  }
  uint32_t act() const {
    return GetField<uint32_t>(VT_ACT, 0);
  }
  const flatbuffers::Vector<uint8_t> *data() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_DATA);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_SENDER, 4) &&
           VerifyField<uint32_t>(verifier, VT_CONTRACT, 4) &&
           VerifyField<uint32_t>(verifier, VT_ACT, 4) &&
           VerifyOffset(verifier, VT_DATA) &&
           verifier.VerifyVector(data()) &&
           verifier.EndTable();
  }
  actionT *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
  void UnPackTo(actionT *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;
  static flatbuffers::Offset<action> Pack(flatbuffers::FlatBufferBuilder &_fbb, const actionT* _o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

struct actionBuilder {
  typedef action Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_sender(uint32_t sender) {
    fbb_.AddElement<uint32_t>(action::VT_SENDER, sender, 0);
  }
  void add_contract(uint32_t contract) {
    fbb_.AddElement<uint32_t>(action::VT_CONTRACT, contract, 0);
  }
  void add_act(uint32_t act) {
    fbb_.AddElement<uint32_t>(action::VT_ACT, act, 0);
  }
  void add_data(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> data) {
    fbb_.AddOffset(action::VT_DATA, data);
  }
  explicit actionBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<action> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<action>(end);
    return o;
  }
};

inline flatbuffers::Offset<action> Createaction(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t sender = 0,
    uint32_t contract = 0,
    uint32_t act = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> data = 0) {
  actionBuilder builder_(_fbb);
  builder_.add_data(data);
  builder_.add_act(act);
  builder_.add_contract(contract);
  builder_.add_sender(sender);
  return builder_.Finish();
}

inline flatbuffers::Offset<action> CreateactionDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t sender = 0,
    uint32_t contract = 0,
    uint32_t act = 0,
    const std::vector<uint8_t> *data = nullptr) {
  auto data__ = data ? _fbb.CreateVector<uint8_t>(*data) : 0;
  return cliofb::Createaction(
      _fbb,
      sender,
      contract,
      act,
      data__);
}

flatbuffers::Offset<action> Createaction(flatbuffers::FlatBufferBuilder &_fbb, const actionT *_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);

struct transactionT : public flatbuffers::NativeTable {
  typedef transaction TableType;
  uint32_t expire = 0;
  uint16_t tapos = 0;
  uint16_t flags = 0;
  std::vector<std::unique_ptr<cliofb::actionT>> actions{};
  transactionT() = default;
  transactionT(const transactionT &o);
  transactionT(transactionT&&) FLATBUFFERS_NOEXCEPT = default;
  transactionT &operator=(transactionT o) FLATBUFFERS_NOEXCEPT;
};

struct transaction FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef transactionT NativeTableType;
  typedef transactionBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_EXPIRE = 4,
    VT_TAPOS = 6,
    VT_FLAGS = 8,
    VT_ACTIONS = 10
  };
  uint32_t expire() const {
    return GetField<uint32_t>(VT_EXPIRE, 0);
  }
  uint16_t tapos() const {
    return GetField<uint16_t>(VT_TAPOS, 0);
  }
  uint16_t flags() const {
    return GetField<uint16_t>(VT_FLAGS, 0);
  }
  const flatbuffers::Vector<flatbuffers::Offset<cliofb::action>> *actions() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<cliofb::action>> *>(VT_ACTIONS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_EXPIRE, 4) &&
           VerifyField<uint16_t>(verifier, VT_TAPOS, 2) &&
           VerifyField<uint16_t>(verifier, VT_FLAGS, 2) &&
           VerifyOffset(verifier, VT_ACTIONS) &&
           verifier.VerifyVector(actions()) &&
           verifier.VerifyVectorOfTables(actions()) &&
           verifier.EndTable();
  }
  transactionT *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
  void UnPackTo(transactionT *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;
  static flatbuffers::Offset<transaction> Pack(flatbuffers::FlatBufferBuilder &_fbb, const transactionT* _o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

struct transactionBuilder {
  typedef transaction Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_expire(uint32_t expire) {
    fbb_.AddElement<uint32_t>(transaction::VT_EXPIRE, expire, 0);
  }
  void add_tapos(uint16_t tapos) {
    fbb_.AddElement<uint16_t>(transaction::VT_TAPOS, tapos, 0);
  }
  void add_flags(uint16_t flags) {
    fbb_.AddElement<uint16_t>(transaction::VT_FLAGS, flags, 0);
  }
  void add_actions(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<cliofb::action>>> actions) {
    fbb_.AddOffset(transaction::VT_ACTIONS, actions);
  }
  explicit transactionBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<transaction> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<transaction>(end);
    return o;
  }
};

inline flatbuffers::Offset<transaction> Createtransaction(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t expire = 0,
    uint16_t tapos = 0,
    uint16_t flags = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<cliofb::action>>> actions = 0) {
  transactionBuilder builder_(_fbb);
  builder_.add_actions(actions);
  builder_.add_expire(expire);
  builder_.add_flags(flags);
  builder_.add_tapos(tapos);
  return builder_.Finish();
}

inline flatbuffers::Offset<transaction> CreatetransactionDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t expire = 0,
    uint16_t tapos = 0,
    uint16_t flags = 0,
    const std::vector<flatbuffers::Offset<cliofb::action>> *actions = nullptr) {
  auto actions__ = actions ? _fbb.CreateVector<flatbuffers::Offset<cliofb::action>>(*actions) : 0;
  return cliofb::Createtransaction(
      _fbb,
      expire,
      tapos,
      flags,
      actions__);
}

flatbuffers::Offset<transaction> Createtransaction(flatbuffers::FlatBufferBuilder &_fbb, const transactionT *_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);

inline actionT *action::UnPack(const flatbuffers::resolver_function_t *_resolver) const {
  auto _o = std::unique_ptr<actionT>(new actionT());
  UnPackTo(_o.get(), _resolver);
  return _o.release();
}

inline void action::UnPackTo(actionT *_o, const flatbuffers::resolver_function_t *_resolver) const {
  (void)_o;
  (void)_resolver;
  { auto _e = sender(); _o->sender = _e; }
  { auto _e = contract(); _o->contract = _e; }
  { auto _e = act(); _o->act = _e; }
  { auto _e = data(); if (_e) { _o->data.resize(_e->size()); std::copy(_e->begin(), _e->end(), _o->data.begin()); } }
}

inline flatbuffers::Offset<action> action::Pack(flatbuffers::FlatBufferBuilder &_fbb, const actionT* _o, const flatbuffers::rehasher_function_t *_rehasher) {
  return Createaction(_fbb, _o, _rehasher);
}

inline flatbuffers::Offset<action> Createaction(flatbuffers::FlatBufferBuilder &_fbb, const actionT *_o, const flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { flatbuffers::FlatBufferBuilder *__fbb; const actionT* __o; const flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto _sender = _o->sender;
  auto _contract = _o->contract;
  auto _act = _o->act;
  auto _data = _o->data.size() ? _fbb.CreateVector(_o->data) : 0;
  return cliofb::Createaction(
      _fbb,
      _sender,
      _contract,
      _act,
      _data);
}

inline transactionT::transactionT(const transactionT &o)
      : expire(o.expire),
        tapos(o.tapos),
        flags(o.flags) {
  actions.reserve(o.actions.size());
  for (const auto &v : o.actions) { actions.emplace_back((v) ? new cliofb::actionT(*v) : nullptr); }
}

inline transactionT &transactionT::operator=(transactionT o) FLATBUFFERS_NOEXCEPT {
  std::swap(expire, o.expire);
  std::swap(tapos, o.tapos);
  std::swap(flags, o.flags);
  std::swap(actions, o.actions);
  return *this;
}

inline transactionT *transaction::UnPack(const flatbuffers::resolver_function_t *_resolver) const {
  auto _o = std::unique_ptr<transactionT>(new transactionT());
  UnPackTo(_o.get(), _resolver);
  return _o.release();
}

inline void transaction::UnPackTo(transactionT *_o, const flatbuffers::resolver_function_t *_resolver) const {
  (void)_o;
  (void)_resolver;
  { auto _e = expire(); _o->expire = _e; }
  { auto _e = tapos(); _o->tapos = _e; }
  { auto _e = flags(); _o->flags = _e; }
  { auto _e = actions(); if (_e) { _o->actions.resize(_e->size()); for (flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { if(_o->actions[_i]) { _e->Get(_i)->UnPackTo(_o->actions[_i].get(), _resolver); } else { _o->actions[_i] = std::unique_ptr<cliofb::actionT>(_e->Get(_i)->UnPack(_resolver)); }; } } }
}

inline flatbuffers::Offset<transaction> transaction::Pack(flatbuffers::FlatBufferBuilder &_fbb, const transactionT* _o, const flatbuffers::rehasher_function_t *_rehasher) {
  return Createtransaction(_fbb, _o, _rehasher);
}

inline flatbuffers::Offset<transaction> Createtransaction(flatbuffers::FlatBufferBuilder &_fbb, const transactionT *_o, const flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { flatbuffers::FlatBufferBuilder *__fbb; const transactionT* __o; const flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto _expire = _o->expire;
  auto _tapos = _o->tapos;
  auto _flags = _o->flags;
  auto _actions = _o->actions.size() ? _fbb.CreateVector<flatbuffers::Offset<cliofb::action>> (_o->actions.size(), [](size_t i, _VectorArgs *__va) { return Createaction(*__va->__fbb, __va->__o->actions[i].get(), __va->__rehasher); }, &_va ) : 0;
  return cliofb::Createtransaction(
      _fbb,
      _expire,
      _tapos,
      _flags,
      _actions);
}

inline const cliofb::transaction *Gettransaction(const void *buf) {
  return flatbuffers::GetRoot<cliofb::transaction>(buf);
}

inline const cliofb::transaction *GetSizePrefixedtransaction(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<cliofb::transaction>(buf);
}

inline bool VerifytransactionBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<cliofb::transaction>(nullptr);
}

inline bool VerifySizePrefixedtransactionBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<cliofb::transaction>(nullptr);
}

inline void FinishtransactionBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<cliofb::transaction> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedtransactionBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<cliofb::transaction> root) {
  fbb.FinishSizePrefixed(root);
}

inline std::unique_ptr<cliofb::transactionT> UnPacktransaction(
    const void *buf,
    const flatbuffers::resolver_function_t *res = nullptr) {
  return std::unique_ptr<cliofb::transactionT>(Gettransaction(buf)->UnPack(res));
}

inline std::unique_ptr<cliofb::transactionT> UnPackSizePrefixedtransaction(
    const void *buf,
    const flatbuffers::resolver_function_t *res = nullptr) {
  return std::unique_ptr<cliofb::transactionT>(GetSizePrefixedtransaction(buf)->UnPack(res));
}

}  // namespace cliofb

#endif  // FLATBUFFERS_GENERATED_TRANS_CLIOFB_H_