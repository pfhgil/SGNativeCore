#sg_pragma once

#sg_include "sg_shaders/glsl4/postprocessing/layered/base.glsl"
#sg_include "sg_shaders/glsl4/disks.glsl"
#sg_include "sg_shaders/glsl4/math.glsl"
#sg_include "sg_shaders/glsl4/random.glsl"
#sg_include "sg_shaders/glsl4/postprocessing/layered/utils.glsl"

SGSubPass(SGPPLayerFXPass)
{
    SGSubShader(Fragment)
    {
        uniform sampler2D currentLayer;

        in vec2 vs_UVAttribute;

        const int samplesCount = 6;

        uniform sampler2D SGPP_LayersVolumes;
        uniform sampler2D SGPP_LayersColors;
        uniform int SGPP_CurrentLayerIndex;

        void main()
        {
            vec2 finalUV = vs_UVAttribute.xy;

            #ifdef FLIP_TEXTURES_Y
            finalUV.y = 1.0 - vs_UVAttribute.y;
            #endif

            float rand = random(finalUV);
            float rotAngle = rand * PI;
            vec2 rotTrig = vec2(cos(rotAngle), sin(rotAngle));

            vec4 finalCol = vec4(0.0);

            for(int i = 0; i < samplesCount; i++)
            {
                finalCol += texture(currentLayer, finalUV + rotate(poissonDisk[i], rotTrig) / 70.0).rgba;
            }

            finalCol.rgb /= float(samplesCount);

            // just do nothing
            // if(texture(SGPP_LayersVolumes, finalUV).rgb == calculatePPLayerVolume(SGPP_CurrentLayerIndex).rgb)
            {
                gl_FragColor = texture(SGPP_LayersColors, finalUV);
            }
            // gl_FragColor = finalCol;
        }
    }
}