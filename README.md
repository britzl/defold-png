# defold-png
Defold native extension to load and save PNG images using [lodepng](https://github.com/lvandeve/lodepng).

# Installation
You can use defold-png in your own project by adding this project as a Defold library dependency. Open your game.project file and in the dependencies field under project add:

https://github.com/britzl/defold-png/archive/master.zip

# Usage

__png.decode_rgba(bytes)__

Decode a PNG into a [Defold buffer](https://www.defold.com/ref/buffer/) containing the raw RGBA pixel data of the PNG. Once you have the buffer it's possible to get and manipulate the raw pixel data in Lua using `buffer.get_stream()` or set it as a texture on a sprite:

```
	-- load a png from disk
	local f = io.open("my.png", "rb")
	local bytes = f:read("*a")

	-- decode the png to RGBA buffer
	local buf, w, h = png.decode_rgba(bytes)

	-- change the texture on a sprite to the loaded png
	local resource_path = go.get("#sprite", "texture0")
	local header = { width = w, height = h, type = resource.TEXTURE_TYPE_2D, format = resource.TEXTURE_FORMAT_RGBA, num_mip_maps = 1 }
	resource.set_texture(resource_path, header, buf)
```

__png.decode_rgb(bytes)__

Decode a PNG into a [Defold buffer](https://www.defold.com/ref/buffer/) containing the raw RGB pixel data of the PNG. Once you have the buffer it's possible to get and manipulate the raw pixel data in Lua using `buffer.get_stream()` or set it as a texture on a sprite:

```
	-- load a png from disk
	local f = io.open("my.png", "rb")
	local bytes = f:read("*a")

	-- decode the png to RGBA buffer
	local buf, w, h = png.decode_rgba(bytes)

	-- change the texture on a sprite to the loaded png
	local resource_path = go.get("#sprite", "texture0")
	local header = { width = w, height = h, type = resource.TEXTURE_TYPE_2D, format = resource.TEXTURE_FORMAT_RGB, num_mip_maps = 1 }
	resource.set_texture(resource_path, header, buf)
```

***png.info(bytes)***

Read the PNG header and return information about the PNG.

```
	-- load a png from disk
	local f = io.open("my.png", "rb")
	local bytes = f:read("*a")

	local info = png.info(bytes)
	print(info.width)
	print(info.height)
	print(info.colortype)
	print(info.bitdepth)
```

***png.encode_rgb(pixels, width, height)***

Encode raw RGB pixels to a PNG with the same color type.

```
	-- create 100x100 red pixels
	local w = 100
	local h = 100
	local pixels = string.rep(string.char(255,0,0), w * h)

	-- encode
	local bytes = png.encode_rgb(pixels, w, h)

	-- save png to file
	local f = io.open("my.png", "wb")
	f:write(bytes)
	f:flush()
	f:close()
```

***png.encode_rgba(pixels, width, height)***

Encode raw RGBA pixels to a PNG with the same color type.

```
	-- create 100x100 red pixels with 50% transparency
	local w = 100
	local h = 100
	local pixels = string.rep(string.char(255,0,0, 128), w * h)

	-- encode
	local bytes = png.encode_rgba(pixels, w, h)

	-- save png to file
	local f = io.open("my.png", "wb")
	f:write(bytes)
	f:flush()
	f:close()
```

***png.RGB*** - Red, green and blue: rgb/truecolor

***png.RGBA*** - Red, green, blue and alpha

***png.GREY*** - Grayscale

***png.GREY_ALPHA*** - Grayscale and alpha: level of opacity for each pixel

***png.PALETTE*** - Indexed: channel containing indices into a palette of colors
