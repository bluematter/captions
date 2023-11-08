#ifndef CREATE_INTERMEDIATE_VIDEOS_H
#define CREATE_INTERMEDIATE_VIDEOS_H

#include <vector>
#include <future>
#include "nlohmann/json.hpp"

// Forward declaration of the create_video function
void create_video(const nlohmann::json &j, const std::string &video_filename, const std::string &intermediate_video, double start_time, double end_time, int width, int height);

// Function to create intermediate videos for each group of n items
std::vector<std::string> create_intermediate_videos(const std::vector<std::pair<nlohmann::json::const_iterator, nlohmann::json::const_iterator>> &divided_json, const std::string &video_filename, int width, int height);

#endif
