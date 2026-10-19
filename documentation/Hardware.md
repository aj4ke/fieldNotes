## MVP technical:

### Hardware/device:
- small device prototyped on a breadboard, using a esp32 and a small microphone device. This will have 2 buttons. one button will be for start recording and stop recording. the second button will be for initiaiting a bluetooth connection to send to a device.
- as a first step see if we can just record audio in .wav or .mp3 format. Ideally whichever is the smaller file format and has clear enough audio but I would not worry about audio clarity too much rn.

Hardware questions I have:
- can we save timestamps when each audio is recorded? or extract that somehow. We should have some specific metadata format. 
- how much data can we save locally on a device like this? minutes or hours? do they have to offload data every day? 
- instead of bluetooth could we just have a usbc port on the device that we plug into a phone and transfer data that way? we should talk about that and would likely need to do some talking with actual potential customers? what does the end of day workflow look like. This also depends on the software endpoint we decide on, more about that in Software. 
- charging this device/battery powered. Questions for post mvp but putting it here for now. Lowkey should just be able to use one of those small disk batteries. 




# TODO
- specify the hardware we are using for the MVP (ex. micro controller, buttons, etc.) This should be done after some protoyping but can be changed in the future. 