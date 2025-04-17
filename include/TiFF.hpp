#pragma once

#include <vector>
#include <string>
#include <cstdint>

class TiFF {
public:
	TiFF();
	TiFF(const TiFF& other) = default;
	TiFF& operator=(const TiFF& other) = default;

	bool load(const std::string& filename);
	bool save(const std::string& filename) const;
	bool create(uint32_t w, uint32_t h, double initial_value = 0.0);

	double getPixel(uint32_t x, uint32_t y) const;
	void setPixel(uint32_t x, uint32_t y, double value);

	void normalize();
	void fill(double value);
	void normalizePercentile(double pmin, double pmax);

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	std::vector<double>& getData() { return data; }
	const std::vector<double>& getData() const { return data; }

private:
	uint32_t width, height;
	std::vector<double> data;
};
