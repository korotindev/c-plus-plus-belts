#include "real_sphere_projection.h"

namespace MapRenderers {
  namespace Real {
    namespace RealSphere {

      Svg::Point RealProjector::operator()(Sphere::Point point) const {
        return {
            (point.longitude - min_lon_) * zoom_coef_ + padding_,
            (max_lat_ - point.latitude) * zoom_coef_ + padding_,
        };
      }
    }  // namespace RealSphere
  }    // namespace Real
}  // namespace MapRenderers
