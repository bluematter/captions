#define CROOT_MAIN_
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include "crow/crow_all.h"
#include <curl/curl.h>
#include <filesystem>
#include <thread>
#include <vector>
#include "parallel_pngs/parallel_generate_pngs.h"
#include "create_intermediate/create_intermediate_videos.h"
#include "concatenate_videos/concatenate_videos.h"

std::vector<std::pair<nlohmann::json::const_iterator, nlohmann::json::const_iterator>> divide_json_array_by_time(const nlohmann::json &j, double time_limit)
{
    std::vector<std::pair<nlohmann::json::const_iterator, nlohmann::json::const_iterator>> divided_json;
    nlohmann::json::const_iterator chunk_start = j.begin();
    double chunk_end_time = 0;

    for (auto it = j.begin(); it != j.end(); ++it)
    {
        double start_time = (*it)["start"];
        double end_time = (*it)["end"];

        if (end_time - chunk_end_time > time_limit && it != chunk_start)
        {
            divided_json.emplace_back(chunk_start, it);
            chunk_start = it;
            chunk_end_time = start_time;
        }
    }

    if (chunk_start != j.end())
    {
        divided_json.emplace_back(chunk_start, j.end());
    }

    return divided_json;
}

std::pair<int, int> get_video_dimensions(const std::string &filename) {
    std::string command = "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 " + filename;
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != nullptr)
            result += buffer;
    }
    std::istringstream f(result);
    std::string s;
    std::getline(f, s, 'x');
    int width = std::stoi(s);
    std::getline(f, s);
    int height = std::stoi(s);
    return {width, height};
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json getJSONfromURL(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    nlohmann::json j = nlohmann::json::parse(readBuffer);
    return j;
}

nlohmann::json getJSONfromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    nlohmann::json j;
    file >> j;
    return j;
}

void cleanup(const std::vector<std::string> &intermediate_videos, const std::string &captions_path)
{
    // Remove intermediate videos
    for (const auto &video : intermediate_videos)
    {
        remove(video.c_str());
    }

    // Remove filter_complex.txt
    remove("filter_complex.txt");

    // Remove captions .png files
    // for (const auto &entry : std::filesystem::directory_iterator(captions_path))
    // {
    //     if (entry.path().extension() == ".png")
    //     {
    //         remove(entry.path().c_str());
    //     }
    // }
}

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    .methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            // Start the timer
            auto start_time = std::chrono::high_resolution_clock::now();

            unsigned int num_threads = std::thread::hardware_concurrency();
            if(num_threads == 0) {
                num_threads = 4; // Default value if number of cores could not be determined
            }

            std::cout << "num_threads " << num_threads << std::endl;

            // Parse the JSON object from the request
            nlohmann::json j = nlohmann::json::parse(req.body);

            // Extract the video filename from the JSON
            std::string video_filename = j["video_filename"].get<std::string>();
            std::string json_source = j["json_url"].get<std::string>();
            nlohmann::json j_transcriptions;
            std::string output_name = j["output_name"];
            std::string webhook = j.value("webhook", "");
            bool highlighter = j.value("highlighter", true);
            std::string text_color = j.value("text_color", "#FFFFFF");
            std::string selected_font = j.value("font", "Open Sans Bold 32");
            std::string output_path = "/tmp/" + output_name + ".mp4";

            // Make sure captions exists
            std::filesystem::create_directories("/tmp/captions/");

            // Check if json_source is a URL or a local path
            if (json_source.rfind("http", 0) == 0) {
                // If json_source starts with "http", it's a URL
                j_transcriptions = getJSONfromURL(json_source);
            } else {
                // Otherwise, assume it's a local path
                j_transcriptions = getJSONfromFile(json_source);
            }

            nlohmann::json segments = j_transcriptions["segments"];

            // Generate PNGs from the request JSON
            auto dimensions = get_video_dimensions(video_filename);
            parallel_generate_pngs(segments, 1, dimensions.first, dimensions.second, selected_font, num_threads, highlighter, text_color);

            // Stop the timer and calculate duration for PNG generation
            auto png_end_time = std::chrono::high_resolution_clock::now();
            auto png_duration = std::chrono::duration_cast<std::chrono::seconds>(png_end_time - start_time);
            std::cout << "PNG generation took " << png_duration.count() << " seconds." << std::endl;

            // Divide the JSON array into groups of n items
            std::vector<std::pair<nlohmann::json::const_iterator, nlohmann::json::const_iterator>> divided_json = divide_json_array_by_time(segments, 10);
            std::vector<std::string> intermediate_videos = create_intermediate_videos(divided_json, video_filename, dimensions.first, dimensions.second);

            // Stop the timer and calculate duration for intermediate_videos
            auto intermediate_videos_end_time = std::chrono::high_resolution_clock::now();
            auto intermediate_videos_duration = std::chrono::duration_cast<std::chrono::seconds>(intermediate_videos_end_time - start_time);
            std::cout << "Intermediate video generation took " << intermediate_videos_duration.count() << " seconds." << std::endl;

            // Concatenate all the intermediate videos to create the final video
            concatenate_videos(intermediate_videos, output_path);

            // Stop the timer and calculate total duration
            auto end_time = std::chrono::high_resolution_clock::now();
            auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
            std::cout << "Total operation took " << total_duration.count() << " seconds." << std::endl;

            // Upload to bucket
            system(("gsutil cp \"" + output_path + "\" gs://motionbox-website/").c_str());

            cleanup(intermediate_videos, "/tmp/captions/");

            if (!webhook.empty()) {
                CURL *curl;
                CURLcode res;

                curl_global_init(CURL_GLOBAL_DEFAULT);

                curl = curl_easy_init();
                if(curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, webhook.c_str());

                    /* specify the POST data */ 
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "key=value");

                    /* Perform the request, res will get the return code */ 
                    res = curl_easy_perform(curl);
                    /* Check for errors */ 
                    if(res != CURLE_OK)
                        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

                    /* always cleanup */ 
                    curl_easy_cleanup(curl);
                }

                curl_global_cleanup();
            }

            // Respond to the request
            return crow::response(200);
        } catch (const std::exception& e) {
            std::cerr << "Exception occurred: " << e.what() << std::endl;
        }
    });

    char* portVar = std::getenv("PORT");
    int port = (portVar != NULL) ? std::stoi(portVar) : 8080;
    std::cout << "Server running on port " << port << "..." << std::endl;

    app.port(port).multithreaded().run();

    return 0;
}
