#include "create_intermediate_videos.h"
#include <iostream>
#include <sstream>
#include <fstream>

void create_video(const nlohmann::json &j, const std::string &video_filename, const std::string &intermediate_video, double start_time, double end_time, int width, int height)
{
    std::ostringstream command, filter_complex;

    command << "ffmpeg -threads 2 -y -ss " << start_time << " -t " << (end_time - start_time) << " -i " << video_filename;

    int overlay_count = 0;

    double buffer_time = 0.01;
    double last_segment_end = 0.0;

    for (int i = 0; i < j.size(); ++i)
    {
        const auto &segment = j[i];
        if (segment.find("words") != segment.end())
        {
            const auto &words = segment["words"];
            double last_end = 0.0;
            double overlay_end = 0.0;

            for (size_t w = 0; w < words.size(); ++w)
            {
                if (words[w].find("start") != words[w].end())
                {
                    double start = words[w]["start"].get<double>() - start_time;

                    if (i > 0 && w == 0)
                    {
                        start += buffer_time;
                    }

                    double end = words[w]["end"].get<double>() - start_time;

                    if (w == words.size() - 1) // If this is the last word of the segment
                    {
                        if(i < j.size() - 1) // If this is not the last segment
                        {
                            const auto &next_segment = j[i+1];
                            if(next_segment.find("words") != next_segment.end())
                            {
                                const auto &next_words = next_segment["words"];
                                // Set overlay_end to the start time of the first word of the next segment
                                overlay_end = next_words[0]["start"].get<double>() - start_time;
                            }
                        }
                        else // If this is the last segment, just use the word end time
                        {
                            overlay_end = end;
                        }
                    }
                    else // If this is not the last word of the segment
                    {
                        overlay_end = words[w + 1]["start"].get<double>() - start_time;
                    }

                    command << " -i /tmp/captions/frame" << segment["id"].get<int>() << "_" << w << ".png";
                    if (w == 0 && i == 0)
                    {
                        filter_complex << "[" << overlay_count + 1 << ":v]scale=" << width << ":-1[temp" << overlay_count + 1 << "]; [0:v][temp" << overlay_count + 1 << "] overlay=enable='between(t," << start << "," << overlay_end << ")'[v" << overlay_count + 1 << "]";
                    }
                    else
                    {
                        filter_complex << "; [" << overlay_count + 1 << ":v]scale=" << width << ":-1[temp" << overlay_count + 1 << "]; ["
                                       << "v" << overlay_count << "][temp" << overlay_count + 1 << "] overlay=enable='between(t," << start << "," << overlay_end << ")'[v" << overlay_count + 1 << "]";
                    }
                    ++overlay_count;
                    last_end = end;
                }
            }

            last_segment_end = last_end; 
        }
    }

    command << " -filter_complex \"" << filter_complex.str() << "\" -map \"[v" << overlay_count << "]\" -preset ultrafast -crf 28 -map 0:a " << intermediate_video;

    std::cout << "Executing command: " << command.str() << std::endl;

    std::string error_file = "/tmp/error.txt";
    command << " 2>" << error_file;
    int ret = system(command.str().c_str());

    if (ret != 0)
    {
        std::cout << "Error occurred in creating the intermediate video. Exiting." << std::endl;
        std::ifstream error_stream(error_file);
        if (error_stream)
        {
            std::cout << "Error message: " << std::endl;
            std::string line;
            while (std::getline(error_stream, line))
            {
                std::cout << line << std::endl;
            }
            error_stream.close();
        }
        else
        {
            std::cout << "Unable to open error file for reading." << std::endl;
        }
    }
}

// Function to create intermediate videos for each group of n items
std::vector<std::string> create_intermediate_videos(const std::vector<std::pair<nlohmann::json::const_iterator, nlohmann::json::const_iterator>> &divided_json, const std::string &video_filename, int width, int height)
{
    std::vector<std::future<void>> futures;
    std::vector<std::string> intermediate_videos;

    int i = 0;
    for (const auto& range : divided_json)
    {
        std::string intermediate_video = "/tmp/intermediate" + std::to_string(i) + ".mp4";
        intermediate_videos.push_back(intermediate_video);

        // Calculate the start and end times of this segment
        double start_time = range.first->at("start");
        double end_time = (std::prev(range.second))->at("end");

        // Create a json chunk for the create_video function
        nlohmann::json chunk;
        for(auto it = range.first; it != range.second; ++it){
            chunk.push_back(*it);
        }

        // Run create_video asynchronously and store the future
        futures.push_back(std::async(std::launch::async, create_video, chunk, video_filename, intermediate_video, start_time, end_time, width, height));
        ++i;
    }

    // Wait for all tasks to complete
    for (auto &f : futures)
    {
        f.get();
    }

    return intermediate_videos;
}