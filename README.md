# Audio Capture and Playback

This is an application which captures audio input from MIC and playback it through speaker and perform echo noise cancellation on audio data.
It uses ALSA interface, and configure both capture and playback device for 16 bit, 44.1Khz, 2 channels and interleaved mode of access.

## Dependencies:
- lasound
### Install Dependencies:
sudo apt-get install libasound2-dev

## Building Package:
### To Build package
- $ make all

### To clean package
- $ make clean

#### Example Usage:
 - ./playback <audio_device(OPTIONAL)>


