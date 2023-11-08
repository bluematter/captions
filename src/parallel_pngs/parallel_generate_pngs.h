#ifndef PARALLEL_GENERATE_PNGS_H
#define PARALLEL_GENERATE_PNGS_H

#include <nlohmann/json.hpp>
#include <string>

void parallel_generate_pngs(nlohmann::json &segments, int scale, int width, int height, std::string font, unsigned int num_threads, bool highlighter, const std::string& text_color);

#endif