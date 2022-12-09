ifilter
=======
Image filters

Installation
-------------
Clone the repository using `git/clone`
```sh
% cd ifilter
% mk
% mk install
```

Usage
-----
```sh
% png -9 image.png | image/cfilter -f grayscale | image/dither | topng > out.png
```
See `ifilter(1)` for documentation.

Author
------
phil9

License
-------
MIT

