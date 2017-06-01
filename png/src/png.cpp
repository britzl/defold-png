// Extension lib defines
#define LIB_NAME "Png"
#define MODULE_NAME "png"

#define DLIB_LOG_DOMAIN "PNG"

#include <dmsdk/sdk.h>
#include "lodepng.h"
#include "png_private.h"

/**
 * Encode raw pixels (8 bits per component) to a PNG
 */
static int Encode(lua_State* L, LodePNGColorType type) {
    int top = lua_gettop(L);

    const char* pixels = luaL_checkstring(L, 1);
    const int w = luaL_checkint(L, 2);
    const int h = luaL_checkint(L, 3);

    // encode to png
    unsigned char* out = 0;
    size_t outsize = 0;
    lodepng_encode_memory(&out, &outsize, (unsigned char*)pixels, w, h, type, 8);

    // put the pixel data onto the stack
    lua_pushlstring(L, (char*)out, outsize);

    assert(top + 1 == lua_gettop(L));
    return 1;
}

/**
 * Convert RGB pixel data to a PNG of the same colortype
 */
static int EncodeRGB(lua_State* L) {
    return Encode(L, LCT_RGB);
}

/**
 * Convert RGBA pixel data to a PNG of the same colortype
 */
static int EncodeRGBA(lua_State* L) {
    return Encode(L, LCT_RGBA);
}

/**
 * Convert PNG to a buffer
 */
static int ToBuffer(lua_State* L, LodePNGColorType type) {
    int top = lua_gettop(L);

    size_t png_length;
    const char* png = luaL_checklstring(L, 1, &png_length);

    // decode png to pixels
    unsigned char* pixels = 0;
    uint32_t outw = 0;
    uint32_t outh = 0;
    uint32_t bytes_per_pixel;
    lodepng::State state;
    state.decoder.color_convert = 1;
    state.info_raw.bitdepth = 8;
    switch(type) {
        case LCT_RGBA:
            state.info_raw.colortype = LCT_RGBA;
            bytes_per_pixel = 4;
            break;
        case LCT_RGB:
        default:
            state.info_raw.colortype = LCT_RGB;
            bytes_per_pixel = 3;
            break;
    }
    lodepng_decode(&pixels, &outw, &outh, &state, (unsigned char*)png, png_length);

    // flip vertically
    for (int yi=0; yi < (outh / 2); yi++) {
        for (int xi=0; xi < outw; xi++) {
            unsigned int offset1 = (xi + (yi * outw)) * bytes_per_pixel;
            unsigned int offset2 = (xi + ((outh - 1 - yi) * outw)) * bytes_per_pixel;
            for (int bi=0; bi < bytes_per_pixel; bi++) {
                unsigned char byte1 = pixels[offset1 + bi];
                unsigned char byte2 = pixels[offset2 + bi];
                pixels[offset1 + bi] = byte2;
                pixels[offset2 + bi] = byte1;
            }
        }
    }

    // create buffer
    dmBuffer::HBuffer buffer;
    dmBuffer::StreamDeclaration streams_decl[] = {
    {
        dmHashString64("pixels"), dmBuffer::VALUE_TYPE_UINT8, 1 }
    };
    dmBuffer::Result r = dmBuffer::Create(outw * outh * bytes_per_pixel, streams_decl, 1, &buffer);
    if (r == dmBuffer::RESULT_OK) {
        // copy pixels into buffer
        uint8_t* data = 0;
        uint32_t datasize = 0;
        dmBuffer::GetBytes(buffer, (void**)&data, &datasize);
        memcpy(data, pixels, datasize);
        delete pixels;

        // validate and return
        if (dmBuffer::ValidateBuffer(buffer) == dmBuffer::RESULT_OK) {
            dmScript::LuaHBuffer luabuffer = { buffer, true };
            dmScript::PushBuffer(L, luabuffer);
            lua_pushnumber(L, outw);
            lua_pushnumber(L, outh);
        }
        else {
            lua_pushnil(L);
            lua_pushnil(L);
            lua_pushnil(L);
        }
    }
    // buffer creation failed
    else {
        delete pixels;
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
    }

    assert(top + 3 == lua_gettop(L));
    return 3;
}

/**
 * Convert PNG to an RGB buffer
 */
static int DecodeRGB(lua_State* L) {
    return ToBuffer(L, LCT_RGB);
}

/**
 * Convert PNG to an RGBA buffer
 */
static int DecodeRGBA(lua_State* L) {
    return ToBuffer(L, LCT_RGBA);
}

/**
 * Get information about a PNG
 */
static int Info(lua_State* L) {
    int top = lua_gettop(L);

    size_t png_length;
    const char* png = luaL_checklstring(L, 1, &png_length);

    // decode png to pixels
    uint32_t outw = 0;
    uint32_t outh = 0;
    lodepng::State state;
    lodepng_inspect(&outw, &outh, &state, (unsigned char*)png, png_length);

    // return table with PNG info
    lua_newtable(L);
    lua_pushstring(L, "width");
    lua_pushnumber(L, outw);
    lua_rawset(L, -3);
    lua_pushstring(L, "height");
    lua_pushnumber(L, outh);
    lua_rawset(L, -3);
    lua_pushstring(L, "colortype");
    lua_pushnumber(L, state.info_png.color.colortype);
    lua_rawset(L, -3);
    lua_pushstring(L, "bitdepth");
    lua_pushnumber(L, state.info_png.color.bitdepth);
    lua_rawset(L, -3);

    assert(top + 1 == lua_gettop(L));
    return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] = {
    {"encode_rgb", EncodeRGB},
    {"encode_rgba", EncodeRGBA},
    {"decode_rgb", DecodeRGB},
    {"decode_rgba", DecodeRGBA},
    {"info", Info},
    {0, 0}
};

static void LuaInit(lua_State* L) {
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    #define SETCONSTANT(name) \
        lua_pushnumber(L, (lua_Number) name); \
        lua_setfield(L, -2, #name);\

        SETCONSTANT(RGBA)
        SETCONSTANT(RGB)
        SETCONSTANT(GREY)
        SETCONSTANT(GREY_ALPHA)
        SETCONSTANT(PALETTE)
    #undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializePngExtension(dmExtension::AppParams* params) {
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializePngExtension(dmExtension::Params* params) {
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizePngExtension(dmExtension::AppParams* params) {
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizePngExtension(dmExtension::Params* params) {
    return dmExtension::RESULT_OK;
}


// Defold SDK uses a macro for setting up extension entry points:
//
// DM_DECLARE_EXTENSION(symbol, name, app_init, app_final, init, update, on_event, final)

DM_DECLARE_EXTENSION(Png, LIB_NAME, AppInitializePngExtension, AppFinalizePngExtension, InitializePngExtension, 0, 0, FinalizePngExtension)
