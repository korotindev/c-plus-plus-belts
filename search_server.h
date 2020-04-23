#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
#include <thread>
#include <future>

using namespace std;

class InvertedIndex {
public:
    void Add(const string &document);

    map<string, vector<pair<size_t, size_t>>>::iterator Lookup(const string &word);
    map<string, vector<pair<size_t, size_t>>>::iterator LookupEnd();

    const string &GetDocument(size_t id) const {
        return docs[id];
    }

    size_t DocsCount() {
        return docs.size();
    }

    bool Empty() {
        return docs.empty();
    }

private:
    map<string, vector<pair<size_t, size_t>>> index;
    map<string, map<size_t, size_t>> update_consistency_index;
    vector<string> docs;
    vector<pair<size_t, size_t>> emptyDocs;
};

class SearchServer {
public:
    SearchServer() = default;

    explicit SearchServer(istream &document_input);

    void UpdateDocumentBase(istream &document_input);
    void AddQueriesStream(istream &query_input, ostream &search_results_output);
    void UpdateDocumentBaseSync(istream &document_input);
    void AddQueriesStreamSync(istream &query_input, ostream &search_results_output);

private:
    shared_mutex m;
    InvertedIndex index;
    vector<future<void>> futures;
};
