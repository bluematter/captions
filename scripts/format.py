import requests
import json


def load_local_json(path):
    with open(path, 'r') as f:
        data = json.load(f)
    return data


def fetch_and_parse_json(url):
    response = requests.get(url)
    response.raise_for_status()  # Ensure we got a successful response
    data = response.json()  # Parse JSON data from the response

    return data


def generate_frames(data):
    new_segments = []
    segment_id = 0  # Add a counter for the segment id

    for segment in data['segments']:
        frames = []
        current_frame = []
        word_count = 0

        for word in segment['words']:
            if word_count < 4 and not word['word'].lstrip().startswith(" "):
                current_frame.append(word)
                word_count += 1
            else:
                frames.append(current_frame)
                current_frame = [word]
                word_count = 1

        if current_frame:  # Append any remaining words
            frames.append(current_frame)

        # Create new segments based on the frames
        for i, frame in enumerate(frames):
            new_segment = segment.copy()  # Start with a copy of the original segment
            # Replace the words with the words in the frame
            new_segment['words'] = frame
            # Generate text from the words in the frame
            new_segment['text'] = " ".join(
                word['word'].lstrip() for word in frame)
            # Set the start to the start of the first word in the frame
            new_segment['start'] = frame[0]['start']
            # Set the end to the end of the last word in the frame
            new_segment['end'] = frame[-1]['end']
            # Set the id to the current value of segment_id
            new_segment['id'] = segment_id
            new_segments.append(new_segment)
            segment_id += 1  # Increment the counter

    # Return data in the original shape
    return {
        "detected_language": data.get("detected_language", "english"),
        "segments": new_segments
    }


def generate_frames_flat(data):
    new_segments = []
    segment_id = 0  # Add a counter for the segment id

    frames = []
    current_frame = []
    word_count = 0

    for item in data['transcription']:
        word = {
            'start': item['offsets']['from'] / 1000,  # Convert to seconds
            'end': item['offsets']['to'] / 1000,      # Convert to seconds
            'word': item['text']
        }
        if word_count < 4 and not word['word'].lstrip().startswith(" "):
            current_frame.append(word)
            word_count += 1
        else:
            frames.append(current_frame)
            current_frame = [word]
            word_count = 1

    if current_frame:  # Append any remaining words
        frames.append(current_frame)

    # Create new segments based on the frames
    for i, frame in enumerate(frames):
        new_segment = {
            'words': frame,
            'text': " ".join(word['word'].lstrip() for word in frame),
            'start': frame[0]['start'],
            'end': frame[-1]['end'],
            'id': segment_id
        }
        new_segments.append(new_segment)
        segment_id += 1  # Increment the counter

    # Return data in the original shape
    return {
        "detected_language": data.get("detected_language", "english"),
        "segments": new_segments
    }


# replace with your URL
url = 'https://storage.googleapis.com/youtuber-uploads/transcription_99.json'
path = '../automate/output.json'
# data = fetch_and_parse_json(url)
# frames = generate_frames(data)
frames = generate_frames_flat(load_local_json(path))

# Write to a local JSON file
with open('../automate/formatted_output.json', 'w') as f:
    json.dump(frames, f)
