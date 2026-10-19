### Software:

The idea is that we can take the data saved locally on the device and somehow. 
- we need a way to transport/display this data. for the end user. I don't know if this should be a mobile application which is just an endpoint for us.
- We ideally also want a recap to either be through email, a website/app someone can look at to see there history. and then they can export there files to a format they want. 

As an initial mvp I suspect some minimal application, mobile or web based if fine. We connect to the device, extract the recordings from the device, use some language to text model (free model), create a document that has that days notes. 

This will include the following:
- Date
- Timestamps
- "Recording in text format"


This "raw" format could potentially be the free model and then we can have an AI model parse this, take out important findings. etc. 


# TODO
- determine the OS we are going to develop in. 