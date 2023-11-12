# Use a base image with a C++ compiler
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
# Set the working directory
WORKDIR /app

# Install the necessary libraries
RUN apt-get update && \
    apt-get install -y \
    g++ \
    libboost-system-dev \
    libboost-thread-dev \
    libasio-dev \
    libcairo2-dev \
    libpango1.0-dev \
    libglib2.0-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    pkg-config \
    curl \
    ffmpeg

# Copy the source file
COPY . .

RUN mkdir -p out

# Compile the application
CMD g++ -std=c++17 \
  `pkg-config --cflags --libs cairo pango pangocairo`\
  src/captions.cpp \
  src/main.cpp \
  src/parallel_pngs/parallel_generate_pngs.cpp \
  src/create_intermediate/create_intermediate_videos.cpp \
  src/concatenate_videos/concatenate_videos.cpp \
  -o out/captions \
    -lboost_system \
    -lcurl \
    -lpthread \
    -lcairo \
    -lpango-1.0 \
    -lgobject-2.0  \
    -lpangocairo-1.0