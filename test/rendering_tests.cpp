#include "rendering_tests.h"

#include <cassert>
#include <string>
#include <unordered_map>

#include "test_runner.h"

using namespace std;

namespace RenderingTests {
  enum class StopNames {
    Teatralnaya,
    PansionatSvetlana,
    Cirk,
    UlitsaLisayaGora,
    UlitsaVerhnayaLisayaGora,
    Stadion,
    SanatoriiMetalurg,
    UlitsaBitha,
    SanatoriiImVoroshilova,
    SanatoriiPrimorie,
    KraevoGrecheskayaUlitsa,
    SanatoriiZarya,
    Macesta,
    MacestinskayaDolina
  };

  string c(StopNames name) {
    static const unordered_map<StopNames, string> data = {
        {StopNames::Teatralnaya, "Teatralnaya"},
        {StopNames::PansionatSvetlana, "PansionatSvetlana"},
        {StopNames::Cirk, "Cirk"},
        {StopNames::UlitsaLisayaGora, "UlitsaLisayaGora"},
        {StopNames::UlitsaVerhnayaLisayaGora, "UlitsaVerhnayaLisayaGora"},
        {StopNames::Stadion, "Stadion"},
        {StopNames::SanatoriiMetalurg, "SanatoriiMetalurg"},
        {StopNames::UlitsaBitha, "UlitsaBitha"},
        {StopNames::SanatoriiImVoroshilova, "SanatoriiImVoroshilova"},
        {StopNames::SanatoriiPrimorie, "SanatoriiPrimorie"},
        {StopNames::KraevoGrecheskayaUlitsa, "KraevoGrecheskayaUlitsa"},
        {StopNames::SanatoriiZarya, "SanatoriiZarya"},
        {StopNames::Macesta, "Macesta"},
        {StopNames::MacestinskayaDolina, "MacestinskayaDolina"}};

    return data.at(name);
  }

  vector<Descriptions::Stop> GenerateStops() {
    vector<Descriptions::Stop> stops;
    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::Teatralnaya),
                                          .position = Sphere::Point{3, 1},
                                          .distances = {{c(StopNames::PansionatSvetlana), 1}}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::PansionatSvetlana),
                                          .position = Sphere::Point{4, 2},
                                          .distances = {{c(StopNames::UlitsaLisayaGora), 1}, {c(StopNames::Cirk), 1}}});

    stops.emplace_back(Descriptions::Stop{
        .name = c(StopNames::Cirk), .position = Sphere::Point{5, 3}, .distances = {{c(StopNames::Stadion), 1}}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::UlitsaLisayaGora),
                                          .position = Sphere::Point{2, 4},
                                          .distances = {{c(StopNames::UlitsaVerhnayaLisayaGora), 1}}});

    stops.emplace_back(Descriptions::Stop{
        .name = c(StopNames::UlitsaVerhnayaLisayaGora), .position = Sphere::Point{1, 5}, .distances = {}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::Stadion),
                                          .position = Sphere::Point{8, 6},
                                          .distances = {{c(StopNames::SanatoriiMetalurg), 1}}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::SanatoriiMetalurg),
                                          .position = Sphere::Point{9, 7},
                                          .distances = {{c(StopNames::UlitsaBitha), 1}}});

    stops.emplace_back(Descriptions::Stop{
        .name = c(StopNames::UlitsaBitha),
        .position = Sphere::Point{6, 8},
        .distances = {{c(StopNames::KraevoGrecheskayaUlitsa), 1}, {c(StopNames::SanatoriiImVoroshilova), 1}}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::SanatoriiImVoroshilova),
                                          .position = Sphere::Point{11, 9},
                                          .distances = {{c(StopNames::SanatoriiPrimorie), 1}}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::SanatoriiPrimorie),
                                          .position = Sphere::Point{12, 10},
                                          .distances = {{c(StopNames::SanatoriiZarya), 1}}});

    stops.emplace_back(Descriptions::Stop{
        .name = c(StopNames::KraevoGrecheskayaUlitsa), .position = Sphere::Point{7, 11}, .distances = {}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::SanatoriiZarya),
                                          .position = Sphere::Point{13, 12},
                                          .distances = {{c(StopNames::Macesta), 1}}});

    stops.emplace_back(Descriptions::Stop{.name = c(StopNames::Macesta),
                                          .position = Sphere::Point{14, 13},
                                          .distances = {{c(StopNames::MacestinskayaDolina), 1}}});

    stops.emplace_back(Descriptions::Stop{
        .name = c(StopNames::MacestinskayaDolina), .position = Sphere::Point{10, 14}, .distances = {}});

    return stops;
  }

  void assert_projector_mapping(const Sphere::Projector &projector, const Descriptions::Stop &stop,
                                Svg::Point ideal_point) {
    auto result_point = projector(&stop);
    ASSERT_COMPARE_INFO(result_point.x, ideal_point.x, double(0.00001), stop.name + " x coord");
    ASSERT_COMPARE_INFO(result_point.y, ideal_point.y, double(0.00001), stop.name + " y coord");
  }

  Sphere::Projector build_projector(const vector<Descriptions::Stop> &stops, const double max_width,
                                    const double max_height, const double padding) {
    Descriptions::StopsDict dict;
    for (const auto &stop : stops) {
      dict[stop.name] = &stop;
    }

    return Sphere::Projector(dict, max_width, max_height, padding);
  }

  namespace NoCollissions {
    void TestOneLineManyStops() {
      vector<Descriptions::Stop> stops;
      stops.emplace_back(Descriptions::Stop{.name = "1", .position = Sphere::Point{1, 1}, .distances = {{"2", 1}}});
      stops.emplace_back(Descriptions::Stop{.name = "2", .position = Sphere::Point{2, 2}, .distances = {{"3", 1}}});
      stops.emplace_back(Descriptions::Stop{.name = "3", .position = Sphere::Point{3, 3}, .distances = {{"4", 1}}});
      stops.emplace_back(Descriptions::Stop{.name = "4", .position = Sphere::Point{4, 4}, .distances = {}});
      const double max_width = 3;
      const double max_height = 3;
      const double padding = 0;
      auto projector = build_projector(stops, max_width, max_height, padding);
      assert_projector_mapping(projector, stops[0], Svg::Point{.x = 0, .y = max_height});
      assert_projector_mapping(projector, stops[1], Svg::Point{.x = 1, .y = max_height - 1});
      assert_projector_mapping(projector, stops[2], Svg::Point{.x = 2, .y = max_height - 2});
      assert_projector_mapping(projector, stops[3], Svg::Point{.x = 3, .y = max_height - 3});
    }

    void TestOneStop() {
      vector<Descriptions::Stop> stops;
      stops.emplace_back(Descriptions::Stop{.name = "1", .position = Sphere::Point{1, 1}, .distances = {}});
      const double max_width = 2;
      const double max_height = 2;
      const double padding = 0;
      auto projector = build_projector(stops, max_width, max_height, padding);
      assert_projector_mapping(projector, stops[0], Svg::Point{.x = 0, .y = max_height});
    }
  }  // namespace NoCollissions

  namespace HasCollissions {
    void TestTwoSimpleStops() {
      vector<Descriptions::Stop> stops;
      stops.emplace_back(Descriptions::Stop{.name = "1", .position = Sphere::Point{1, 1}, .distances = {}});
      stops.emplace_back(Descriptions::Stop{.name = "2", .position = Sphere::Point{2, 2}, .distances = {}});
      const double max_width = 1;
      const double max_height = 1;
      const double padding = 0;
      auto projector = build_projector(stops, max_width, max_height, padding);
      assert_projector_mapping(projector, stops[0], Svg::Point{.x = 0, .y = max_height});
      assert_projector_mapping(projector, stops[1], Svg::Point{.x = 0, .y = max_height});  // collide !
    }

    void TestTwoStopsInOneGroupInOneLine() {
      {
        vector<Descriptions::Stop> stops;
        stops.emplace_back(Descriptions::Stop{.name = "1", .position = Sphere::Point{1, 1}, .distances = {{"2", 1}}});
        stops.emplace_back(Descriptions::Stop{.name = "2", .position = Sphere::Point{2, 2}, .distances = {{"3", 1}}});
        stops.emplace_back(Descriptions::Stop{.name = "3", .position = Sphere::Point{3, 3}, .distances = {{"4", 1}}});
        stops.emplace_back(Descriptions::Stop{.name = "4", .position = Sphere::Point{4, 2.5}, .distances = {}});
        const double max_height = 3;
        const double max_width = 2;
        const double padding = 0;
        auto projector = build_projector(stops, max_width, max_height, padding);
        assert_projector_mapping(projector, stops[0], Svg::Point{.x = 0, .y = max_height});
        assert_projector_mapping(projector, stops[1], Svg::Point{.x = 1, .y = max_height - 1});
        assert_projector_mapping(projector, stops[2], Svg::Point{.x = 2, .y = max_height - 2});
        assert_projector_mapping(projector, stops[3], Svg::Point{.x = 1, .y = max_height - 3});  // collide on x!
      }
      {
        vector<Descriptions::Stop> stops;
        stops.emplace_back(Descriptions::Stop{.name = "1", .position = Sphere::Point{1, 1}, .distances = {{"2", 1}}});
        stops.emplace_back(Descriptions::Stop{.name = "2", .position = Sphere::Point{2, 2}, .distances = {{"3", 1}}});
        stops.emplace_back(Descriptions::Stop{.name = "3", .position = Sphere::Point{3, 3}, .distances = {{"4", 1}}});
        stops.emplace_back(Descriptions::Stop{.name = "4", .position = Sphere::Point{2, 4}, .distances = {}});
        const double max_height = 2;
        const double max_width = 3;
        const double padding = 0;
        auto projector = build_projector(stops, max_width, max_height, padding);
        assert_projector_mapping(projector, stops[0], Svg::Point{.x = 0, .y = max_height});
        assert_projector_mapping(projector, stops[1], Svg::Point{.x = 1, .y = max_height - 1});
        assert_projector_mapping(projector, stops[2], Svg::Point{.x = 2, .y = max_height - 2});
        assert_projector_mapping(projector, stops[3], Svg::Point{.x = 3, .y = max_height - 1});  // collide on y!
      }
    }

    void TestTwoCrossedLinesWithStopInTheMiddle() {
      vector<Descriptions::Stop> stops;
      stops.emplace_back(Descriptions::Stop{.name = "left down", .position = Sphere::Point{1, 1}, .distances = {{"mid", 1}}});
      stops.emplace_back(Descriptions::Stop{.name = "right down", .position = Sphere::Point{0, 7}, .distances = {{"mid", 1}}});
      stops.emplace_back(Descriptions::Stop{.name = "mid", .position = Sphere::Point{3, 3}, .distances = {{"left up", 1}, {"right up", 1}}});
      stops.emplace_back(Descriptions::Stop{.name = "left up", .position = Sphere::Point{5, 0}, .distances = {}});
      stops.emplace_back(Descriptions::Stop{.name = "right up", .position = Sphere::Point{7, 5}, .distances = {}});
      const double max_height = 2;
      const double max_width = 2;
      const double padding = 0;
      auto projector = build_projector(stops, max_width, max_height, padding);
      assert_projector_mapping(projector, stops[0], Svg::Point{.x = 0, .y = 2});
      assert_projector_mapping(projector, stops[1], Svg::Point{.x = 2, .y = 2});
      assert_projector_mapping(projector, stops[2], Svg::Point{.x = 1, .y = 1});
      assert_projector_mapping(projector, stops[3], Svg::Point{.x = 0, .y = 0});
      assert_projector_mapping(projector, stops[4], Svg::Point{.x = 2, .y = 0});
      // all of points now forms a square with point in the middle
    }
  }  // namespace HasCollissions

  void TestNoColissions(TestRunner &tr) {
    RUN_TEST(tr, NoCollissions::TestOneLineManyStops);
    RUN_TEST(tr, NoCollissions::TestOneStop);
  }

  void TestHasColissions(TestRunner &tr) {
    RUN_TEST(tr, HasCollissions::TestTwoSimpleStops);
    RUN_TEST(tr, HasCollissions::TestTwoStopsInOneGroupInOneLine);
    RUN_TEST(tr, HasCollissions::TestTwoCrossedLinesWithStopInTheMiddle);
  }

  void TestProjector(TestRunner &tr) {
    TestNoColissions(tr);
    TestHasColissions(tr);
    // auto projector = build_projector(GenerateStops(), 10, 8, 0);

    // unordered_map<string, Svg::Point> result;
    // for (const auto &stop : stops) {
    //   result[stop.name] = projector(stop.position);
    // }

    // assert_stop(result, StopNames::Teatralnaya, Sphere::Point{.latitude = 1, .longitude = 0});
    // assert_stop(result, StopNames::PansionatSvetlana, Sphere::Point{.latitude = 4, .longitude = 2});
    // assert_stop(result, StopNames::PansionatSvetlana, 2, 4);
    // assert_stop(result, StopNames::Cirk, 3, 5);
    // assert_stop(result, StopNames::UlitsaLisayaGora, 3, 2);
    // assert_stop(result, StopNames::UlitsaVerhnayaLisayaGora, 5, 1);
    // assert_stop(result, StopNames::Stadion, 5, 7);
    // assert_stop(result, StopNames::SanatoriiMetalurg, 7, 9);
    // assert_stop(result, StopNames::UlitsaBitha, 8, 5);
    // assert_stop(result, StopNames::SanatoriiImVoroshilova, 9, 9);
    // assert_stop(result, StopNames::SanatoriiPrimorie, 10, 12);
    // assert_stop(result, StopNames::KraevoGrecheskayaUlitsa, 10, 7);
    // assert_stop(result, StopNames::SanatoriiZarya, 12, 13);
    // assert_stop(result, StopNames::Macesta, 13, 14);
    // assert_stop(result, StopNames::MacestinskayaDolina, 14, 9);
  }
}  // namespace RenderingTests