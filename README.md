A small Unity game image export tool.
================================

## Dependencies
Qt library
## Features:
* Single File Mode
```
    UnityImageOut.exe [filename] [type]
```

    [type] only take effect when the texture is 16bpp
    type will be 0, i.e. rgba4444 when not set
    1 = argb1555
    2 = rgb565

* Batch Mode with multithreading
    
    automatically transform the *.bin file in the current folder Using Alpha8,rgba4444,rgb888,rgba8888

## Download Pages
[google code](https://code.google.com/p/unityimagesuite/downloads/list)
