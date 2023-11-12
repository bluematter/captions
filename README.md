## Compile

Binaries for different platforms can be built using `docker buildx` and specifying a --platform directive. Tested platforms are `linux/amd64` and `linux/arm64`.

```bash
docker buildx create --name captions-builder --use
docker buildx inspect --bootstrap
docker buildx build --platform linux/amd64 -t captions .
docker run --platform=linux/amd64 -v .:/app/out captions

./captions
```

## Run command

```bash
./captions \
  --input input.mp4 \
  --segments segments.json \
  --output output.mp4 \
  --font "Montserrat ExtraBold 56" \
  --highlighter true \
  --text_color "#f6be0e"
```

or with hosted assets...

```bash
./captions \
  --input https://example.com/input.mp4 \
  --segments https://example.com/segments.json \
  --output output.mp4 \
  --font "Montserrat ExtraBold 56" \
  --highlighter true \
  --text_color "#f6be0e"
```
