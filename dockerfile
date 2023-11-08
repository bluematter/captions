# Use a base image with a C++ compiler
FROM gcc:11-bullseye AS build

# Set the working directory
WORKDIR /app

# Install the necessary libraries
RUN apt-get update && \
    apt-get install -y libboost-system-dev libboost-thread-dev libasio-dev libcairo2-dev libpango1.0-dev libglib2.0-dev libcurl4-openssl-dev nlohmann-json3-dev pkg-config curl ffmpeg

# Copy the source file
COPY . .

# Compile the application
RUN g++ -std=c++17 \
  `pkg-config --cflags --libs cairo pango pangocairo`\
  src/main.cpp \
  src/parallel_pngs/parallel_generate_pngs.cpp \
  src/create_intermediate/create_intermediate_videos.cpp \
  src/concatenate_videos/concatenate_videos.cpp \
  -o captions-over \
  -lboost_system \
  -lcurl


# Use the same base image for the runtime
FROM gcc:11-bullseye

# Set the working directory
WORKDIR /app

# Set the Google Cloud credentials environment variable
ENV GOOGLE_APPLICATION_CREDENTIALS=./google-credentials.json

# Install the Google Cloud SDK and ffmpeg
RUN echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] https://packages.cloud.google.com/apt cloud-sdk main" | tee -a /etc/apt/sources.list.d/google-cloud-sdk.list && \
    curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key --keyring /usr/share/keyrings/cloud.google.gpg add - && \
    apt-get update -y && apt-get install google-cloud-sdk ffmpeg -y

# Copy the binary from the build stage
COPY --from=build /app/captions-over .

# Copy the service account key file from the build stage
COPY --from=build /app/google-credentials.json ./google-credentials.json

# Install the runtime dependencies
RUN apt-get update && apt-get install -y libcairo2 libpango1.0-0 libpangocairo-1.0-0 libglib2.0-0 libboost-system1.74.0 libcurl4 && \
    rm -rf /var/lib/apt/lists/*

COPY ./fonts/OpenSans-Bold.ttf ./
RUN mkdir -p /usr/share/fonts/truetype/
RUN install -m644 OpenSans-Bold.ttf /usr/share/fonts/truetype/
RUN rm ./OpenSans-Bold.ttf
RUN fc-cache -f -v
RUN fc-list | grep 'OpenSans'

# Set the command to run your program
CMD ["./captions-over"]