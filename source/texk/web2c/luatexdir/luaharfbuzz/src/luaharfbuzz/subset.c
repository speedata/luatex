

#include <hb-subset.h>

#include "luaharfbuzz.h"

int subset_create(lua_State *L) {
  hb_subset_input_t *input;
  input = hb_subset_input_create_or_fail();
  if (input == NULL) {
    lua_pushnil(L);
  } else {
    hb_subset_input_t **ud =
        (hb_subset_input_t **)lua_newuserdata(L, sizeof(hb_subset_input_t *));
    *ud = input;
    luaL_getmetatable(L, "harfbuzz.subset");
    lua_setmetatable(L, -2);
  }
  return 1;
}

// hb_subset_input_pin_axis_location
static int subset_input_pin_axis_location(lua_State *L) {
  // arguments: input, face, tag, value
  hb_subset_input_t *input =
      *(hb_subset_input_t **)luaL_checkudata(L, 1, "harfbuzz.subset");
  Face *face = (Face *)luaL_checkudata(L, 2, "harfbuzz.Face");
  if (*face == NULL) {
    luaL_error(L, "Face is NULL");
    return 0;
  }
  if (hb_face_get_empty() == *face) {
    luaL_error(L, "Face is empty");
    return 0;
  }
  if (hb_face_get_glyph_count(*face) == 0) {
    luaL_error(L, "Face has no glyphs");
    return 0;
  }
  if (lua_type(L, 3) != LUA_TUSERDATA) {
    luaL_error(L, "Expected Tag userdata");
    return 0;
  }
  if (lua_type(L, 4) != LUA_TNUMBER) {
    luaL_error(L, "Expected number for axis value");
    return 0;
  }

  Tag *tag = (Tag *)luaL_checkudata(L, 3, "harfbuzz.Tag");
  lua_Number value = luaL_checknumber(L, 4);
  // input, face, tag, and value
  hb_bool_t ok = hb_subset_input_pin_axis_location(input, *face, *tag, value);
  // return ok
  lua_pushboolean(L, ok);
  return 1;
}

// hb_subset_input_keep_everything
static int subset_input_keep_everything(lua_State *L) {
  // arguments: input only
  hb_subset_input_t *input =
      *(hb_subset_input_t **)luaL_checkudata(L, 1, "harfbuzz.subset");
  hb_subset_input_keep_everything(input);
  return 0;
}


int subset_or_fail(lua_State *L) {
  // arguments: input and face
  hb_subset_input_t *input =
      *(hb_subset_input_t **)luaL_checkudata(L, 1, "harfbuzz.subset");
  Face *face = (Face *)luaL_checkudata(L, 2, "harfbuzz.Face");
  if (*face == NULL) {
    luaL_error(L, "Face is NULL");
    return 0;
  }
  if (hb_face_get_empty() == *face) {
    luaL_error(L, "Face is empty");
    return 0;
  }
  if (hb_face_get_glyph_count(*face) == 0) {
    luaL_error(L, "Face has no glyphs");
    return 0;
  }
  // perform the subset
  hb_face_t* subset_face = hb_subset_or_fail(*face, input);
  if (subset_face == NULL) {
      hb_subset_input_destroy(input);
      hb_face_destroy(*face);
      luaL_error(L, "hb_subset_or_fail failed");
      return 0;
  }
  // create a new userdata for the subset face
  hb_face_t **ud = (hb_face_t **)lua_newuserdata(L, sizeof(hb_face_t *));
  *ud = subset_face;
  luaL_getmetatable(L, "harfbuzz.Face");
  lua_setmetatable(L, -2);
  return 1;
}

int subset_input_unicode_set(lua_State *L) {
  hb_subset_input_t *input =
      *(hb_subset_input_t **)luaL_checkudata(L, 1, "harfbuzz.subset");
  hb_set_t *unicode_set = hb_subset_input_unicode_set(input);
  if (unicode_set == NULL) {
    lua_pushnil(L);
  } else {
    // add metatable for hb_set_t
    hb_set_t **ud = (hb_set_t **)lua_newuserdata(L, sizeof(hb_set_t *));
    *ud = unicode_set;
    luaL_getmetatable(L, "harfbuzz.Set");
    lua_setmetatable(L, -2);
  }
  return 1;
}

// implement hb_set_add for unicode
static int subset_set_add(lua_State *L) {
  hb_set_t *set = *(hb_set_t **)luaL_checkudata(L, 1, "harfbuzz.Set");
  unsigned int codepoint = (unsigned int)luaL_checkinteger(L, 2);
  hb_set_add(set, codepoint);
  return 0;
}

static const struct luaL_Reg set_methods[] = {{NULL, NULL}};

// register the set method add
static const luaL_Reg set_functions[] = {{"add", subset_set_add}, {NULL, NULL}};

// register the namespace "harfbuzz.Set" and associate ith with the set_methods
int register_set(lua_State *L) {
  return register_class(L, "harfbuzz.Set", set_functions, set_functions, NULL);
}

static const struct luaL_Reg subset_methods[] = {
    {"unicode_set", subset_input_unicode_set},
    {"pin_axis_location", subset_input_pin_axis_location},
    {"subset_keep_everything", subset_input_keep_everything},
    {"subset",subset_or_fail},
    {"create", subset_create},
    {NULL, NULL}};

static const struct luaL_Reg subset_functions[] = {{"create", subset_create},
                                                   {NULL, NULL}};

int register_subset(lua_State *L) {
  return register_class(L, "harfbuzz.subset", subset_methods, subset_functions,
                        NULL);
}
