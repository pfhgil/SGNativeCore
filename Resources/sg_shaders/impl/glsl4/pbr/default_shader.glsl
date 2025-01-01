#include "sg_shaders/impl/glsl4/uniform_bufs_decl.glsl"
#include "sg_shaders/impl/glsl4/random.glsl"
#include "sg_shaders/impl/glsl4/disks.glsl"
#include "sg_shaders/impl/glsl4/math.glsl"
#include "sg_shaders/impl/glsl4/defines.glsl"
#include "sg_shaders/impl/glsl4/color_correction/aces.glsl"
#include "sg_shaders/impl/glsl4/postprocessing/layered/utils.glsl"
#include "sg_shaders/impl/glsl4/alpha_resolving/wboit.glsl"
#include "dir_lights_shadows_calc.glsl"

#subpass [GeometryPass]

#attribute [Culling] [true]

float ambient = 0.1;

#vertex

layout (location = 0) in vec3 positionsAttribute;
layout (location = 1) in vec3 UVAttribute;
layout (location = 2) in vec3 normalsAttribute;
layout (location = 3) in vec3 tangentsAttribute;
layout (location = 4) in vec3 bitangentsAttribute;
layout (location = 5) in vec4 vertexColor0Attribute;
layout (location = 6) in vec4 vertexColor1Attribute;

out VSOut
{
    vec2 UV;
    vec3 normal;
    vec3 worldNormal;

    vec3 vertexPos;
    vec3 fragPos;
    mat3 TBN;

    vec4 vertexColor0;
    vec4 vertexColor1;
} vsOut;

void main()
{
    vsOut.UV = UVAttribute.xy;
    vsOut.normal = normalize(normalsAttribute);
    vsOut.worldNormal = normalize(mat3(transpose(inverse(objectTransform.modelMatrix))) * normalsAttribute);

    vsOut.vertexPos = positionsAttribute;
    vsOut.fragPos = vec3(objectTransform.modelMatrix * vec4(positionsAttribute, 1.0));

    vec3 T = normalize(vec3(objectTransform.modelMatrix * vec4(tangentsAttribute, 0.0)));
    vec3 B = normalize(vec3(objectTransform.modelMatrix * vec4(bitangentsAttribute, 0.0)));
    vec3 N = normalize(vec3(objectTransform.modelMatrix * vec4(vsOut.normal, 0.0)));
    vsOut.TBN = mat3(T, B, N);

    vsOut.vertexColor0 = vertexColor0Attribute;
    vsOut.vertexColor1 = vertexColor1Attribute;

    gl_Position = camera.projectionSpaceMatrix * vec4(vsOut.fragPos, 1.0);
}

#end

// ============================================================================
// ============================================================================
// ============================================================================

#fragment

// REQUIRED COLORS!!!
layout(location = 0) out vec4 layerVolume;
layout(location = 1) out vec4 layerColor;
layout(location = 2) out vec4 pickingColor;
// accum alpha output for weight blended OIT
layout(location = 3) out vec4 layerWBOITColorAccum;
layout(location = 4) out float layerWBOITReveal;


uniform sampler2D mat_diffuseSamplers[3];
uniform int mat_diffuseSamplers_CURRENT_COUNT;

uniform sampler2D mat_metalnessSamplers[1];
uniform int mat_metalnessSamplers_CURRENT_COUNT;

uniform sampler2D mat_normalsSamplers[1];
uniform int mat_normalsSamplers_CURRENT_COUNT;

uniform sampler2D mat_lightmapSamplers[1];
uniform int mat_lightmapSamplers_CURRENT_COUNT;

uniform sampler2D mat_diffuseRoughnessSamplers[1];
uniform int mat_diffuseRoughnessSamplers_CURRENT_COUNT;

uniform int SGPP_CurrentLayerIndex;

// REQUIRED UNIFORM!!
uniform vec3 u_pickingColor;

// REQUIRED UNIFORM!!
uniform int u_verticesColorsAttributesCount;

// REQUIRED UNIFORM!!
uniform int u_isTransparentPass;

in VSOut
{
    vec2 UV;
    vec3 normal;
    vec3 worldNormal;

    vec3 vertexPos;
    vec3 fragPos;
    mat3 TBN;
    vec4 vertexColor0;
    vec4 vertexColor1;
} vsIn;

// N - normal
// VD - view dir
float GeometryShlickGGX(const in float NdotVD, const in float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = NdotVD * (1.0 - k) + k;

    return NdotVD / denom;
}

// площадь поверхности, где микроскопические неровности перекрывают друг друга
float GeometrySmith(const in float NdotVD, const in float NdotL, float roughness)
{
    // ggx from view dir
    // насколько освещён фрагмент при виде от камеры
    float ggx2 = GeometryShlickGGX(NdotVD, roughness);
    // ggx from light dir
    // насколько освещён тот же фрагмент при виде от источника света
    float ggx1 = GeometryShlickGGX(NdotL, roughness);

    // если ggx1 = 1, а ggx2 = 0, то это geometry shadowing
    // если ggx1 = 0, а ggx2 = 1, то это geometry obstruction
    return ggx1 * ggx2;
}

/*float GeometrySmith(float NdotVD, float NdotL, float roughness)
{
    float m = saturate(roughness * roughness);

    float m2 = m * m;
    float a = NdotL + clamp(sqrt((NdotL - m2 * NdotL) * NdotL + m2), 0, 1);
    float b = NdotVD + clamp(sqrt((NdotVD - m2 * NdotVD) * NdotVD + m2), 0, 1);

    return 1.0 / a * b;
}*/

// Trowbridge-Reitz GGX (Normal Distribution Function)
float GGXTR(const in vec3 normal, const in vec3 medianVec, const in float roughness)
{
    float roughness4 = pow(roughness, 4.0);

    // косинус между медианным вектором и нормалью фрагмента
    float NdotMV = max(dot(normal, medianVec), 0.0);
    float NdotMV2 = NdotMV * NdotMV;

    // числитель
    float numerator = roughness4;
    // заменатель
    float denominator = (NdotMV2 * (roughness4 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return numerator / denominator;
}

vec3 SchlickFresnel(const in float cosTheta, const in vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 normalizedNormal = vsIn.normal;

    vec4 diffuseColor = vec4(materialDiffuseCol.rgb, 1.0);
    vec4 aoRoughnessMetallic = vec4(materialAmbientFactor, materialRoughnessFactor, materialMetallicFactor, 1);
    vec3 normalMapColor = vec3(0);
    vec3 finalNormal = vec3(0);

    vec2 finalUV = vsIn.UV;
    #ifdef FLIP_TEXTURES_Y
        finalUV.y = 1.0 - vsIn.UV.y;
    #endif

    // ===============================================================================================
    // ===============================        load textures       ====================================
    // ===============================================================================================

    {
        if(mat_diffuseSamplers_CURRENT_COUNT > 0)
        {
            float mixCoeff = 1.0 / mat_diffuseSamplers_CURRENT_COUNT;

            diffuseColor.rgba = vec4(0.0, 0.0, 0.0, 0.0);

            for (int i = 0; i < mat_diffuseSamplers_CURRENT_COUNT; ++i)
            {
                diffuseColor += texture(mat_diffuseSamplers[i], finalUV) * mixCoeff;
            }
        }
    }

    {
        {
            if(mat_lightmapSamplers_CURRENT_COUNT > 0)
            {
                float mixCoeff = 1.0 / mat_lightmapSamplers_CURRENT_COUNT;

                aoRoughnessMetallic.r = 0.0;

                for (int i = 0; i < mat_lightmapSamplers_CURRENT_COUNT; ++i)
                {
                    aoRoughnessMetallic.r += texture(mat_lightmapSamplers[i], finalUV).r * mixCoeff;
                }
            }
        }

        {
            if(mat_diffuseRoughnessSamplers_CURRENT_COUNT > 0)
            {
                float mixCoeff = 1.0 / mat_diffuseRoughnessSamplers_CURRENT_COUNT;

                aoRoughnessMetallic.g = 0.0;

                for (int i = 0; i < mat_diffuseRoughnessSamplers_CURRENT_COUNT; ++i)
                {
                    aoRoughnessMetallic.g += texture(mat_diffuseRoughnessSamplers[i], finalUV).g * mixCoeff;
                }

                aoRoughnessMetallic.g *= 1.0;
            }
        }

        {
            if(mat_metalnessSamplers_CURRENT_COUNT > 0)
            {
                float mixCoeff = 1.0 / mat_metalnessSamplers_CURRENT_COUNT;

                aoRoughnessMetallic.b = 0.0;

                for (int i = 0; i < mat_metalnessSamplers_CURRENT_COUNT; ++i)
                {
                    aoRoughnessMetallic.b += texture(mat_metalnessSamplers[i], finalUV).b * mixCoeff;
                }

                aoRoughnessMetallic.b *= 1.0;
            }
        }
    }

    if(mat_normalsSamplers_CURRENT_COUNT > 0)
    {
        float mixCoeff = 1.0 / mat_normalsSamplers_CURRENT_COUNT;

        for (int i = 0; i < mat_normalsSamplers_CURRENT_COUNT; ++i)
        {
            normalMapColor += texture(mat_normalsSamplers[i], finalUV).rgb * mixCoeff;
        }

        finalNormal = normalize(vsIn.TBN * (normalMapColor * 2.0 - 1.0));
    }
    else
    {
        // finalNormal = vec3(normalizedNormal.x, normalizedNormal.z, -normalizedNormal.y);
        // finalNormal = normalizedNormal;
        finalNormal = vsIn.worldNormal;
        // finalNormal = normalize(vsIn.TBN * normalizedNormal);
        // finalNormal = normalize(vsIn.TBN * (normalizedNormal * 2.0 - 1.0));
    }

    // if(diffuseColor.a < 0.1) discard;

    /*gFragPos = vec4(vsIn.fragPos, 1.0);
    gNormal = vec4(normalMapColor.rg, 0.0, 1.0);
    gAlbedoSpec = vec4(diffuseColor.rgb, 1.0);*/

    //unknownAttachment0 = vec3(1.0);
    //unknownAttachment1 = vec3(1.0);

    // ===============================================================================================
    // ===============================================================================================
    // ===============================================================================================

    // TODO: make customizable with defines
    // PBR pipeline (using Cook-Torrance BRDF) --------------------

    vec3 viewDir = normalize(camera.position - vsIn.fragPos);

    // colorFromRoughness.r = AO MAP
    // colorFromRoughness.g = ROUGHNESS
    // colorFromRoughness.b = METALNESS

    vec3 albedo         = diffuseColor.rgb;
    float ao            = aoRoughnessMetallic.r;
    float roughness     = aoRoughnessMetallic.g;
    float metalness     = aoRoughnessMetallic.b;

    // для формулы Шлика-Френеля
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metalness);

    vec3 dirLightsShadowCoeff = vec3(0.0);

    vec3 ambient = vec3(0.0);
    vec3 lo = vec3(0.0);
    for (int i = 0; i < directionalLightsCount; i++)
    {
        DirectionalLight dirLight = directionalLights[i];

        vec3 lightDir = normalize(dirLight.position - vsIn.fragPos);// TRUE
        vec3 halfWayDir = normalize(lightDir + viewDir);// TRUE

        float distance = length(dirLight.position - vsIn.fragPos);// TRUE
        float attenuation = (1.0 / (distance * distance)) * dirLight.intensity;// TRUE
        vec3 radiance = dirLight.color.rgb * attenuation;// TRUE

        // energy brightness coeff (коэфф. энергетической яркости)
        float NdotL = max(dot(finalNormal, lightDir), 0.0);
        float NdotVD = max(dot(finalNormal, viewDir), 0.0);

        vec3 finalRadiance = NdotL * radiance + radiance * 0.04;

        // ===================        shadows calc        =====================

        /*dirLightsShadowCoeff += calcDirLightShadow(
            directionalLights[i],
            vsIn.fragPos,
            finalNormal,
            sgmat_shadowMap2DSamplers[i]
        ) * finalRadiance;*/

        // ====================================================================

        // cooktorrance func: DFG /

        // NDF (normal distribution func)
        float D = GGXTR(
            finalNormal,
            halfWayDir,
            roughness
        );// TRUE

        float cosTheta = max(dot(halfWayDir, viewDir), 0.0);

        // это по сути зеркальная часть (kS)
        vec3 F = SchlickFresnel(cosTheta, F0);// kS
        // geometry function
        float G = GeometrySmith(NdotVD, NdotL, roughness);// TRUE

        vec3 diffuse = vec3(1.0) - F;
        diffuse *= (1.0 - metalness);// check diffuse color higher

        vec3 ctNumerator = D * F * G;
        float ctDenominator = 4.0 * NdotVD * NdotL;
        vec3 specular = (ctNumerator / max(ctDenominator, 0.001)) * materialSpecularCol.rgb;

        lo += (diffuse * albedo.rgb / PI + specular) * radiance * NdotL;

        ambient += 0.025 * radiance;
        // lo = vec3(1.0);
    }

    // calculating sun
    {
        // ambient += atmosphere.sunAmbient;

        vec3 lightDir = normalize(atmosphere.sunPosition);// TRUE
        vec3 halfWayDir = normalize(lightDir + viewDir);// TRUE

        // energy brightness coeff (коэфф. энергетической яркости)
        float NdotL = saturate(dot(finalNormal, lightDir));
        float NdotVD = abs(dot(finalNormal, viewDir)) + 1e-5f;

        /*if(NdotL <= 0.0)
        {
            ambient += atmosphere.sunAmbient;
        }*/

        // vec3 finalRadiance = NdotL * radiance + radiance * 0.04;

        // ===================        shadows calc        =====================

        /*dirLightsShadowCoeff += calcDirLightShadow(
            directionalLights[i],
            vsIn.fragPos,
            finalNormal,
            sgmat_shadowMap2DSamplers[i]
        ) * finalRadiance;*/

        // ====================================================================

        // cooktorrance func: DFG /

        // NDF (normal distribution func)
        float D = GGXTR(
            finalNormal,
            halfWayDir,
            roughness
        );// TRUE

        float cosTheta = saturate(dot(halfWayDir, viewDir));

        // это по сути зеркальная часть (kS)
        vec3 F = SchlickFresnel(cosTheta, F0);// kS
        // geometry function
        float G = GeometrySmith(NdotVD, NdotL, roughness);// TRUE

        vec3 diffuse = vec3(1.0) - F;
        diffuse *= (1.0 - metalness);// check diffuse color higher

        // vec3 ctNumerator = vec3(1.0, 1.0, 1.0) * G;
        vec3 ctNumerator = D * F * G;
        float ctDenominator = 1.0 * NdotVD * NdotL;
        vec3 specular = (ctNumerator / max(ctDenominator, 0.001)) * materialSpecularCol.rgb;
        // vec3 specular = ctNumerator / (ctDenominator + 0.001);
        //vec3 specular = vec3(0.1);

        /*vec3 reflectDir = reflect(-lightDir, finalNormal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = vec3(spec);*/
        // vec3 specular = vec3(0.0);

        lo += (diffuse * albedo.rgb / PI + specular) * max(atmosphere.sunColor.rgb, vec3(0, 0, 0)) * NdotL * 1.0;
    }

    // ambient = ambient * albedo.rgb * ao;
    ambient = albedo.rgb * ao;
    vec3 finalCol = ambient * materialAmbientCol.rgb * materialAmbientFactor + ambient + lo;
    float exposure = 0.5;

    // finalCol *= dirLightsShadowCoeff;

    // HDR standard tonemapper
    // ВЫГЛЯДИТ ПЛОХО
    /*finalCol = finalCol / (finalCol + vec3(1.0));
    finalCol = pow(finalCol, vec3(1.0 / exposure));*/
    // finalCol = ACESTonemap(finalCol, exposure);
    // finalCol = lottes(finalCol);
    // finalCol = reinhard2(finalCol);
    // finalCol = reinhard(finalCol);
    // finalCol = neutral(finalCol);
    // finalCol = filmic(finalCol);

    finalCol = finalCol;

    if(u_verticesColorsAttributesCount > 0)
    {
        finalCol.rgb = finalCol * vsIn.vertexColor0.rgb;
    }

    // todo: make
    // if(u_WBOITEnabled == 1)
    {
        // layerColor = vec4(finalCol, diffuseColor.a);
        calculateWBOITComponents(finalCol.rgb, diffuseColor.a, gl_FragCoord.z, layerWBOITColorAccum, layerColor, layerWBOITReveal, u_isTransparentPass);
        // layerWBOITAccumAlpha.a = 1.0;
        // layerWBOITReveal = 1.0;
        // layerWBOITReveal = 0.0;
        // layerWBOITReveal.a = 1.0;
    }

    // layerColor.a = diffuseColor.a;

    layerVolume = calculatePPLayerVolume(SGPP_CurrentLayerIndex);

    pickingColor = vec4(u_pickingColor, 1.0);

    /*fragColor0 = vec4(normalMapColor, 1.0);
    fragColor1 = vec4(normalMapColor, 1.0);*/

    // layerColor.rgb = vec3(1.0);

    // DEBUG ==================================
    // base color
    // layerColor.rgb = albedo.rgb; // PASSED
    // layerColor.rgb = vec3(albedo.r, albedo.g, albedo.b); // PASSED
    // layerColor.rgb = vec3(metalness); // PASSED
    // layerColor.rgb = vec3(roughness); // PASSED
    // layerColor.rgb = finalNormal;
    // layerColor.rgb = normalizedNormal;
    // layerColor.rgb = normalMapColor; // PASSED
    // layerColor.rgb = vec3(ao); // PASSED
    // layerColor.rgb = vec3(vsIn.UV, 1.0);
}

#end