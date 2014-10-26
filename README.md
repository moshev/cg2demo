Demo R&D
========

Written for MSVS2013. Also works on Linux and Mac.

Guidelines
==========

* Encoding of all files is utf-8.
* Indentation is 4 spaces. When using the space key to indent,
  it shall be pressed 4 times. One, two, three, four. Pressing it
  one, two or three times is not allowed, if not followed by
  pressing it a fourth time. Five is right out.
  However, every good text editor, and also Emacs, supports
  smart indentation via the tab key.
* After the 4 spaces, you must write well-thought-out code.
* A line shall be no longer than 120 symbols.

Recording
=========

* Run the demo and see in flog.txt the WIDTH and HEIGHT for your system
* mkfifo fvideoout
* mkfifo faudioout
* (replace WxH with the width and height from the first step) ffmpeg -f rawvideo -pix_fmt rgb24 -r 60 -video_size WxH -i fvideoout -vf 'vflip' -c:v libx264 -qmax 16 video.mp4
* ffmpeg -f s16le -ar 44100 -ac 1 -i fiaudioout -c:a ac3 audio.ac3
* cg2demo --record
* When done run: ffmpeg -i video.mp4 -i audio.ac3 -c:a copy -c:v copy -shortest combined.mp4

Competition
===========

We won first prize at the CG2 Demo Competition!

A rendered video is available at http://www.youtube.com/watch?v=F0EsAEaxN6E