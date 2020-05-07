#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

struct Email {
  string from;
  string to;
  string body;
};

class Worker {
public:
  virtual ~Worker() = default;

  virtual void Process(unique_ptr<Email> email) = 0;

  virtual void Run() {
    // только первому worker-у в пайплайне нужно это имплементировать
    throw logic_error("Unimplemented");
  }

protected:
  // реализации должны вызывать PassOn, чтобы передать объект дальше
  // по цепочке обработчиков
  void PassOn(unique_ptr<Email> email) const {
    if (next) {
      next->Process(move(email));
    }
  }

public:
  void SetNext(unique_ptr<Worker> next_) {
    this->next = move(next_);
  }

  unique_ptr<Worker> next;
};


class Reader : public Worker {
public:
  Reader(istream& input_) : input(input_) {}

  void Process(unique_ptr<Email> email) override {
    PassOn(move(email));
  }

  void Run() override {
    bool ok = true;
    while (ok) {
      auto email = make_unique<Email>();
      ok = ok && getline(input, email->from);
      ok = ok && getline(input, email->to);
      ok = ok && getline(input, email->body);
      if (ok) {
        Process(move(email));
      }
    }
  }

private:
  istream& input;
};


class Filter : public Worker {
public:
  using Function = function<bool(const Email&)>;

  Filter(Function func) : predicate(move(func)) {}

  void Process(unique_ptr<Email> email) {
    if (predicate(*email)) {
      PassOn(move(email));
    }
  }

private:
  Function predicate;
};


class Copier : public Worker {
public:
  Copier(string recipient_) : recipient(move(recipient_)) {}

  void Process(unique_ptr<Email> email) {
    if (email->to != recipient) {
      auto email_copy = make_unique<Email>(*email);
      email_copy->to = recipient;
      PassOn(move(email));
      PassOn(move(email_copy));
    } else {
      PassOn(move(email));
    }
  }

private:
  string recipient;
};


class Sender : public Worker {
public:
  Sender(ostream& output_) : output(output_) {}

  void Process(unique_ptr<Email> email) {
    output << email->from << '\n' << email->to << '\n' << email->body << '\n';
    PassOn(move(email));
  }

private:
  ostream& output;
};


// реализуйте класс
class PipelineBuilder {
public:
  // добавляет в качестве первого обработчика Reader
  explicit PipelineBuilder(istream& in) {
    head = make_unique<Reader>(in);
    tail = head.get();
  }

  // добавляет новый обработчик Filter
  PipelineBuilder& FilterBy(Filter::Function filter) {
    tail->SetNext(make_unique<Filter>(filter));
    tail = tail->next.get();
    return *this;
  }

  // добавляет новый обработчик Copier
  PipelineBuilder& CopyTo(string recipient) {
    tail->SetNext(make_unique<Copier>(move(recipient)));
    tail = tail->next.get();
    return *this;
  }

  // добавляет новый обработчик Sender
  PipelineBuilder& Send(ostream& out) {
    tail->SetNext(make_unique<Sender>(out));
    tail = tail->next.get();
    return *this;
  }

  // возвращает готовую цепочку обработчиков
  unique_ptr<Worker> Build() {
    return move(head);
  }

private:
  unique_ptr<Worker> head;
  Worker* tail;
};


void TestSanity() {
  string input = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "ralph@example.com\n"
    "erich@example.com\n"
    "I do not make mistakes of that kind\n"
  );
  istringstream inStream(input);
  ostringstream outStream;

  PipelineBuilder builder(inStream);
  builder.FilterBy([](const Email& email) {
    return email.from == "erich@example.com";
  });
  builder.CopyTo("richard@example.com");
  builder.Send(outStream);
  auto pipeline = builder.Build();

  pipeline->Run();

  string expectedOutput = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "erich@example.com\n"
    "richard@example.com\n"
    "Are you sure you pressed the right button?\n"
  );

  ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSanity);
  return 0;
}