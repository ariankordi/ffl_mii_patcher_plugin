# NxInVer3Pack / "eFFSD"
This is a custom Mii data format that I originally developed in December 2024. Or is it a steganography scheme? Compression? Whatever you want to call it.

It allows storing extra data, including new Switch-exclusive colors, in 3DS/Wii U Mii data (FFLStoreData/Ver3StoreData).

**WARNING: Currently this format has no checksum or reliable way to detect it. Until it does, PLEASE don’t unironically use this.**

(If you want to anyway, please contact me)

## How does it work?
The extra data is stored across fields that are never used. Nintendo officially calls them "padding", and they are for bitfield alignment.
Because of this, the extra data doesn't take up any more room and is fully backwards compatible.

To pack the new colors in a smaller space, it takes advantage of how Switch colors are downgraded to older "Ver3" colors. Knowing the old color narrows the range from 100 possible colors to 32, so only the "group index" is stored.

(This also means the conversion tables differ slightly, see NxInVer3Pack.hpp.)

If you want to understand this better better, see [this JSFiddle that roughly illustrates what I'm talking about.](https://jsfiddle.net/arian_/n8fesLdj/3/) Also see an earlier (incompatible) sample for this format in JS: https://jsfiddle.net/arian_/hwokt9sr/4/

The extra data itself is tricky to read from/write to. The unused bitfields are treated together as one buffer ("ExtraDataBlock"). On top of that, the custom colors themselves ("GroupIndices") are packed into bits.

But, the methods for packing/unpacking padding data and extra data are split into separate functions in the "NxInVer3" namespace.
## Building
Currently, this is implemented in C++20. I plan to reimplement it in [the Fusion Programming Language](https://fusion-lang.org/) later on, so it’ll be more portable.

There’s a CLI and Google Tests, but no specific build instructions right now. Since there's also no dependencies, you should be able to just build it:

`g++ -std=c++20 -g -I. src/NxInVer3Pack.cpp ./NxInVer3PackCli.cpp -o NxInVer3PackCli`

Example:
`echo -n "AwAAQGQ0OliAJ4ZL1x8zGO1WaS0MPQAAAShiAGwAYQBuAGMAbwAAAAAAAAAAABIAEhB7BFxuRByNZMcYAAgZJA0AIEGzW4NdAAAAAAAAAAAAAAAAAAAAAAAAAAAAALjJ" | base64 -d | ./NxInVer3PackCli pack /dev/stdin /dev/stdout 0 99 47 46 19 73 42 3 | base64`

## Why?
The FFSD format is, objectively, the best Mii format.
* Supports the most features - Switch only adds colors/new glass types, everything else is same. Wii has less parts available.
* Contains the most fields - Switch formats don't have birthdays, creator names, or copyable/mingling flags.
* The most common file format - 60M+ NNIDs, hundreds of thousands of Mii QR codes
* The only format you can transfer to/from the Switch. (amiibo)
    * *Technically, nn::mii::StoreData can be transferred over local wireless (LDN) via consoles only, but amiibo can be written/read from a phone.
* Tightly bit-packed. Only 72 bytes are needed - snip the rest, add the checksum back, and you're good.
* Easy to detect. Detect via the file size, or the mandatory "03" prefix and checksum. (Actually don't use the 03 because camera Miis from Wii U don't have it for some reason)
* The format has been around for 14+ years.

Why use anything newer? Why invent your own proprietary format?

eFFSD™ keeps the best Mii format relevant.

## Plans
In order for this format to actually be viable and last, a few things are needed.
* Checksum and flag bits

There's barely any room for this, but the remaining space should be used for a tiny checksum, as well as some reserved bits.
The reserved/flag space can be used to indicate if it stores a different kind of custom data, or just Switch colors.
* Reliable detection

We also need to be certain that custom data is really there. A tiny checksum will probably detect invalid data 60% of the time, but ideally we should see which bits are usually always set to zero, and use one of those to indicate if custom data may be there.
That can also be used by simple code that doesn't want to implement the entire format, to determine if it's a custom NxInVer3Pack file or not.
* Format name

I've thought of a handful of names for this format. "eFFSD" is one I gravitate towards, but I'm still not sure if it would be the best.
* 3DS support

The corresponding mod will be more challenging since there is no decomp for CFL.

But, one issue with 3DS support is that this currently uses the roomIndex/positionInRoom fields, which are needed for the Mii Maker database on 3DS only. We either can't use them, or somehow have to ignore them on 3DS.
* Tool + editor support

This is kinda out of scope, but for this format to work, it'll need some good use cases and be compatible with tools and Mii editors.

As of writing, there's only one accurate Mii editor out there which is mostly closed source. So, that is TBD.

There also isn't a great multi-tool out there for editing and converting Mii data. I guess the closest thing is mii-unsecure.ariankordi.net's conversion functionality, but it isn't fully fleshed out. That is also TBD.

## Potential Uses
I originally thought of this format because of the customization it would allow, while still staying backwards compatible.
* Kaerutomo currently doesn't restore Switch colors when you scan a QR code. Miitomo used to store that all server-side, but this format can be used to store the Mii data and its Switch colors in the QR code itself. The encoding/decoding from/to nn::mii::CoreData will just have to be implemented on the backend (in their case, PHP)
* Pretendo, or any custom NN, could allow users to add Switch colors to their Mii online. The Mii Studio API can render them, but with this format and the Aroma plugin, the colors could be shown on the console as well as online. They will also be shown in UGC.
* A potential mod for Tomodachi Life could allow hair dye colors to be used outside of the game.

But even if it doesn't involve consoles, anything that reads FFSD is compatible with custom NxInVer3Pack data.
* Custom tools, servers, and different Mii editors (potentially) can exchange these files and decode the custom colors. It can also be used in QR codes, so that they are the same size and don't need to awkwardly append unencrypted data at the end.
