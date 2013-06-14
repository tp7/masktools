# pre-release vmToon v0.7
# used to be known as mfToon
# pre-release because vectrangle is in the opposite timezone of me ;)
# if you're wondering why 0.7 and not 0.6, mftoon 0.6 was never released

function mf_min(string a, string b)
{
return a + " " + b + " < " + a + " " + b + " ?"
}

function mf_max(string a, string b)
{
return a + " " + b + " > " + a + " " + b + " ?"
}

function mf_str_level(string x, int in_low, int in_high, int out_low, int out_high)
{
return mf_max(mf_min(x + " 16 - 255 * 219 / " + string(in_low) + " - " + string(in_high-in_low) + " / " + string(out_high - out_low) + " * " + string(out_low) + " + 219 * 255 / 16 +", "235"), "16")
}

function vmToon(clip input, int "strength", int "luma_cap", int "threshold", int "thinning",     \
bool "sharpen", bool "mask", bool "show", string "showclip", int "ssw", int "ssh", int "xstren", \
int "xthresh") {

# vectrangle's stuff

str      = String(Default(strength, 48) /128.) # line darkening amount, 0-255
lum      = String(Default(luma_cap, 191))      # bright limit for line detection, 0-255 (255 = no limit)
thr      = String(Default(threshold, 4))       # threshold to disable slight darkening (of noise) 0-255
thinning =        Default(thinning,24)         # line thinning amount, 0-255
thn      = String(thinning /16.)

# mf's stuff

sharpen  = Default(sharpen, true)     # sharpening on/off
mask     = Default(mask, true)        # masking on/off
show     = Default(show, false)       # show the showclip or the output, true/false
showclip = Default(showclip, "lines") # show the lines or something else
ssw      = Default(ssw, 4)            # supersample factor horizontally, 0-inf
ssh      = Default(ssh, 4)            # supersample factor vertically, 0-inf
xstren   = Default(xstren, 255)       # xsharpening strength, 0-255
xthresh  = Default(xthresh, 255)      # xsharpening threshold, 0-255

mt_lutxy(input, input.blur(1.0), mf_str_level("x y - abs 128 +", 132, 145, 0, 255), y=3, u=1, v=1).blur(1.0).mt_lut(mf_str_level("x", 0, 64, 0, 255), y=3)

##
edgemask = last

# vectrangle's stuff

exin     = input.mt_Expand().mt_Inpand()	
diff     = mt_lutxy(input, exin, YExpr= "y "+lum+" < y "+lum+" ? x "+thr+" + > x y " \
	   +lum+" < y "+lum+" ? - 0 ? 127 +")
linemask = mt_lutxy(diff.mt_Inpand(), "x 127 - "+thn+" * 255 +")                          \
           .mt_Convolution("1 1 1", "1 1 1", Y=3)
thick    = mt_lutxy(input, exin, YExpr= "y "+lum+" < y "+lum+" ? x "+thr+" + > x y "  \
           +lum+" < y "+lum+" ? - 0 ? "+str+" * x +")
thin     = mt_lutxy(input.mt_Expand(), diff, YExpr="x y 127 - "+str+" 1 + * +")
(thinning == 0) ? thick : mt_merge(thin, thick, linemask, Y=3, U=1, V=1)
darkened = last

# mf's stuff

MergeChroma(Overlay(input, BlankClip(input, color=$00FF00), mask=edgemask))
lines = last

mt_merge(input, darkened, edgemask, Y=3, U=2, V=2)
masked = last

mask ? masked : mergechroma(darkened, input)
LanczosResize(width*ssw, height*ssh)
XSharpen(xstren, xthresh)
LanczosResize(width/ssw, height/ssh)
sharpened = last

showclip2 = Eval(showclip)
sharpen ? sharpened  : mask ? masked : darkened
show    ? showclip2  : last

}