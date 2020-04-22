#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <array>

vector<string> SplitIntoWords(const string &line, TotalDuration &d) {
    ADD_DURATION(d);
    istringstream words_input(line);
    return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

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
        new_index.Add(move(current_document));
    }

    index = move(new_index);
}


void InvertedIndex::Add(string document) {
    docs.push_back(document);

    const size_t docId = docs.size() - 1;
    for (const auto &word : SplitIntoWords(document)) {
        auto &wordConsistencyMap = update_consistency_index[word];
        auto wordIdInIndexIter = wordConsistencyMap.find(docId);

        if (wordIdInIndexIter != wordConsistencyMap.end()) {
            InvertedIndexDoc &d = index[word][wordIdInIndexIter->second];
            d.count++;
        } else {
            auto &docsForWord = index[word];
            size_t ind = docsForWord.size();
            docsForWord.push_back({docId, 1});
            wordConsistencyMap[docId] = ind;
        }
    }
}

vector<InvertedIndexDoc> &InvertedIndex::Lookup(const string &word) {
    if (auto it = index.find(word); it != index.end()) {
        return it->second;
    } else {
        return emptyDocs;
    }
}



struct Pair {
    size_t hitcount;
    int64_t docId;
};

bool operator<(const Pair &lhs, const Pair &rhs) {
    return rhs.hitcount < lhs.hitcount || (!(lhs.hitcount < rhs.hitcount) && -rhs.docId < -lhs.docId);
}


void SearchServer::AddQueriesStream(
        istream &query_input, ostream &search_results_output
) {
    LOG_DURATION("ADDQuery");

    TotalDuration comp("comp");
    TotalDuration output("splitWords");
    TotalDuration sortResults("sortResults");
    TotalDuration lookUp("lookUp");
    TotalDuration splitWords("splitWords");
    TotalDuration reinit("reinit");
    array<Pair, 50000> docId_count;
    for (string current_query; getline(query_input, current_query);) {
        const auto words = SplitIntoWords(current_query, splitWords);
        {
            ADD_DURATION(reinit);
            docId_count.fill({0, 0});
        }

        {
            ADD_DURATION(lookUp);
            for (const auto &word : words) {
                for (const auto &doc : index.Lookup(word)) {
                    auto &d = docId_count[doc.docId];
                    d.hitcount += doc.count;
                    d.docId = doc.docId;
                }
            }
        }

        {
            ADD_DURATION(sortResults);
            partial_sort(
                    begin(docId_count),
                    begin(docId_count) + 6,
                    end(docId_count)
            );
        }


        {
            ADD_DURATION(output);
            search_results_output << current_query << ':';
            for (auto &elem : Head(docId_count, 5)) {
                if (elem.hitcount == 0) continue;
                search_results_output << " {"
                                      << "docid: " << elem.docId << ", "
                                      << "hitcount: " << elem.hitcount << '}';
            }
            search_results_output << endl;
        }

    }
}

