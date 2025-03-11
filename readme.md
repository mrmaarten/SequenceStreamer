# SequenceStreamer
### Plays image sequences from a folder and stream them to other apps with Syphon

This app is on mac with _openFrameworks_. The app is as follows:
- user assigns a folder to read
- from that folder all images are read
- all images are played in sequence, set by a speed interval
- images are played in real time from disk
- there is a UI 
    - shows range of images
    - range can be set by user and range is updated live when more image are added to the folder
    - speed, with fixed settings
- the output of the image playback is a window, and a syphon server 
- user can also toggle a black screen
- new UI > range slider etc
https://github.com/braitsch/ofxDatGui
- ping pong toggle
- remember last speed: use that to toggle pause and play

Todo
- brightness slider
- contrast slider
- saturation slider

- play last x frames: 5, 10, 100, user input