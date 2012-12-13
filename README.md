RetinaTattoo is a software for live video streaming to LED strips. It is written in C++11 and consists of a client and server. The client can be substituted with common video streaming solutions (like gstreamer or vlc), but often the command line client is just simpler to use.

The server can be configured to match the hardware specifics  (size, pixel format...) of your led stripes and tune color characteristics (via HSL).

[WSyS](http://metalab.at/wiki/User:WSyS) and [me](http://metalab.at/wiki/User:amir) used it to stream and play super mario bros. from a nes emulator to wsys's LED Matrix.

## Examples ##

### Streaming video to you LED Matrix using gstreamer ###
Let's say you have a Raspberry PI connect via SPI to your LED Matrix and some videos on your notebook which you would like to display.
The following procedure scales and streams the video to the LED Matrix, and also mirrors the output to a local X11 window.

On the raspberry:
<pre>
# clone the RetinaTattoo repository
git clone https://github.com/Metalab/RetinaTattoo.git

# install dependencies
apt-get install build-essential libboost-system-dev libboost-thread-dev g++-4.7

# build it
cd RetinaTattoo
make

# start the server. in this example for a 24x24 led matrix with bgr pixel format and alternating scan line direction. additionally it slightly dims lightness.
./server -a -f bgr -d24x24 -l-10 8888
</pre>


On your notebook:
<pre>
#install gstreamer 
apt-get install gstreamer-tools gstreamer0.10-alsa gstreamer0.10-ffmpeg gstreamer0.10-plugins-base gstreamer0.10-plugins-good gstreamer0.10-plugins-ugly gstreamer0.10-x

# start the stream assuming your raspberry has the ip address 10.20.30.11
cd RetinaTattoo/scripts
./videostream.sh myvideo.avi 10.20.30.11 8888
</pre>

It's also possible to capture a X11 window and stream it directly to a LED Matrix, but i haven't written easy-to-use scripts for that yet. feel free to bug me :)
anyway, there's a video of us using the technique to play super mario bros. [Super Mario LED Matrix](http://vimeo.com/54252456)

== Featuring articles ==

* [@obviouswinner](http://www.obviouswinner.com/obvwin/2012/11/26/ultra-low-res-super-mario-bros-on-a-large-glowing-led-strip.html)
* [@jeuxvideo](http://www.jeuxvideo.org/index.php/2012/11/27/une-version-de-super-mario-bros-comme-vous-nen-avez-jamais-vu/)
* [@hiyoko-g](http://www.hiyoko-g.com/30349_led_mario/)
* [@estoapesta](http://estoapesta.com/2012/11/27/el-primer-juego-de-super-mario-bros-en-una-matriz-led-de-ultra-baja-resolucion/)


