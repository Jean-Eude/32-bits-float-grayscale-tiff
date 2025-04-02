#include <Traitements.hpp>
#include <Filesystem.hpp>

float computeVariance(const std::vector<float>& vec) {
    float mean = std::accumulate(vec.begin(), vec.end(), 0.0f) / vec.size();
    float var = 0.0f;
    for (float v : vec) {
        var += std::pow(v - mean, 2);
    }
    return var / vec.size();
}

int main() {
    Traitements trt;
    trt.Process();

    //std::cout << Filesystem::listFiles("Conversion") << std::endl;

    return 0;
}