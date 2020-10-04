#pragma once

#include <future>
#include <istream>
#include <list>
#include <map>
#include <mutex>
#include <ostream>
#include <set>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class InvertedIndex {
 public:
  void Add(const string &document);

  void Optimize();

  vector<pair<size_t, size_t>> &Lookup(const string &word);

  const string &GetDocument(size_t id) const { return docs[id]; }

  size_t DocsCount() { return docs.size(); }

 private:
  map<string, vector<pair<size_t, size_t>>> index;
  map<string, map<size_t, size_t>> not_optimized_internal_index__;
  vector<string> docs;
  vector<pair<size_t, size_t>> emptyLookupResult;
};

class SearchServer {
 public:
  SearchServer() = default;

  explicit SearchServer(istream &document_input);

  void UpdateDocumentBase(istream &document_input);

  void UpdateDocumentBaseSync(istream &document_input);

  void AddQueriesStream(istream &query_input, ostream &search_results_output);

  void AddQueriesStreamSync(istream &query_input, ostream &search_results_output);

  static const size_t MAX_THREADS = 6;

 private:
  bool isFirstUpdate = true;
  InvertedIndex index;
  shared_mutex m;
  vector<future<void>> futures;
};
