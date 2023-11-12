#!/bin/bash

g++ -w -std=c++17 \
  -I/opt/homebrew/include \
  `pkg-config --cflags --libs cairo pango pangocairo`\
  src/captions.cpp \
  src/main.cpp \
  src/parallel_pngs/parallel_generate_pngs.cpp \
  src/create_intermediate/create_intermediate_videos.cpp \
  src/concatenate_videos/concatenate_videos.cpp \
  -o main \
  -lcurl
