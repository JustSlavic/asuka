#include "color.hpp"

namespace Asuka {

Color24 Color24::White { 1.f, 1.f, 1.f };
Color24 Color24::Black { 0.f, 0.f, 0.f };
Color24 Color24::Red   { 1.f, 0.f, 0.f };
Color24 Color24::Green { 0.f, 1.f, 0.f };
Color24 Color24::Blue  { 0.f, 0.f, 1.f };


Color32 Color32::White { 1.0f, 1.0f, 1.0f, 1.0f };
Color32 Color32::Black { 1.0f, 1.0f, 1.0f, 1.0f };

Color32 Color32::Red   { 1.0f, 0.0f, 0.0f, 1.0f };
Color32 Color32::Green { 0.0f, 1.0f, 0.0f, 1.0f };
Color32 Color32::Blue  { 0.0f, 0.0f, 1.0f, 1.0f };

} // namespace Asuka
