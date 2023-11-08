## Compile

```
sh ./scripts/compile
```

## Run Server

```
./main
```

## Trigger Job

Use PostMan to send a request to `http://localhost:8080` send a raw body that looks something like this

```
{
    "video_filename": "https://storage.googleapis.com/ai-video-maker/clop1ciqu0007vynmu65snejx_edited.mp4",
    "json_url": "https://storage.googleapis.com/ai-video-maker/transcription_clop1ciqu0007vynmu65snejx.json",
    "output_name": "lol",
    "font": "Montserrat ExtraBold 56",
    "highlighter": false,
    "text_color": "#f6be0e"
}
```
