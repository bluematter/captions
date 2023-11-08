g++ -std=c++17 -I/opt/homebrew/include `pkg-config --cflags --libs cairo pango pangocairo` main.cpp -o main

./main ~/Desktop/youtube_video.mp4
