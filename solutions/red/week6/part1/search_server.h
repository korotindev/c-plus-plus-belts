#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
using namespace std;

struct InvertedIndexDoc {
  size_t docId;
  size_t count;
};

class InvertedIndex {
public:
  void Add(string document);
  vector<InvertedIndexDoc> &Lookup(const string& word);

  const string& GetDocument(size_t id) const {
    return docs[id];
  }

  size_t DocsCount() {
      return docs.size();
  }

private:
  map<string, vector<InvertedIndexDoc>> index;
  map<string, map<size_t, size_t>> update_consistency_index;
  vector<string> docs;
  vector<InvertedIndexDoc> emptyDocs;
};

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
  InvertedIndex index;
};
