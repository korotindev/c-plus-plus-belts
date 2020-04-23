#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <array>
#include <cmath>
#include <future>

vector<string> SplitIntoWords(const string &line) {
    istringstream words_input(line);
    return {make_move_iterator(istream_iterator<string>(words_input)), make_move_iterator(istream_iterator<string>())};
}

SearchServer::SearchServer(istream &document_input) {
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBaseSync(istream &document_input) {
    if (!index.Empty()) {
        return;
    }

    InvertedIndex new_index;

    for (string current_document; getline(document_input, current_document);) {
        new_index.Add(current_document);
    }

    unique_lock lock(m);
    swap(index, new_index);
}

void SearchServer::UpdateDocumentBase(istream &document_input) {
    futures.push_back(
            async([&] () {
                UpdateDocumentBaseSync(document_input);
            })
    );
}


void InvertedIndex::Add(const string &document) {
    docs.push_back(document);

    const size_t docId = docs.size() - 1;
    for (const auto &word : SplitIntoWords(document)) {
        auto &wordConsistencyMap = update_consistency_index[word];
        auto wordIdInIndexIter = wordConsistencyMap.find(docId);

        if (wordIdInIndexIter != wordConsistencyMap.end()) {
            pair<size_t, size_t> &d = index[word][wordIdInIndexIter->second];
            d.second++;
        } else {
            auto &docsForWord = index[word];
            size_t ind = docsForWord.size();
            docsForWord.emplace_back(docId, 1);
            wordConsistencyMap[docId] = ind;
        }
    }
}

map<string, vector<pair<size_t, size_t>>>::iterator InvertedIndex::Lookup(const string &word) {
    return index.find(word);
}

map<string, vector<pair<size_t, size_t>>>::iterator InvertedIndex::LookupEnd() {
    return index.end();
}


void SearchServer::AddQueriesStream(
        istream &query_input, ostream &search_results_output
) {
    futures.push_back(
            async([&] () {
                AddQueriesStreamSync(query_input, search_results_output);
            })
    );
}

void SearchServer::AddQueriesStreamSync(
        istream &query_input, ostream &search_results_output
) {
    vector<pair<size_t, size_t>> docId_count;

    for (string current_query; getline(query_input, current_query);) {

        const auto &words = SplitIntoWords(current_query);
        {
            shared_lock lock(m);
            docId_count.assign(index.DocsCount(), {0, 0});
        }
        for (const auto &word : words) {
            map<string, vector<pair<size_t, size_t>>>::iterator iter, iterEnd;
            {
                shared_lock lock(m);
                iter = index.Lookup(word);
                iterEnd = index.LookupEnd();
            }
            if (iter != iterEnd) {
                for (const auto &doc : iter->second) {
                    docId_count[doc.first].second += doc.second;
                    docId_count[doc.first].first = doc.first;
                }
            }

        }

        auto docId_count_middle = begin(docId_count);
        auto docId_count_middle_index = static_cast<long>(min(5ul, docId_count.size()));
        advance(docId_count_middle, docId_count_middle_index);
        partial_sort(
                begin(docId_count),
                docId_count_middle,
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
        size_t five = 5;
        for (auto &elem : Head(docId_count, five)) {
            if (elem.second == 0) continue;
            search_results_output << " {"
                                  << "docid: " << elem.first << ", "
                                  << "hitcount: " << elem.second << '}';
        }
        search_results_output << endl;

    }
}

