
 ##############################
 ##############################
 ##                          ##
 ##  mfToon v0.52 by mf ^^;  ##
 ##                          ##
 ## I would claim copyright, ##
 ## but as I do not give my  ##
 ## real identity, I can't.  ##
 ## So as netizen to net-    ##
 ## izen, I ask you, please  ##
 ## don't do lame stuff      ##
 ## with my script, like     ##
 ## claiming you made it.    ##
 ## Thank you.               ##
 ##                          ##
 ##############################
 ##############################
               ##
               ##
################################
################################
##                            ##
## Usage:                     ##
##                            ##
## Import("mfToon-v0.52.avs") ##
## mfToon() or mfToonLite()   ##
##                            ##
################################
################################

####
## 
## New maskbuilding ("cartoon" edgedetection), and mfToon-lite to use in realtime in your
## ffdshow "AVISynth" section. It's not meant for encoding, only playback.
##
##

function mfToon2(clip orig, int "twidth", int "theight", int "ssw", int "ssh", int "xstren", \
int "xthresh", bool "cwarp", bool "sharpen", int "strength", float "wdepth", int "wblur",   \
float "wthresh", int "drange", float "dboost", int "dlimit", bool "debug", bool "doutput",  \
string "dclip", bool "show", int "scolor") {

# normal params

strength = Default(strength, 255)	 # line darkening strength, 0-255
sharpen  = Default(sharpen, true)	 # sharpening on/off
cwarp    = Default(cwarp, true)		 # chroma warp on/off
cwarp2   = cwarp ? 1 : 0
wdepth   = Default(wdepth, 16.0)	 # warping depth, ?-?
wblur    = Default(wblur, 1)		 # warping blur level, ?-?
wthresh  = Default(wthresh, 0.5)	 # warping threshold, 0.0-1.0
ssw      = Default(ssw, 4)     		 # supersample factor horizontally, 0-inf
ssh      = Default(ssh, 4)		 # supersample factor vertically, 0-inf
twidth 	 = Default(twidth, orig.width)	 # target width (useful for cropping), 0-inf
theight	 = Default(theight, orig.height) # target height (useful for cropping), 0-inf
show     = Default(show, false)		 # show which lines are being darkened
scolor	 = Default(scolor, $FF00FF)	 # which color to show lines in

# advanced params

xstren   = Default(xstren, 255)   	 # xsharpening strength, 0-255
xthresh  = Default(xthresh, 255)  	 # xsharpening threshold, 0-255

## Where have all the params gone?! - They were useless as of 0.5, but I forgot to clean them.

drange   = Default(drange, 64)    	 # detail range, 0-255
dboost   = Default(dboost, 1.0)   	 # detail boost, 0.1-10.0
dlimit   = Default(dlimit, 30)    	 # detail limiter, 0-255
debug    = Default(debug, false)	 # debug mode on/off
doutput  = Default(doutput, true)	 # print debug info on processed image
dclip    = Default(dclip, "rclip1")	 # which stage of the processing to show

ssw2     = twidth  * ssw
ssh2     = theight * ssh
sharpen2 = show    ? false : sharpen


orig.Unsharpmask(300, 4, 0)
sharp = last
mt_merge(sharp, orig, orig)
sharp1 = last
mt_merge(sharp1, orig, orig)
sharp2 = last

greymask = orig.mt_invert(u=-128, v=-128)

detailmaskpre0 = orig.mt_edge(thy1 = 3, thy2 = 255, thc1 = 255, thc2 = 255, mode = "cartoon", Y=3, V=1, U=1) \
.Tweak(0.0, 1.0, drange, 1.0).Levels(60, dboost, 255, 0, 255) \
.Levels(0, dboost, dlimit, 255, 0).mt_inflate(u=-128, v=-128).mt_deflate().mt_deflate().mt_deflate()
detailmaskpre1 = orig.mt_edge(thy1 = 3, thy2 = 255, thc1 = 255, thc2 = 255, mode = "roberts", Y=3, V=1, U=1) \
.Tweak(0.0, 1.0, drange, 1.0).Levels(60, dboost, 255, 0, 255) \
.Levels(0, dboost, dlimit, 255, 0).mt_inflate(u=-128, v=-128)
detailmaskpre2 = mt_lut(detailmaskpre1, "x x * x * x * 256 / 256 / 256 /").Blur(1.0).Levels(0, 1.0, 190, 0, 255)
mt_deflate()
detailmask = last
white = orig.mt_binarize(Y=-255,U=-128,V=-128)
linemask1 = mt_merge(white, detailmask, orig.mt_invert()).mt_invert()
linemask = (strength == 255) ? linemask1 : linemask1.levels(0, 1.0, 255, 0, strength)

color = BlankClip(orig, color=scolor)
sharp3 = show ? color : sharp2

dark = mt_merge(orig, sharp3, linemask, Y=3, U=1, V=1)
darkmerged = dark.MergeChroma(orig)
finaldark = show ? dark : darkmerged

semifinal=orig.BicubicResize(twidth, theight, 0, 0.75)

final = dark.BicubicResize(ssw2, ssh2, 0, 0.75).XSharpen(xstren, xthresh) \
.BicubicResize(twidth, theight, 0, 0.75).MergeChroma(semifinal) \
.aWarpSharp(cm=cwarp2, depth=wdepth, blurlevel=wblur, thresh=wthresh)

      rclip1  = sharpen2 ?   final    : finaldark
Eval("dclip1  = doutput  ? "+dclip+"  : orig")

dstring1 = "mfToon v0.52 by mf - Debug mode ;p"
dstring2 = "Pointsized: DisabledxDisabled"
dstring3 = "Bicubicsized: " + String(ssw2) + "x" + String(ssh2)
dstring4 = "Outputsize: " + String(twidth) + "x" + String(theight)
dstring5 = "Debug image: " + dclip
clipstring = sharpen2 ? "final" : "finaldark"
dstring6 = "Output image: " + clipstring

      dclip2  = dclip1.Subtitle(dstring1).Subtitle(dstring2, y=33).Subtitle(dstring3, y=48) \
                .Subtitle(dstring4, y=63).Subtitle(dstring5, y=78).Subtitle(dstring6, y=93)
      rclip2  = debug   ?   dclip2   : rclip1

return rclip2
}

function mfToonLite(clip orig, int "twidth", int "theight", int "strength", int "dstren", int \
"drange", float "dboost", int "dlimit", string "mask") {

# normal params

strength = Default(strength, 255)	 # line darkening strength, 0-255
mask     = Default(mask, "fastest")	 # quality of the mask

# advanced params

dstren   = Default(dstren, 255)   	 # detail strength, 0-255
drange   = Default(drange, 64)    	 # detail range, 0-255
dboost   = Default(dboost, 1.0)   	 # detail boost, 0.1-10.0
dlimit   = Default(dlimit, 30)    	 # detail limiter, 0-255

orig.Unsharpmask(200, 4, 0)
sharp = last
mt_merge(sharp, orig, orig)
sharp1 = last
mt_merge(sharp1, orig, orig)
sharp2 = last

greymask = orig.mt_invert(u=-128,v=-128)
white = orig.mt_binarize(Y=-255,U=-128,V=-128)

## slowest

detailmaskpre = orig.mt_edge(thy1 = 3, thy2 = 255, thC1 = 255, thc2 = 255, mode = "sobel", Y=3, V=1, U=1) \
.Tweak(0.0, 1.0, drange, 1.0).Levels(60, dboost, 255, 0, 255) \
.Levels(0, dboost, dlimit, 255, 0).mt_inflate(u=-128, v=-128)
detailmaskpre2 = mt_lut(detailmaskpre, "x x * x * x * 256 / 256 / 256 /").Blur(1.0).Levels(0, 1.0, 190, 0, 255)
mt_deflate()
detailmask = last
linemask1 = mt_merge(white, detailmask, orig.mt_invert()).mt_invert()
linemaskslowest = (strength == 255) ? linemask1 : linemask1.levels(0,1.0,255,0,dstren)

## fastest

orig.Unsharpmask(100).mt_edge(thy1 = 3, thy2 = 255, thC1 = 255, thc2 = 255, mode = "sobel", Y=3, V=1, U=1) \
.mt_inflate(u=-128, v=-128).mt_deflate()
detailmask = last
linemask1 = mt_merge(white, detailmask, orig.mt_invert()).mt_invert()
linemaskfastest = (strength == 255) ? linemask1 : linemask1.levels(0,1.0,255,0,dstren)

linemask = (mask == "fastest") ? linemaskfastest : linemaskslowest

dark = mt_merge(orig, sharp2, linemask, Y=3, U=2, V=2)

return dark
}