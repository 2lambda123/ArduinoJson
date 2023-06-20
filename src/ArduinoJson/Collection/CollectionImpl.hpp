// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Collection/CollectionData.hpp>
#include <ArduinoJson/Strings/StringAdapters.hpp>
#include <ArduinoJson/Variant/VariantCompare.hpp>
#include <ArduinoJson/Variant/VariantData.hpp>

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

inline void CollectionData::add(VariantSlot* slot) {
  ARDUINOJSON_ASSERT(slot != nullptr);

  if (tail_) {
    tail_->setNextNotNull(slot);
    tail_ = slot;
  } else {
    head_ = slot;
    tail_ = slot;
  }
}

inline void CollectionData::clear() {
  head_ = 0;
  tail_ = 0;
}

template <typename TAdaptedString>
inline VariantSlot* CollectionData::get(TAdaptedString key) const {
  if (key.isNull())
    return 0;
  VariantSlot* slot = head_;
  while (slot) {
    if (stringEquals(key, adaptString(slot->key())))
      break;
    slot = slot->next();
  }
  return slot;
}

inline VariantSlot* CollectionData::get(size_t index) const {
  if (!head_)
    return 0;
  return head_->next(index);
}

inline VariantSlot* CollectionData::getPrevious(VariantSlot* target) const {
  VariantSlot* current = head_;
  while (current) {
    VariantSlot* next = current->next();
    if (next == target)
      return current;
    current = next;
  }
  return 0;
}

inline void CollectionData::remove(VariantSlot* slot) {
  ARDUINOJSON_ASSERT(slot != nullptr);
  VariantSlot* prev = getPrevious(slot);
  VariantSlot* next = slot->next();
  if (prev)
    prev->setNext(next);
  else
    head_ = next;
  if (!next)
    tail_ = prev;
}

inline size_t CollectionData::memoryUsage() const {
  size_t total = 0;
  for (VariantSlot* s = head_; s; s = s->next()) {
    total += sizeof(VariantSlot) + s->data()->memoryUsage();
    if (s->ownsKey())
      total += sizeofString(strlen(s->key()));
  }
  return total;
}

inline size_t CollectionData::size() const {
  return slotSize(head_);
}

template <typename T>
inline void movePointer(T*& p, ptrdiff_t offset) {
  if (!p)
    return;
  p = reinterpret_cast<T*>(
      reinterpret_cast<void*>(reinterpret_cast<char*>(p) + offset));
  ARDUINOJSON_ASSERT(isAligned(p));
}

inline void CollectionData::movePointers(ptrdiff_t variantDistance) {
  movePointer(head_, variantDistance);
  movePointer(tail_, variantDistance);
  for (VariantSlot* slot = head_; slot; slot = slot->next())
    slot->data()->movePointers(variantDistance);
}

inline bool arrayEquals(const CollectionData& lhs, const CollectionData& rhs) {
  auto a = lhs.head();
  auto b = rhs.head();

  for (;;) {
    if (!a && !b)  // both ended
      return true;
    if (!a || !b)  // one ended
      return false;
    if (compare(a->data(), b->data()) != COMPARE_RESULT_EQUAL)
      return false;
    a = a->next();
    b = b->next();
  }
}

inline bool arrayEquals(const CollectionData* lhs, const CollectionData* rhs) {
  if (lhs == rhs)
    return true;
  if (!lhs || !rhs)
    return false;

  return arrayEquals(*lhs, *rhs);
}

inline bool objectEquals(const CollectionData& lhs, const CollectionData& rhs) {
  size_t count = 0;
  for (auto a = lhs.head(); a; a = a->next()) {
    auto b = rhs.get(adaptString(a->key()));
    if (!b)
      return false;
    if (compare(a->data(), b->data()) != COMPARE_RESULT_EQUAL)
      return false;
    count++;
  }
  return count == rhs.size();
}

ARDUINOJSON_END_PRIVATE_NAMESPACE
