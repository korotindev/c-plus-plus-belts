#include "Common.h"
#include <unordered_map>
#include <list>
#include <mutex>

using namespace std;

class LruCache : public ICache {
public:
  LruCache(shared_ptr<IBooksUnpacker> books_unpacker_, const Settings& settings_)
    : books_unpacker(move(books_unpacker_)),
      max_memory(settings_.max_memory),
      allocated_space(0) {}

  BookPtr GetBook(const string& book_name) override {
    lock_guard guard(m);
    auto cacheIter = cache.find(book_name);
    BookPtr result;
    if (cacheIter != cache.end()) {
      auto booksIter = cacheIter->second;
      books.splice(begin(books), books, booksIter);
      result = *books.begin();
    } else {
      auto unpacked_book = books_unpacker->UnpackBook(book_name);
      auto needed_space = unpacked_book->GetContent().size();

      while(allocated_space + needed_space > max_memory && !books.empty()) {
        auto book_to_remove = books.back();
        allocated_space -= book_to_remove->GetContent().size();
        cache.erase(book_to_remove->GetName());
        books.pop_back();
      }

      if (allocated_space + needed_space <= max_memory) {
        books.push_front(move(unpacked_book));
        cache[book_name] = books.begin();
        allocated_space += needed_space;
        result = *books.begin();
      } else {
        result = move(unpacked_book);
      }
    }

    return result;
  }

private:
  using BooksList = list<BookPtr>;
  using BooksListIter = BooksList::const_iterator;

  shared_ptr<IBooksUnpacker> books_unpacker;
  const size_t max_memory;
  size_t allocated_space;

  BooksList books;
  unordered_map<string, BooksListIter> cache;

  mutex m;
};


unique_ptr<ICache> MakeCache(shared_ptr<IBooksUnpacker> books_unpacker, const ICache::Settings& settings) {
  return make_unique<LruCache>(move(books_unpacker), settings);
}