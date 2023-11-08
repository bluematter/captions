import json


def parse_time(timestamp):
    """Converts a timestamp in the format "hh:mm:ss,ms" into a float representing seconds."""
    hours, minutes, rest = timestamp.split(':')
    seconds, milliseconds = rest.split(',')
    return float(hours) * 3600 + float(minutes) * 60 + float(seconds) + float(milliseconds) / 1000


def convert_data(data, max_words_per_segment=16):
    words = data["transcription"]
    segments = []
    current_segment_words = []
    for word in words:
        # Skip if "text" is empty or contains only whitespace
        if not word["text"].strip():
            continue
        if len(current_segment_words) >= max_words_per_segment:
            # Finalize current segment
            segments.append({
                "id": len(segments),
                "end": current_segment_words[-1]["end"],
                "seek": 0,  # You may want to adjust this according to your needs
                "text": " ".join(w["word"] for w in current_segment_words),
                "start": current_segment_words[0]["start"],
                "words": current_segment_words
            })
            current_segment_words = []
        # Convert and add current word to segment
        current_word = {
            "end": parse_time(word["timestamps"]["to"]),
            "word": word["text"].strip(),
            "start": parse_time(word["timestamps"]["from"]),
            "probability": 1.0  # You may want to adjust this according to your needs
        }
        current_segment_words.append(current_word)
    # Don't forget the last segment
    if current_segment_words:
        segments.append({
            "id": len(segments),
            "end": current_segment_words[-1]["end"],
            "seek": 0,  # You may want to adjust this according to your needs
            "text": " ".join(w["word"] for w in current_segment_words),
            "start": current_segment_words[0]["start"],
            "words": current_segment_words
        })
    return {"segments": segments}


# Load data
with open('/Users/michaelaubry/Desktop/output.json') as f:
    input_data = json.load(f)

# Convert data
output_data = convert_data(input_data, max_words_per_segment=16)

# Save data
with open('/Users/michaelaubry/Desktop/formatted.json', 'w') as f:
    json.dump(output_data, f, indent=2)
