#pragma once

#include <utility>
#include <iostream>

namespace RAII {
  template<class Provider>
  class Booking {
  public:
    Booking(const Booking& other) = delete;

    Booking(Booking&& other)
      : provider(other.provider),
        booking_id(other.booking_id) {
      other.provider = nullptr;
    };

    Booking(Provider* provider, int counter)
      : provider(provider),
        booking_id(counter) {
    }

    Booking& operator=(Booking&& other) {
      std::swap(provider, other.provider);
      std::swap(booking_id, other.booking_id);
      return *this;
    }

    ~Booking() {
      if (provider) {
        provider->CancelOrComplete(*this);
      }
    }

  private:
    Provider* provider;
    int booking_id;
  };
}

