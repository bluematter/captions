#include "concatenate_videos.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Function to concatenate all the intermediate videos to create the final video
void concatenate_videos(const std::vector<std::string> &video_filenames, const std::string &output_video)
{
    // Check that there are videos to concatenate
    if (video_filenames.size() == 0)
    {
        std::cout << "No videos to concatenate. Exiting." << std::endl;
        return;
    }

    // Create the file listing all videos to concatenate
    std::ofstream file_list("file_list.txt");
    if (file_list.is_open())
    {
        for (const auto &video : video_filenames)
        {
            file_list << "file '" << video << "'\n";
        }
        file_list.close();
    }
    else
    {
        std::cout << "Unable to open file_list.txt for writing." << std::endl;
        return;
    }

    // Create the ffmpeg command to concatenate the videos
    std::ostringstream command;
    command << "ffmpeg -y -threads 0 -f concat -safe 0 -i file_list.txt -c copy " << output_video;

    // Execute the ffmpeg command
    int ret = system(command.str().c_str());
    if (ret != 0)
    {
        std::cout << "Error occurred in concatenating the videos. Exiting." << std::endl;
        return;
    }
}
