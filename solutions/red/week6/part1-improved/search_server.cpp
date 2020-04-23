#include "search_server.h"
#include "iterator_range.h"
//#include "profile.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <array>
#include <cmath>

//vector<string> SplitIntoWords(const string &line, TotalDuration &d) {
//    ADD_DURATION(d);
//    istringstream words_input(line);
//    return {istream_iterator<string>(words_input), istream_iterator<string>()};
//}

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

    index = move(new_index);
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
//    LOG_DURATION("ADDQuery");

//    TotalDuration output("splitWords");
//    TotalDuration sortResults("sortResults");
//    TotalDuration lookUp("lookUp");
//    TotalDuration splitWords("splitWords");
//    TotalDuration reinit("reinit");
    vector<pair<size_t, size_t>> docId_count(index.DocsCount());
    for (string current_query; getline(query_input, current_query);) {
        const auto words = SplitIntoWords(current_query);
//        const auto words = SplitIntoWords(current_query, splitWords);
        docId_count.assign(index.DocsCount(), {0, 0});

//        {
//            ADD_DURATION(lookUp);
            for (const auto &word : words) {
                auto iter = index.Lookup(word);
                if (iter != index.LookupEnd()){
                    for (const auto &doc : iter->second) {
                        docId_count[doc.first].second += doc.second;
                        docId_count[doc.first].first = doc.first;
                    }
                }

            }
//        }


//        {
//            ADD_DURATION(sortResults);
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
//        }


//        {
//            ADD_DURATION(output);
            search_results_output << current_query << ':';
            size_t five = 5;
            for (auto &elem : Head(docId_count, five)) {
                if (elem.second == 0) continue;
                search_results_output << " {"
                                      << "docid: " << elem.first << ", "
                                      << "hitcount: " << elem.second << '}';
            }
            search_results_output << endl;
//        }

    }
}

