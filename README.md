### MaskTools 2 ###

This is a repo for my games with MaskTools 2. Most important changes are listed in the [wiki](https://github.com/tp7/masktools/wiki/Difference-to-masktools2-a48).

Current roadmap is pretty simple:
* Add AVX2 support to most filters. It might speedup luts quite a lot and luts are important.
* Consider adding multithreading support. All commonly used filters are optimized to the extend where multithreading might not help at all, so this feature might not get implemented.
* Consider adding high bitdepth support. This is complex and time-consuming, but might simplify dither package a lot.

Feel free to create an issue here or contact me on irc (tp7 on _freenode_ or _rizon_) if you want anything implemented or have some interesting ideas.

### License ###
Everything is  [GPL v2](http://www.gnu.org/licenses/gpl-2.0.html). Not because I want it this way or anything.
