# WebM/ffmpeg microframe reference

This is the one dedicated output directory for frame-level analysis of
`screen-capture.webm`. Its PNGs and extracted reference audio are evidence for
timing and choreography only. They are never copied into `assets/` and are not
runtime game resources.

Use `tools/extract_webm_storyboard.py` for change-detected storyboard frames or
`tools/extract_capture_10hz.ps1` for a regular ffmpeg sample. Windows-playable
reference audio belongs here as `capture-audio.wav`, not in the native asset
tree.
