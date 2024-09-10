#include <algorithm>

#define IMITOR_IMPLEMENTATION
#include <image.hpp>

int main()
{
    auto img = load_image("../image.png");
    std::transform(img.begin(), img.end(), img.begin(),
                   [](const RGB24 pixel) -> RGB24 { return {pixel.b, pixel.g, pixel.r}; });
    save_image(img, "../new_image.png");
}
