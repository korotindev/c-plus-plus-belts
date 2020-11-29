#include "test_svg.h"

#include "sstream"
#include "svg.h"

using namespace std;

namespace TestSvg {
  void TestRect() {
    Svg::Rect rect;
    rect.SetPoint(Svg::Point{1, 1})
        .SetWidth(1.2)
        .SetHeight(89)
        .SetStrokeColor("black")
        .SetFillColor("white")
        .SetStrokeWidth(2);
    stringstream ss;
    rect.Render(ss);
    string expected =
        "<rect x=\"1\" y=\"1\" width=\"1.2\" height=\"89\" fill=\"white\" stroke=\"black\" stroke-width=\"2\" />";
    ASSERT_EQUAL(ss.str(), expected);
  }

  void Run(TestRunner &tr) { RUN_TEST(tr, TestRect); }
}  // namespace TestSvg