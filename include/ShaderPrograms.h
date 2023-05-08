#ifndef SHADERPROGRAMS_H
#define SHADERPROGRAMS_H

#ifdef __BUILD_DESKTOP__
#define GLSL_VERSION_STRING "#version 120\n\n"
#else
#define GLSL_VERSION_STRING ""
#endif

//////////////////////////////////////////////////////////////////////
// This version uses a real depth texture as shadow map
//////////////////////////////////////////////////////////////////////

const char* DepthShadowMapVertexShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "attribute vec3 in_coord;\n" \
        "uniform mat4 mvp_matrix;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "   gl_Position = mvp_matrix * vec4(in_coord, 1.0);\n" \
        "}\n";

const char* DepthShadowMapFragmentShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "  //gl_FragColor = vec4(0.1, 1.0, 0.1, 1.0);\n" \
        "}\n";

const char* DepthDefaultVertexShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "attribute vec3 in_coord;\n" \
        "attribute vec2 in_texCoord;\n" \
        "attribute vec3 in_normal;\n" \
        "varying mediump vec2 ex_texCoord;\n" \
        "varying mediump vec4 ex_shadowCoord;\n" \
        "varying mediump vec3 ex_normal;\n" \
        "varying mediump vec3 ex_lightDir;\n" \
        "\n" \
        "uniform mediump mat4 mvp_matrix;\n" \
        "uniform mediump mat4 shadow_proj_matrix;\n" \
        "uniform highp vec3 light_pos;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "   gl_Position = mvp_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_shadowCoord = shadow_proj_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_normal = in_normal;\n" \
        "   ex_lightDir = light_pos - in_coord;\n" \
        "   ex_texCoord = in_texCoord;\n" \
        "}\n";

const char* DepthDefaultFragmentShader =
        GLSL_VERSION_STRING \
        "precision mediump float;\n" \
        "\n" \
        "varying mediump vec2 ex_texCoord;\n" \
        "varying mediump vec4 ex_shadowCoord;\n" \
        "varying mediump vec3 ex_normal;\n" \
        "varying mediump vec3 ex_lightDir;\n" \
        "\n" \
        "uniform lowp sampler2D texture;\n" \
        "uniform lowp sampler2D shadow_texture;\n" \
        "\n" \
        "float myShadowProj(vec4 coord)\n" \
        "{\n" \
        "  float shadowDepth = texture2D(shadow_texture, coord.st).z + 0.002;\n" \
        "  return step(coord.z, shadowDepth);\n" \
        "}\n" \
        "void main()\n" \
        "{\n" \
        "  vec4 texcol = texture2D(texture, ex_texCoord);\n" \
        "  vec4 unitCoord = ex_shadowCoord / ex_shadowCoord.w;\n" \
        "  float step = step(ex_shadowCoord.w, 0.0);\n" \
        "  float intensity = dot(normalize(ex_lightDir), ex_normal);\n" \
        "  intensity = max(intensity * max(myShadowProj(unitCoord), step), 0.3);\n" \
        "  gl_FragColor = vec4(intensity * texcol.rgb, texcol.a);\n" \
        "}\n";

//////////////////////////////////////////////////////////////////////
// This version renders the depth to a RGBA texture
//////////////////////////////////////////////////////////////////////

const char* ShadowMapVertexShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "attribute vec3 in_coord;\n" \
        "uniform mat4 mvp_matrix;\n" \
        "varying highp vec4 ex_coord;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "   gl_Position = mvp_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_coord = gl_Position;\n" \
        "}\n";

const char* ShadowMapFragmentShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "varying highp vec4 ex_coord;\n" \
        "\n" \
        "vec4 pack(float depth)\n" \
        "{\n" \
        "  const vec4 shift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);\n" \
        "  const vec4 mask = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);\n" \
        "  vec4 comp = fract(depth * shift);\n" \
        "  comp -= comp.xxyz * mask;\n" \
        "  return comp;\n" \
        "}\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "  float normalizedDepth = ex_coord.z / ex_coord.w;\n" \
        "  normalizedDepth += 0.002;\n" \
        "  gl_FragColor = pack(normalizedDepth);\n" \
        "}\n";

const char* DefaultVertexShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "attribute vec3 in_coord;\n" \
        "attribute vec2 in_texCoord;\n" \
        "attribute vec3 in_normal;\n" \
        "varying mediump vec2 ex_texCoord;\n" \
        "varying mediump vec4 ex_shadowCoord;\n" \
        "varying mediump vec3 ex_normal;\n" \
        "varying mediump vec3 ex_lightDir;\n" \
        "\n" \
        "uniform mediump mat4 mvp_matrix;\n" \
        "uniform mediump mat4 shadow_proj_matrix;\n" \
        "uniform highp vec3 light_pos;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "   gl_Position = mvp_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_shadowCoord = shadow_proj_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_normal = in_normal;\n" \
        "   ex_lightDir = light_pos - in_coord;\n" \
        "   ex_texCoord = in_texCoord;\n" \
        "}\n";

const char* DefaultFragmentShader =
        GLSL_VERSION_STRING \
        "precision mediump float;\n" \
        "\n" \
        "varying vec4 ex_color;\n" \
        "varying vec2 ex_texCoord;\n" \
        "varying vec4 ex_shadowCoord;\n" \
        "varying vec3 ex_normal;\n" \
        "varying vec3 ex_lightDir;\n" \
        "\n" \
        "uniform lowp sampler2D texture;\n" \
        "uniform lowp sampler2D shadow_texture;\n" \
        "\n" \
        "float myShadowProj(vec4 coord)\n" \
        "{\n" \
        "  const vec4 shift = vec4(1.0 / (256.0*256.0*256.0),\n" \
        "                          1.0 / (256.0*256.0),\n" \
        "                          1.0 / (256.0),\n" \
        "                          1.0);\n" \
        "  vec4 packedShadowDepth = texture2D(shadow_texture, coord.st);\n" \
        "  float shadowDepth = dot(packedShadowDepth, shift);\n" \
        "  return step(coord.z, shadowDepth);\n" \
        "}\n" \
        "void main()\n" \
        "{\n" \
        "  vec4 texcol = texture2D(texture, ex_texCoord);\n" \
        "  vec4 unitCoord = ex_shadowCoord / ex_shadowCoord.w;\n" \
        "  float step = step(ex_shadowCoord.w, 0.0);\n" \
        "  float intensity = dot(normalize(ex_lightDir), ex_normal);\n" \
        "  intensity = max(intensity * max(myShadowProj(unitCoord), step), 0.3);\n" \
        "  gl_FragColor = vec4(intensity * texcol.rgb, texcol.a);\n" \
        "}\n";

// This shader does not have any lighting applied
const char* NoLightVertexShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "attribute vec3 in_coord;\n" \
        "attribute vec2 in_texCoord;\n" \
        "varying mediump vec2 ex_texCoord;\n" \
        "\n" \
        "uniform mat4 mvp_matrix;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "   gl_Position = mvp_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_texCoord = in_texCoord;\n" \
        "}\n";

const char* NoLightFragmentShader =
        GLSL_VERSION_STRING \
        "precision mediump float;\n" \
        "\n" \
        "uniform lowp sampler2D texture;\n" \
        "varying mediump vec2 ex_texCoord;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "    gl_FragColor = texture2D(texture, ex_texCoord);\n" \
        "}\n";

// This shader is used for picking
const char* PickingVertexShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "attribute highp vec3 in_coord;\n" \
        "varying mediump vec4 ex_color;\n" \
        "\n" \
        "uniform mat4 mvp_matrix;\n" \
        "uniform vec3 pick_color;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "   gl_Position = mvp_matrix * vec4(in_coord, 1.0);\n" \
        "   ex_color = vec4(pick_color, 1.0);\n" \
        "}\n";

const char* PickingFragmentShader =
        GLSL_VERSION_STRING \
        "precision highp float;\n" \
        "\n" \
        "varying mediump vec4 ex_color;\n" \
        "\n" \
        "void main()\n" \
        "{\n" \
        "    gl_FragColor = ex_color;\n" \
        "}\n";

#endif // SHADERPROGRAMS_H
