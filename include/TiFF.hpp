#include <tiffio.h>
#include <HeadersBase.hpp>

class TiFF {
    public:
        TiFF();
    
        bool load(const std::string& filename);
        bool save(const std::string& filename) const;
        bool create(uint32_t w, uint32_t h, float initial_value);
        float getPixel(uint32_t x, uint32_t y) const;
        void setPixel(uint32_t x, uint32_t y, float value);
        void normalize();
        void fill(float value);
        void clear();
        
        uint32_t getWidth() const;
        uint32_t getHeight() const;
    
    private:
        uint32_t width, height;
        std::vector<float> data;
};
