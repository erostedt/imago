#define IMAGO_IMPLEMENTATION
#include "imago.hpp"

int main()
{
    const auto rgb = load_image("../image.png");
    const auto bgr =
        convert_image(rgb, [](const RGBA32 pixel) -> RGBA32 { return {pixel.b, pixel.g, pixel.r, pixel.a}; });
    save_image(bgr, "../new_image.png");
}
