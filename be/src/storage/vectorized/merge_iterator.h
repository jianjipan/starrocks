// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#pragma once

#include <vector>

#include "storage/vectorized/chunk_iterator.h"

namespace starrocks::vectorized {

// new_merge_iterator create a sorted iterator based on merge-sort algorithm.
// the order of rows is determinate by the key columns.
// if two rows compared equal, their order is determinate by the index of the source iterator
// in the vector |children|. the one with a lower index will come first.
// if |children| has only one element, the element will be returned directly.
//
// REQUIRES:
//  - |children| not empty.
//  - |children| have the same schemas.
//  - |children| are sorted iterators, i.e, each iterator in |children|
//    should return rows in an ascending order based on the key columns.
// one typical usage of this iterator is merging rows of the segments in the same `rowset`.
ChunkIteratorPtr new_merge_iterator(const std::vector<ChunkIteratorPtr>& children);

} // namespace starrocks::vectorized