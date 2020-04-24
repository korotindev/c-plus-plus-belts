#include "search_server.h"
#include "iterator_range.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <array>
#include <cmath>

vector<string> SplitIntoWords(const string &line) {
    istringstream words_input(line);
    return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream &document_input) {
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream &document_input) {
    InvertedIndex new_index;

    for (string current_document; getline(document_input, current_document);) {
        new_index.Add(current_document);
    }

    new_index.Optimize();

    index = move(new_index);
}


void InvertedIndex::Add(const string &document) {
    docs.push_back(document);

    const size_t docId = docs.size() - 1;
    for (const auto &word : SplitIntoWords(document)) {
        not_optimized_internal_index__[word][docId]++;
    }
}

void InvertedIndex::Optimize() {
    for (const auto &[word, not_optimized_index_slice__] : not_optimized_internal_index__) {
        vector<pair<size_t, size_t>> &index_slice = index[word];
        index_slice.reserve(not_optimized_index_slice__.size());

        for (const auto &[doc_id, words_count] : not_optimized_index_slice__) {
            index_slice.emplace_back(doc_id, words_count);
        }
    }
}

vector<pair<size_t, size_t>> &InvertedIndex::Lookup(const string &word) {
    if (auto iter = index.find(word); iter != index.end()) {
        return iter->second;
    } else {
        return emptyLookupResult;
    }
}


void SearchServer::AddQueriesStream(
        istream &query_input, ostream &search_results_output
) {
    vector<pair<size_t, size_t>> docId_count(index.DocsCount());
    for (string current_query; getline(query_input, current_query);) {
        const auto words = SplitIntoWords(current_query);
        docId_count.assign(index.DocsCount(), {0, 0});

        for (const auto &word : words) {
            for (const auto &doc : index.Lookup(word)) {
                docId_count[doc.first].second += doc.second;
                docId_count[doc.first].first = doc.first;
            }

        }

        partial_sort(
                begin(docId_count),
                begin(docId_count) + static_cast<long>(min(5ul, docId_count.size())),
                end(docId_count),
                [](auto &lhs, auto &rhs) {
                    if (lhs.second > rhs.second) {
                        return true;
                    } else if (lhs.second < rhs.second) {
                        return false;
                    } else {
                        return lhs.first < rhs.first;
                    }
                }
        );

        search_results_output << current_query << ':';
        for (auto &elem : Head(docId_count, 5ul)) {
            if (elem.second == 0) continue;
            search_results_output << " {"
                                  << "docid: " << elem.first << ", "
                                  << "hitcount: " << elem.second << '}';
        }
        search_results_output << endl;
    }
}

