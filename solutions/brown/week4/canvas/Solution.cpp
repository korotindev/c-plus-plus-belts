#include "Common.h"

using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`
class OrdinaryShape : public IShape {
public:
  void SetPosition(Point p) final { this->point = p; }

  Point GetPosition() const final { return this->point; }

  void SetSize(Size s) final { this->size = s; }

  Size GetSize() const final { return this->size; }

  void SetTexture(std::shared_ptr<ITexture> t) final { this->texture = t; }

  ITexture *GetTexture() const final { return this->texture.get(); }

  bool IsPointInTexture(Point p) const {
    if (!texture) {
      return false;
    }

    auto textureSize = texture->GetSize();
    return p.y < textureSize.height && p.x < textureSize.width;
  }

protected:
  shared_ptr<ITexture> texture;
  Point point;
  Size size;
};

class RectangleShape : public OrdinaryShape {
public:
  std::unique_ptr<IShape> Clone() const final { return make_unique<RectangleShape>(*this); }

  void Draw(Image &image) const final {
    int maxHeight = min(this->point.y + this->size.height, static_cast<int>(image.size()));
    int maxWidth = min(this->point.x + this->size.width, static_cast<int>((image.empty() ? 0 : image[0].size())));
    for (int y = this->point.y; y < maxHeight; ++y) {
      for (int x = this->point.x; x < maxWidth; ++x) {
        Point texturePoint{x - this->point.x, y - this->point.y};
        if (this->IsPointInTexture(texturePoint)) {
          image[y][x] = texture->GetImage()[texturePoint.y][texturePoint.x];
        } else {
          image[y][x] = '.';
        }
      }
    }
  }
};

class EllipseShape : public OrdinaryShape {
public:
  std::unique_ptr<IShape> Clone() const final { return make_unique<EllipseShape>(*this); }

  void Draw(Image &image) const final {
    int maxHeight = min(this->point.y + this->size.height, static_cast<int>(image.size()));
    int maxWidth = min(this->point.x + this->size.width, static_cast<int>((image.empty() ? 0 : image[0].size())));
    for (int y = this->point.y; y < maxHeight; ++y) {
      for (int x = this->point.x; x < maxWidth; ++x) {
        Point texturePoint{x - this->point.x, y - this->point.y};
        if (IsPointInEllipse(texturePoint, this->size)) {
          if (this->IsPointInTexture(texturePoint)) {
            image[y][x] = texture->GetImage()[texturePoint.y][texturePoint.x];
          } else {
            image[y][x] = '.';
          }
        }
      }
    }
  }
};

// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
  switch (shape_type) {
  case ShapeType::Rectangle:
    return make_unique<RectangleShape>();

  case ShapeType::Ellipse:
    return make_unique<EllipseShape>();
  }
}