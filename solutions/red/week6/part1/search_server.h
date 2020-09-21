#pragma once

#include <istream>
#include <list>
#include <map>
#include <ostream>
#include <set>
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

  void AddQueriesStream(istream &query_input, ostream &search_results_output);

private:
  InvertedIndex index;
};
