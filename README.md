## VIDEO DEMO

[![video demo](https://img.youtube.com/vi/2Yf_n_ua53o/0.jpg)](https://youtu.be/2Yf_n_ua53o?t=196)

## Building gestural music interfaces using machine leanring.

Implementing hands free or touch free musical instruments/interfaces with precision and accuracy is a challenging problem in music/audio technology. 
Thanks to the rise of modern AI (machine learning in specific), small instruments have more power than we can imagine. Combine it with the wide range of Digital Audio Workstations, we have a complete instrument that can be played, triggered or mixed live by a musician.

## Real Time Machine Learning:

I used real time programming and gesture recognition to create a complete instrument that can be used a MIDI trigger, a sine wave theremin, Digital Audio Effects Pedal. The gestures are used along with palm and finger tip movements in 3 dimensions (X, Y  and Z) or pitch, roll and yaw. 

Gesture Recognition with Machine Learning:
- Use hand recognition data from Leap Motion by processing video stream.
- Build a machine learning model that can classify the gestures.
- Build feature set using finger and palm euclidean distances.
- Support Vector Machines does well with complex datasets and limited dat
- Convert gestures, horizontal and vertical distances into MIDI triggers and Control Changes

## Real Time Machine Learning:
- Map the MIDI triggers to Ableton Live or any other daw to create and shape sound
- Through a glitch in the code, I found out it is easy to affect 2 control changes at the same time. This can be used to control 2 filters with the same knob or in this case the same gestural movement.

### References

- [Leap Motion](https://www.leapmotion.com/)
- [Max](https://cycling74.com/products/max/)
- [Ableton Live](https://www.ableton.com/en/)
- [ml.lib](http://alimomeni.net/ml.lib)
