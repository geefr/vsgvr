#include <vsg/io/VSG.h>
static auto model_world = []() {std::istringstream str(
R"(#vsga 0.1.2
Root id=1 vsg::Group
{
  NumUserObjects 0
  NumChildren 6
  Child id=2 vsg::Group
  {
    NumUserObjects 0
    NumChildren 1
    Child id=3 vsg::CullNode
    {
      NumUserObjects 0
      Bound -2 2 1 2.00499
      Child id=4 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=5 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 3
          Array id=6 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data -1.9 4 1.1 -2.1 4 0.9 -2.1 4 1.1 -1.9 4 0.9
             -2.1 4 0.9 -1.9 -0 0.9 -2.1 -0 0.9 -1.9 4 0.9
             -1.9 4 0.9 -1.9 -0 1.1 -1.9 -0 0.9 -1.9 4 1.1
             -2.1 -0 1.1 -1.9 -0 0.9 -1.9 -0 1.1 -2.1 -0 0.9
             -2.1 4 1.1 -2.1 -0 0.9 -2.1 -0 1.1 -2.1 4 0.9
             -1.9 4 1.1 -2.1 -0 1.1 -1.9 -0 1.1 -2.1 4 1.1
          }
          Array id=7 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0 1 0 0 1 0 0 1 0 0 1 0
             0 -0 -1 0 -0 -1 0 -0 -1 0 -0 -1
             1 -0 0 1 -0 0 1 -0 0 1 -0 0
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 -0 0 -1 -0 0 -1 -0 0 -1 -0 0
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
          }
          Array id=8 vsg::vec2Array
          {
            NumUserObjects 0
            Layout 0 8 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0.875 0.5 0.625 0.75 0.625 0.5 0.875 0.75 0.625 0.75 0.375 1
             0.375 0.75 0.625 1 0.625 0 0.375 0.25 0.375 0 0.625 0.25
             0.375 0.5 0.125 0.75 0.125 0.5 0.375 0.75 0.625 0.5 0.375 0.75
             0.375 0.5 0.625 0.75 0.625 0.25 0.375 0.5 0.375 0.25 0.625 0.5
          }
          Indices id=9 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0 -1
            Size 36
            Storage id=0
            Data 0 1 2 0 3 1 4 5 6 4 7 5
             8 9 10 8 11 9 12 13 14 12 15 13
             16 17 18 16 19 17 20 21 22 20 23 21
          }
          indexCount 36
          instanceCount 1
          firstIndex 0
          vertexOffset 0
          firstInstance 0
        }
        NumStateCommands 1
        StateCommand id=10 vsg::BindGraphicsPipeline
        {
          NumUserObjects 0
          Slot 0
          GraphicsPipeline id=11 vsg::GraphicsPipeline
          {
            NumUserObjects 0
            PipelineLayout id=12 vsg::PipelineLayout
            {
              NumUserObjects 0
              Flags 0
              NumDescriptorSetLayouts 1
              DescriptorSetLayout id=13 vsg::DescriptorSetLayout
              {
                NumUserObjects 0
                NumDescriptorSetLayoutBindings 0
              }
              NumPushConstantRanges 1
              stageFlags 1
              offset 0
              size 128
            }
            NumShaderStages 2
            ShaderStage id=14 vsg::ShaderStage
            {
              NumUserObjects 0
              Stage 1
              EntryPoint "main"
              ShaderModule id=15 vsg::ShaderModule
              {
                NumUserObjects 0
                Source "#version 450
#pragma import_defines ( VSG_NORMAL, VSG_TANGENT, VSG_COLOR, VSG_TEXCOORD0, VSG_LIGHTING, VSG_NORMAL_MAP, VSG_BILLBOARD, VSG_TRANSLATE )
#define VSG_NORMAL
#define VSG_TEXCOORD0
#extension GL_ARB_separate_shader_objects : enable
layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
    //mat3 normal;
} pc;
layout(location = 0) in vec3 osg_Vertex;
#ifdef VSG_NORMAL
layout(location = 1) in vec3 osg_Normal;
layout(location = 1) out vec3 normalDir;
#endif
#ifdef VSG_TANGENT
layout(location = 2) in vec4 osg_Tangent;
#endif
#ifdef VSG_COLOR
layout(location = 3) in vec4 osg_Color;
layout(location = 3) out vec4 vertColor;
#endif
#ifdef VSG_TEXCOORD0
layout(location = 4) in vec2 osg_MultiTexCoord0;
layout(location = 4) out vec2 texCoord0;
#endif
#ifdef VSG_LIGHTING
layout(location = 5) out vec3 viewDir;
layout(location = 6) out vec3 lightDir;
#endif
#ifdef VSG_TRANSLATE
layout(location = 7) in vec3 translate;
#endif


out gl_PerVertex{ vec4 gl_Position; };

void main()
{
    mat4 modelView = pc.modelView;

#ifdef VSG_TRANSLATE
    mat4 translate_mat = mat4(1.0, 0.0, 0.0, 0.0,
                              0.0, 1.0, 0.0, 0.0,
                              0.0, 0.0, 1.0, 0.0,
                              translate.x,  translate.y,  translate.z, 1.0);

    modelView = modelView * translate_mat;
#endif

#ifdef VSG_BILLBOARD
    vec3 lookDir = vec3(-modelView[0][2], -modelView[1][2], -modelView[2][2]);

    // rotate around local z axis
    float l = length(lookDir.xy);
    if (l>0.0)
    {
        float inv = 1.0/l;
        float c = lookDir.y * inv;
        float s = lookDir.x * inv;

        mat4 rotation_z = mat4(c,   -s,  0.0, 0.0,
                               s,   c,   0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0);

        modelView = modelView * rotation_z;
    }
#endif

    gl_Position = (pc.projection * modelView) * vec4(osg_Vertex, 1.0);

#ifdef VSG_TEXCOORD0
    texCoord0 = osg_MultiTexCoord0.st;
#endif
#ifdef VSG_NORMAL
    vec3 n = (modelView * vec4(osg_Normal, 0.0)).xyz;
    normalDir = n;
#endif
#ifdef VSG_LIGHTING
    vec4 lpos = /*osg_LightSource.position*/ vec4(0.0, 0.25, 1.0, 0.0);
#ifdef VSG_NORMAL_MAP
    vec3 t = (modelView * vec4(osg_Tangent.xyz, 0.0)).xyz;
    vec3 b = cross(n, t);
    vec3 dir = -vec3(modelView * vec4(osg_Vertex, 1.0));
    viewDir.x = dot(dir, t);
    viewDir.y = dot(dir, b);
    viewDir.z = dot(dir, n);
    if (lpos.w == 0.0)
        dir = lpos.xyz;
    else
        dir += lpos.xyz;
    lightDir.x = dot(dir, t);
    lightDir.y = dot(dir, b);
    lightDir.z = dot(dir, n);
#else
    viewDir = -vec3(modelView * vec4(osg_Vertex, 1.0));
    if (lpos.w == 0.0)
        lightDir = lpos.xyz;
    else
        lightDir = lpos.xyz + viewDir;
#endif
#endif
#ifdef VSG_COLOR
    vertColor = osg_Color;
#endif
}

"
                SPIRVSize 439
                SPIRV 119734787 66560 524298 60 0 131089 1 393227 1 1280527431 1685353262 808793134
                 0 196622 0 1 786447 0 4 1852399981 0 13 21 29
                 41 43 48 58 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449
                 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 327685 10 1701080941 1701402220
                 119 393221 11 1752397136 1936617283 1953390964 115 393222 11 0 1785688688 1769235301
                 28271 393222 11 1 1701080941 1701402220 119 196613 13 25456 393221 19
                 1348430951 1700164197 2019914866 0 393222 19 0 1348430951 1953067887 7237481 196613 21
                 0 327685 29 1600615279 1953654102 30821 327685 41 1131963764 1685221231 48 458757
                 43 1600615279 1953264973 2019906665 1919905603 12388 196613 46 110 327685 48 1600615279
                 1836216142 27745 327685 58 1836216174 1766091873 114 262216 11 0 5 327752
                 11 0 35 0 327752 11 0 7 16 262216 11 1
                 5 327752 11 1 35 64 327752 11 1 7 16 196679
                 11 2 327752 19 0 11 0 196679 19 2 262215 29
                 30 0 262215 41 30 4 262215 43 30 4 262215 48
                 30 1 262215 58 30 1 131091 2 196641 3 2 196630
                 6 32 262167 7 6 4 262168 8 7 4 262176 9
                 7 8 262174 11 8 8 262176 12 9 11 262203 12
                 13 9 262165 14 32 1 262187 14 15 1 262176 16
                 9 8 196638 19 7 262176 20 3 19 262203 20 21
                 3 262187 14 22 0 262167 27 6 3 262176 28 1
                 27 262203 28 29 1 262187 6 31 1065353216 262176 37 3
                 7 262167 39 6 2 262176 40 3 39 262203 40 41
                 3 262176 42 1 39 262203 42 43 1 262176 45 7
                 27 262203 28 48 1 262187 6 50 0 262176 57 3
                 27 262203 57 58 3 327734 2 4 0 3 131320 5
                 262203 9 10 7 262203 45 46 7 327745 16 17 13
                 15 262205 8 18 17 196670 10 18 327745 16 23 13
                 22 262205 8 24 23 262205 8 25 10 327826 8 26
                 24 25 262205 27 30 29 327761 6 32 30 0 327761
                 6 33 30 1 327761 6 34 30 2 458832 7 35
                 32 33 34 31 327825 7 36 26 35 327745 37 38
                 21 22 196670 38 36 262205 39 44 43 196670 41 44
                 262205 8 47 10 262205 27 49 48 327761 6 51 49
                 0 327761 6 52 49 1 327761 6 53 49 2 458832
                 7 54 51 52 53 50 327825 7 55 47 54 524367
                 27 56 55 55 0 1 2 196670 46 56 262205 27
                 59 46 196670 58 59 65789 65592
              }
              NumSpecializationConstants 0
            }
            ShaderStage id=16 vsg::ShaderStage
            {
              NumUserObjects 0
              Stage 16
              EntryPoint "main"
              ShaderModule id=17 vsg::ShaderModule
              {
                NumUserObjects 0
                Source "#version 450
#pragma import_defines ( VSG_NORMAL, VSG_COLOR, VSG_TEXCOORD0, VSG_LIGHTING, VSG_MATERIAL, VSG_DIFFUSE_MAP, VSG_OPACITY_MAP, VSG_AMBIENT_MAP, VSG_NORMAL_MAP, VSG_SPECULAR_MAP )
#define VSG_NORMAL
#define VSG_TEXCOORD0
#extension GL_ARB_separate_shader_objects : enable
#ifdef VSG_DIFFUSE_MAP
layout(binding = 0) uniform sampler2D diffuseMap;
#endif
#ifdef VSG_OPACITY_MAP
layout(binding = 1) uniform sampler2D opacityMap;
#endif
#ifdef VSG_AMBIENT_MAP
layout(binding = 4) uniform sampler2D ambientMap;
#endif
#ifdef VSG_NORMAL_MAP
layout(binding = 5) uniform sampler2D normalMap;
#endif
#ifdef VSG_SPECULAR_MAP
layout(binding = 6) uniform sampler2D specularMap;
#endif

#ifdef VSG_MATERIAL
layout(binding = 10) uniform MaterialData
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    float shininess;
} material;
#endif

#ifdef VSG_NORMAL
layout(location = 1) in vec3 normalDir;
#endif
#ifdef VSG_COLOR
layout(location = 3) in vec4 vertColor;
#endif
#ifdef VSG_TEXCOORD0
layout(location = 4) in vec2 texCoord0;
#endif
#ifdef VSG_LIGHTING
layout(location = 5) in vec3 viewDir;
layout(location = 6) in vec3 lightDir;
#endif
layout(location = 0) out vec4 outColor;

void main()
{
#ifdef VSG_DIFFUSE_MAP
    vec4 base = texture(diffuseMap, texCoord0.st);
#else
    vec4 base = vec4(1.0,1.0,1.0,1.0);
#endif
#ifdef VSG_COLOR
    base = base * vertColor;
#endif
#ifdef VSG_MATERIAL
    vec3 ambientColor = material.ambientColor.rgb;
    vec3 diffuseColor = material.diffuseColor.rgb;
    vec3 specularColor = material.specularColor.rgb;
    float shininess = material.shininess;
#else
    vec3 ambientColor = vec3(0.1,0.1,0.1);
    vec3 diffuseColor = vec3(1.0,1.0,1.0);
    vec3 specularColor = vec3(0.3,0.3,0.3);
    float shininess = 16.0;
#endif
#ifdef VSG_AMBIENT_MAP
    ambientColor *= texture(ambientMap, texCoord0.st).r;
#endif
#ifdef VSG_SPECULAR_MAP
    specularColor = texture(specularMap, texCoord0.st).rrr;
#endif
#ifdef VSG_LIGHTING
#ifdef VSG_NORMAL_MAP
    vec3 nDir = texture(normalMap, texCoord0.st).xyz*2.0 - 1.0;
    nDir.g = -nDir.g;
#else
    vec3 nDir = normalDir;
#endif
    vec3 nd = normalize(nDir);
    vec3 ld = normalize(lightDir);
    vec3 vd = normalize(viewDir);
    vec4 color = vec4(0.01, 0.01, 0.01, 1.0);
    color.rgb += ambientColor;
    float diff = max(dot(ld, nd), 0.0);
    color.rgb += diffuseColor * diff;
    color *= base;
    if (diff > 0.0)
    {
        vec3 halfDir = normalize(ld + vd);
        color.rgb += base.a * specularColor *
            pow(max(dot(halfDir, nd), 0.0), shininess);
    }
#else
    vec4 color = base;
    color.rgb *= diffuseColor;
#endif
    outColor = color;
#ifdef VSG_OPACITY_MAP
    outColor.a *= texture(opacityMap, texCoord0.st).r;
#endif
    if (outColor.a==0.0) discard;
}

"
                SPIRVSize 341
                SPIRV 119734787 66560 524298 52 0 131089 1 393227 1 1280527431 1685353262 808793134
                 0 196622 0 1 524303 4 4 1852399981 0 34 48 51
                 196624 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331
                 1868526181 1667590754 29556 262149 4 1852399981 0 262149 9 1702060386 0 393221
                 14 1768058209 1131703909 1919904879 0 393221 17 1717987684 1130722165 1919904879 0 393221
                 19 1667592307 1918987381 1869377347 114 327685 23 1852401779 1936027241 115 262149 25
                 1869377379 114 327685 34 1131705711 1919904879 0 327685 48 1836216174 1766091873 114
                 327685 51 1131963764 1685221231 48 262215 34 30 0 262215 48 30
                 1 262215 51 30 4 131091 2 196641 3 2 196630 6
                 32 262167 7 6 4 262176 8 7 7 262187 6 10
                 1065353216 458796 7 11 10 10 10 10 262167 12 6 3
                 262176 13 7 12 262187 6 15 1036831949 393260 12 16 15
                 15 15 393260 12 18 10 10 10 262187 6 20 1050253722
                 393260 12 21 20 20 20 262176 22 7 6 262187 6
                 24 1098907648 262176 33 3 7 262203 33 34 3 262165 36
                 32 0 262187 36 37 3 262176 38 3 6 262187 6
                 41 0 131092 42 262176 47 1 12 262203 47 48 1
                 262167 49 6 2 262176 50 1 49 262203 50 51 1
                 327734 2 4 0 3 131320 5 262203 8 9 7 262203
                 13 14 7 262203 13 17 7 262203 13 19 7 262203
                 22 23 7 262203 8 25 7 196670 9 11 196670 14
                 16 196670 17 18 196670 19 21 196670 23 24 262205 7
                 26 9 196670 25 26 262205 12 27 17 262205 7 28
                 25 524367 12 29 28 28 0 1 2 327813 12 30
                 29 27 262205 7 31 25 589903 7 32 31 30 4
                 5 6 3 196670 25 32 262205 7 35 25 196670 34
                 35 327745 38 39 34 37 262205 6 40 39 327860 42
                 43 40 41 196855 45 0 262394 43 44 45 131320 44
                 65788 131320 45 65789 65592
              }
              NumSpecializationConstants 0
            }
            NumPipelineStates 6
            PipelineState id=18 vsg::VertexInputState
            {
              NumUserObjects 0
              NumBindings 3
              binding 0
              stride 12
              inputRate 0
              binding 1
              stride 12
              inputRate 0
              binding 2
              stride 8
              inputRate 0
              NumAttributes 3
              location 0
              binding 0
              format 106
              offset 0
              location 1
              binding 1
              format 106
              offset 0
              location 4
              binding 2
              format 103
              offset 0
            }
            PipelineState id=19 vsg::InputAssemblyState
            {
              NumUserObjects 0
              topology 3
              primitiveRestartEnable 0
            }
            PipelineState id=20 vsg::RasterizationState
            {
              NumUserObjects 0
              depthClampEnable 0
              rasterizerDiscardEnable 0
              polygonMode 0
              cullMode 2
              frontFace 0
              depthBiasEnable 0
              depthBiasConstantFactor 1
              depthBiasClamp 0
              depthBiasSlopeFactor 1
              lineWidth 1
            }
            PipelineState id=21 vsg::MultisampleState
            {
              NumUserObjects 0
              rasterizationSamples 1
              sampleShadingEnable 0
              minSampleShading 0
              NumSampleMask 0
              alphaToCoverageEnable 0
              alphaToOneEnable 0
            }
            PipelineState id=22 vsg::ColorBlendState
            {
              NumUserObjects 0
              logicOp 3
              logicOpEnable 0
              NumColorBlendAttachments 1
              blendEnable 0
              srcColorBlendFactor 0
              dstColorBlendFactor 0
              colorBlendOp 0
              srcAlphaBlendFactor 0
              dstAlphaBlendFactor 0
              alphaBlendOp 0
              colorWriteMask 15
              blendConstants 0 0 0 0
            }
            PipelineState id=23 vsg::DepthStencilState
            {
              NumUserObjects 0
              depthTestEnable 1
              depthWriteEnable 1
              depthCompareOp 1
              depthBoundsTestEnable 0
              stencilTestEnable 0
              front.failOp 0
              front.passOp 0
              front.depthFailOp 0
              front.compareOp 0
              front.compareMask 0
              front.writeMask 0
              front.reference 0
              back.failOp 0
              back.passOp 0
              back.depthFailOp 0
              back.compareOp 0
              back.compareMask 0
              back.writeMask 0
              back.reference 0
              minDepthBounds 0
              maxDepthBounds 1
            }
            subpass 0
          }
        }
      }
    }
  }
  Child id=24 vsg::Group
  {
    NumUserObjects 0
    NumChildren 1
    Child id=25 vsg::CullNode
    {
      NumUserObjects 0
      Bound 2 2 -1 2.00499
      Child id=26 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=27 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 3
          Array id=28 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 2.1 4 -0.9 1.9 4 -1.1 1.9 4 -0.9 2.1 4 -1.1
             1.9 4 -1.1 2.1 -0 -1.1 1.9 -0 -1.1 2.1 4 -1.1
             2.1 4 -1.1 2.1 -0 -0.9 2.1 -0 -1.1 2.1 4 -0.9
             1.9 -0 -0.9 2.1 -0 -1.1 2.1 -0 -0.9 1.9 -0 -1.1
             1.9 4 -0.9 1.9 -0 -1.1 1.9 -0 -0.9 1.9 4 -1.1
             2.1 4 -0.9 1.9 -0 -0.9 2.1 -0 -0.9 1.9 4 -0.9
          }
          Array id=29 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0 1 0 0 1 0 0 1 0 0 1 0
             0 -0 -1 0 -0 -1 0 -0 -1 0 -0 -1
             1 -0 0 1 -0 0 1 -0 0 1 -0 0
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 -0 0 -1 -0 0 -1 -0 0 -1 -0 0
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
          }
          Array id=30 vsg::vec2Array
          {
            NumUserObjects 0
            Layout 0 8 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0.875 0.5 0.625 0.75 0.625 0.5 0.875 0.75 0.625 0.75 0.375 1
             0.375 0.75 0.625 1 0.625 0 0.375 0.25 0.375 0 0.625 0.25
             0.375 0.5 0.125 0.75 0.125 0.5 0.375 0.75 0.625 0.5 0.375 0.75
             0.375 0.5 0.625 0.75 0.625 0.25 0.375 0.5 0.375 0.25 0.625 0.5
          }
          Indices id=31 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0 -1
            Size 36
            Storage id=0
            Data 0 1 2 0 3 1 4 5 6 4 7 5
             8 9 10 8 11 9 12 13 14 12 15 13
             16 17 18 16 19 17 20 21 22 20 23 21
          }
          indexCount 36
          instanceCount 1
          firstIndex 0
          vertexOffset 0
          firstInstance 0
        }
        NumStateCommands 1
        StateCommand id=10
      }
    }
  }
  Child id=32 vsg::Group
  {
    NumUserObjects 0
    NumChildren 1
    Child id=33 vsg::CullNode
    {
      NumUserObjects 0
      Bound -2 1 -1 1.00995
      Child id=34 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=35 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 3
          Array id=36 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data -1.9 2 -0.9 -2.1 2 -1.1 -2.1 2 -0.9 -1.9 2 -1.1
             -2.1 2 -1.1 -1.9 -0 -1.1 -2.1 -0 -1.1 -1.9 2 -1.1
             -1.9 2 -1.1 -1.9 -0 -0.9 -1.9 -0 -1.1 -1.9 2 -0.9
             -2.1 -0 -0.9 -1.9 -0 -1.1 -1.9 -0 -0.9 -2.1 -0 -1.1
             -2.1 2 -0.9 -2.1 -0 -1.1 -2.1 -0 -0.9 -2.1 2 -1.1
             -1.9 2 -0.9 -2.1 -0 -0.9 -1.9 -0 -0.9 -2.1 2 -0.9
          }
          Array id=37 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0 1 0 0 1 0 0 1 0 0 1 0
             0 -0 -1 0 -0 -1 0 -0 -1 0 -0 -1
             1 -0 0 1 -0 0 1 -0 0 1 -0 0
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 -0 0 -1 -0 0 -1 -0 0 -1 -0 0
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
          }
          Array id=38 vsg::vec2Array
          {
            NumUserObjects 0
            Layout 0 8 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0.875 0.5 0.625 0.75 0.625 0.5 0.875 0.75 0.625 0.75 0.375 1
             0.375 0.75 0.625 1 0.625 0 0.375 0.25 0.375 0 0.625 0.25
             0.375 0.5 0.125 0.75 0.125 0.5 0.375 0.75 0.625 0.5 0.375 0.75
             0.375 0.5 0.625 0.75 0.625 0.25 0.375 0.5 0.375 0.25 0.625 0.5
          }
          Indices id=39 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0 -1
            Size 36
            Storage id=0
            Data 0 1 2 0 3 1 4 5 6 4 7 5
             8 9 10 8 11 9 12 13 14 12 15 13
             16 17 18 16 19 17 20 21 22 20 23 21
          }
          indexCount 36
          instanceCount 1
          firstIndex 0
          vertexOffset 0
          firstInstance 0
        }
        NumStateCommands 1
        StateCommand id=10
      }
    }
  }
  Child id=40 vsg::Group
  {
    NumUserObjects 0
    NumChildren 1
    Child id=41 vsg::CullNode
    {
      NumUserObjects 0
      Bound 2 1.5 1 1.50665
      Child id=42 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=43 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 3
          Array id=44 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 2.1 3 1.1 1.9 3 0.9 1.9 3 1.1 2.1 3 0.9
             1.9 3 0.9 2.1 -0 0.9 1.9 -0 0.9 2.1 3 0.9
             2.1 3 0.9 2.1 -0 1.1 2.1 -0 0.9 2.1 3 1.1
             1.9 -0 1.1 2.1 -0 0.9 2.1 -0 1.1 1.9 -0 0.9
             1.9 3 1.1 1.9 -0 0.9 1.9 -0 1.1 1.9 3 0.9
             2.1 3 1.1 1.9 -0 1.1 2.1 -0 1.1 1.9 3 1.1
          }
          Array id=45 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0 1 0 0 1 0 0 1 0 0 1 0
             0 -0 -1 0 -0 -1 0 -0 -1 0 -0 -1
             1 -0 0 1 -0 0 1 -0 0 1 -0 0
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 -0 0 -1 -0 0 -1 -0 0 -1 -0 0
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
          }
          Array id=46 vsg::vec2Array
          {
            NumUserObjects 0
            Layout 0 8 0 1 1 1 0 -1
            Size 24
            Storage id=0
            Data 0.875 0.5 0.625 0.75 0.625 0.5 0.875 0.75 0.625 0.75 0.375 1
             0.375 0.75 0.625 1 0.625 0 0.375 0.25 0.375 0 0.625 0.25
             0.375 0.5 0.125 0.75 0.125 0.5 0.375 0.75 0.625 0.5 0.375 0.75
             0.375 0.5 0.625 0.75 0.625 0.25 0.375 0.5 0.375 0.25 0.625 0.5
          }
          Indices id=47 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0 -1
            Size 36
            Storage id=0
            Data 0 1 2 0 3 1 4 5 6 4 7 5
             8 9 10 8 11 9 12 13 14 12 15 13
             16 17 18 16 19 17 20 21 22 20 23 21
          }
          indexCount 36
          instanceCount 1
          firstIndex 0
          vertexOffset 0
          firstInstance 0
        }
        NumStateCommands 1
        StateCommand id=10
      }
    }
  }
  Child id=48 vsg::Group
  {
    NumUserObjects 0
    NumChildren 1
    Child id=49 vsg::CullNode
    {
      NumUserObjects 0
      Bound 0.021233 1.5823 -2 1.7718
      Child id=50 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=51 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 3
          Array id=52 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 588
            Storage id=0
            Data 1.51692 1.9034 -2 1.57327 1.90568 -2 1.55763 1.9059 -2 1.51692 1.9034 -2
             1.61746 1.90188 -2 1.60297 1.90372 -2 1.56643 1.4179 -2 1.76323 1.3907 -2
             1.57695 1.4181 -2 1.47567 1.3553 -2 1.66043 1.3563 -2 1.67712 1.36024 -2
             1.51439 1.34829 -2 1.62676 1.35041 -2 1.64368 1.35303 -2 1.51439 1.34829 -2
             1.59197 1.347 -2 1.60956 1.3484 -2 1.03177 1.90473 -2 1.06643 1.90564 -2
             1.05603 1.9059 -2 1.03177 1.90473 -2 1.07682 1.90484 -2 1.06643 1.90564 -2
             1.03177 1.90473 -2 1.08718 1.9035 -2 1.07682 1.90484 -2 1.00904 1.90129 -2
             1.1078 1.89924 -2 1.09751 1.90164 -2 0.902245 1.82052 -2 1.02573 1.83337 -2
             1.03735 1.83496 -2 0.902245 1.82052 -2 0.995696 1.8228 -2 1.00483 1.82724 -2
             1.07716 1.42278 -2 1.17917 1.41749 -2 1.08768 1.42738 -2 0.940359 1.36795 -2
             1.12516 1.36525 -2 0.92842 1.37392 -2 1.00399 1.34894 -2 1.08831 1.35092 -2
             0.99064 1.35133 -2 0.05499 1.45111 -2 0.069977 1.45088 -2 0.061633 1.4515 -2
             0.048692 1.44996 -2 0.069977 1.45088 -2 0.05499 1.45111 -2 0.042783 1.44809 -2
             0.077785 1.44903 -2 0.048692 1.44996 -2 0.027832 1.4386 -2 0.091648 1.44187 -2
             0.032308 1.44237 -2 -0.396417 1.64911 -2 -0.262766 1.65447 -2 -0.246367 1.6555 -2
             -0.396417 1.64911 -2 -0.278293 1.65145 -2 -0.262766 1.65447 -2 -0.972767 1.4139 -2
             -0.847167 1.3883 -2 -0.962016 1.41423 -2 -1.04218 1.35318 -2 -0.923766 1.3531 -2
             -0.912168 1.3563 -2 -1.01733 1.34777 -2 -0.961649 1.34704 -2 -0.948417 1.34848 -2
             1.55603 1.8339 -2 1.57245 1.83352 -2 1.37672 1.84823 -2 1.34883 1.8246 -2
             1.52702 1.83206 -2 1.34733 1.4243 -2 1.53486 1.41966 -2 1.50508 1.42481 -2
             1.37488 1.40141 -2 1.56643 1.4179 -2 0.048692 1.44996 -2 0.077785 1.44903 -2
             0.069977 1.45088 -2 0.042783 1.44809 -2 0.08502 1.44602 -2 0.037307 1.44555 -2
             0.091648 1.44187 -2 0.032308 1.44237 -2 0.027832 1.4386 -2 0.097631 1.43665 -2
             0.023923 1.43429 -2 0.102933 1.4304 -2 0.020625 1.42946 -2 0.107518 1.42317 -2
             0.017982 1.42417 -2 0.01604 1.41845 -2 0.111351 1.415 -2 0.014841 1.41235 -2
             0.114395 1.40594 -2 0.014433 1.4059 -2 0.116614 1.39604 -2 0.014761 1.39949 -2
             0.015725 1.39347 -2 0.117972 1.38535 -2 0.017295 1.38788 -2 0.01944 1.38273 -2
             0.118433 1.3739 -2 0.022129 1.37807 -2 0.025332 1.3739 -2 0.029019 1.37028 -2
             0.117798 1.36239 -2 0.033158 1.36721 -2 0.03772 1.36473 -2 0.042673 1.36287 -2
             0.047988 1.36165 -2 0.053633 1.3611 -2 0.115888 1.35076 -2 0.055083 1.35367 -2
             0.055833 1.34587 -2 0.112695 1.33914 -2 0.055882 1.3378 -2 0.108211 1.32768 -2
             0.055232 1.32961 -2 0.053882 1.3214 -2 0.102426 1.31652 -2 0.051832 1.3133 -2
             0.095333 1.3058 -2 0.049082 1.30544 -2 0.086923 1.29567 -2 0.045632 1.29793 -2
             0.041482 1.2909 -2 0.077188 1.28626 -2 0.036632 1.28447 -2 0.06612 1.27772 -2
             0.031082 1.27877 -2 0.024833 1.2739 -2 0.05371 1.27018 -2 0.024833 1.2587 -2
             0.039951 1.2638 -2 -0.381767 1.3997 -2 -0.246367 1.4099 -2 -0.263103 1.41094 -2
             -0.105167 1.3985 -2 -0.228162 1.41089 -2 1.51439 1.34829 -2 1.60956 1.3484 -2
             1.62676 1.35041 -2 1.55523 1.3459 -2 1.59197 1.347 -2 1.5739 1.34617 -2
             1.64368 1.35303 -2 1.47567 1.3553 -2 1.66043 1.3563 -2 1.67712 1.36024 -2
             1.69384 1.36487 -2 1.43932 1.36665 -2 1.71071 1.3702 -2 1.72783 1.37627 -2
             1.40563 1.38211 -2 1.7453 1.3831 -2 1.76323 1.3907 -2 1.37488 1.40141 -2
             1.56643 1.4179 -2 1.53486 1.41966 -2 1.34733 1.4243 -2 1.50508 1.42481 -2
             1.47726 1.43318 -2 1.45159 1.4446 -2 1.32327 1.45053 -2 1.42825 1.4589 -2
             1.40743 1.4759 -2 1.30297 1.47983 -2 1.38931 1.49544 -2 1.2867 1.51195 -2
             1.37408 1.51734 -2 1.36191 1.54143 -2 1.27473 1.54665 -2 1.35299 1.56753 -2
             1.26735 1.58365 -2 1.3475 1.59548 -2 1.26483 1.6227 -2 1.34563 1.6251 -2
             1.34751 1.65409 -2 1.26742 1.66206 -2 1.353 1.68168 -2 1.27497 1.69951 -2
             1.36186 1.70768 -2 1.37387 1.73186 -2 1.28718 1.73479 -2 1.3888 1.75401 -2
             1.30377 1.76759 -2 1.40643 1.7739 -2 1.42653 1.79133 -2 1.32442 1.79763 -2
             1.44886 1.80608 -2 1.47321 1.81793 -2 1.34883 1.8246 -2 1.49934 1.82666 -2
             1.52702 1.83206 -2 1.55603 1.8339 -2 1.37672 1.84823 -2 1.57245 1.83352 -2
             1.58888 1.83238 -2 1.75203 1.8595 -2 1.60533 1.83048 -2 1.62178 1.82783 -2
             1.63822 1.82444 -2 1.65463 1.8203 -2 1.67102 1.81544 -2 1.68735 1.80984 -2
             1.70363 1.80353 -2 1.71985 1.7965 -2 1.73599 1.78875 -2 1.75203 1.7803 -2
             1.73145 1.86798 -2 1.40777 1.86821 -2 1.71236 1.87544 -2 1.69455 1.88194 -2
             1.44168 1.88427 -2 1.67781 1.88753 -2 1.66194 1.89227 -2 1.47817 1.89609 -2
             1.64673 1.8962 -2 1.63197 1.89939 -2 1.61746 1.90188 -2 1.51692 1.9034 -2
             1.51692 1.9034 -2 1.58831 1.90497 -2 1.57327 1.90568 -2 1.60297 1.90372 -2
             -0.587167 1.3531 -2 -0.515967 1.8987 -2 -0.587168 1.8987 -2 -0.515967 1.3531 -2
             -0.762367 1.3531 -2 -0.691167 1.8987 -2 -0.762367 1.8987 -2 -0.691167 1.3531 -2
             -0.990367 1.3459 -2 -0.975603 1.34619 -2 -0.961649 1.34704 -2 -1.01733 1.34777 -2
             -0.948417 1.34848 -2 -0.935819 1.3505 -2 -0.923766 1.3531 -2 -1.04218 1.35318 -2
             -0.912168 1.3563 -2 -0.900936 1.36011 -2 -1.06487 1.36183 -2 -0.889982 1.36451 -2
             -0.879218 1.36953 -2 -1.08536 1.37343 -2 -0.868553 1.37516 -2 -0.857899 1.38142 -2
             -1.10361 1.38769 -2 -0.847167 1.3883 -2 -1.11957 1.4043 -2 -0.857553 1.45054 -2
             -0.847167 1.4571 -2 -0.867916 1.44446 -2 -0.878267 1.43888 -2 -0.888619 1.43382 -2
             -0.898982 1.42928 -2 -0.909368 1.4253 -2 -0.919786 1.42189 -2 -0.930249 1.41906 -2
             -0.940768 1.41683 -2 -0.951353 1.41522 -2 -0.962016 1.41423 -2 -0.972767 1.4139 -2
             -0.989383 1.41467 -2 -1.00483 1.41696 -2 -1.01909 1.42077 -2 -1.13319 1.42299 -2
             -1.03217 1.42608 -2 -1.04407 1.4329 -2 -1.05477 1.4412 -2 -1.14444 1.44344 -2
             -1.06427 1.45099 -2 -1.07256 1.46226 -2 -1.15327 1.46538 -2 -1.07964 1.47499 -2
             -1.15963 1.4885 -2 -1.08551 1.48918 -2 -1.09015 1.50482 -2 -1.16348 1.5125 -2
             -1.09357 1.5219 -2 -1.16477 1.5371 -2 -0.840767 1.5219 -2 -0.842019 1.55369 -2
             -1.16336 1.56355 -2 -0.911168 1.5691 -2 -1.09197 1.5691 -2 -0.912362 1.58228 -2
             -1.08908 1.58278 -2 -0.845712 1.58265 -2 -1.15921 1.58839 -2 -0.914723 1.59457 -2
             -1.08524 1.5954 -2 -0.918218 1.60593 -2 -1.08044 1.60694 -2 -0.851755 1.6088 -2
             -1.15247 1.6115 -2 -0.922812 1.6163 -2 -1.07469 1.61737 -2 -0.928473 1.62565 -2
             -1.068 1.62667 -2 -0.860056 1.63216 -2 -1.14326 1.63275 -2 -0.935168 1.6339 -2
             -1.06037 1.6348 -2 -0.942862 1.64103 -2 -1.0518 1.64176 -2 -0.951523 1.64697 -2
             -1.04231 1.6475 -2 -0.961118 1.65168 -2 -1.13171 1.65199 -2 -0.870524 1.65272 -2
             -1.03189 1.65202 -2 -0.971612 1.6551 -2 -1.02056 1.65527 -2 -0.982973 1.65719 -2
             -1.00832 1.65724 -2 -0.995167 1.6579 -2 -1.11797 1.6691 -2 -0.883068 1.6705 -2
             -1.10216 1.68395 -2 -0.897594 1.68552 -2 -1.08441 1.69639 -2 -0.914012 1.69779 -2
             -1.06487 1.7063 -2 -0.93223 1.7073 -2 -1.04366 1.71355 -2 -0.952156 1.71409 -2
             -1.02091 1.71799 -2 -0.973699 1.71815 -2 -0.996767 1.7195 -2 -0.245567 1.3459 -2
             -0.217542 1.34748 -2 -0.27249 1.34754 -2 -0.191167 1.35212 -2 -0.297945 1.35234 -2
             -0.166592 1.35968 -2 -0.321767 1.36013 -2 -0.143967 1.37002 -2 -0.34379 1.37073 -2
             -0.123442 1.38301 -2 -0.363845 1.38398 -2 -0.105167 1.3985 -2 -0.381767 1.3997 -2
             -0.246367 1.4099 -2 -0.263103 1.41094 -2 -0.278849 1.41399 -2 -0.39739 1.41772 -2
             -0.293517 1.41897 -2 -0.307019 1.42578 -2 -0.319266 1.43436 -2 -0.410545 1.43787 -2
             -0.330167 1.4446 -2 -0.339636 1.45643 -2 -0.421067 1.45998 -2 -0.347582 1.46976 -2
             -0.42879 1.48387 -2 -0.353917 1.48449 -2 -0.358553 1.50055 -2 -0.433545 1.50937 -2
             -0.361399 1.51785 -2 -0.362367 1.5363 -2 -0.435167 1.5363 -2 -0.361368 1.5531 -2
             -0.433484 1.56159 -2 -0.358441 1.56902 -2 -0.353692 1.58395 -2 -0.428567 1.58571 -2
             -0.347227 1.59778 -2 -0.420617 1.60847 -2 -0.33915 1.61041 -2 -0.329567 1.6217 -2
             -0.409834 1.62967 -2 -0.318585 1.63157 -2 -0.306308 1.63989 -2 -0.292843 1.64655 -2
             -0.396417 1.64911 -2 -0.278293 1.65145 -2 -0.246367 1.6555 -2 -0.380567 1.6666 -2
             -0.090021 1.65047 -2 -0.229002 1.65447 -2 -0.212579 1.65145 -2 -0.197205 1.64654 -2
             -0.106067 1.6678 -2 -0.362484 1.68195 -2 -0.076501 1.63109 -2 -0.124463 1.68292 -2
             -0.18299 1.63986 -2 -0.342367 1.69494 -2 -0.170041 1.63151 -2 -0.145034 1.69565 -2
             -0.158467 1.6216 -2 -0.320417 1.70539 -2 -0.06568 1.60982 -2 -0.167605 1.70584 -2
             -0.148377 1.61025 -2 -0.296834 1.7131 -2 -0.139879 1.59755 -2 -0.192001 1.71332 -2
             -0.057734 1.58682 -2 -0.271817 1.71787 -2 -0.13308 1.58362 -2 -0.218046 1.71793 -2
             -0.245567 1.7195 -2 -0.052838 1.56226 -2 -0.12809 1.56856 -2 -0.125016 1.55249 -2
             -0.051167 1.5363 -2 -0.123967 1.5355 -2 -0.052792 1.50869 -2 -0.12494 1.51673 -2
             -0.127812 1.49926 -2 -0.057567 1.48276 -2 -0.132517 1.48314 -2 -0.13899 1.46845 -2
             -0.065342 1.45863 -2 -0.147162 1.45525 -2 -0.075967 1.43645 -2 -0.156967 1.4436 -2
             -0.168339 1.43357 -2 -0.089292 1.41636 -2 -0.181212 1.42522 -2 -0.195517 1.41862 -2
             -0.21119 1.41382 -2 -0.228162 1.41089 -2 1.58003 1.5515 -2 1.76323 1.6219 -2
             1.58003 1.6219 -2 1.68483 1.5515 -2 1.68483 1.4395 -2 1.67541 1.43656 -2
             1.66595 1.43374 -2 1.65643 1.43107 -2 1.64685 1.42857 -2 1.63718 1.42628 -2
             1.62743 1.4242 -2 1.61758 1.42238 -2 1.60762 1.42084 -2 1.59753 1.41959 -2
             1.58731 1.41867 -2 1.57695 1.4181 -2 1.00904 1.90129 -2 1.08718 1.9035 -2
             1.03177 1.90473 -2 1.09751 1.90164 -2 0.98797 1.89564 -2 1.1078 1.89924 -2
             1.11803 1.8963 -2 1.1282 1.89284 -2 0.968655 1.88789 -2 1.13829 1.88884 -2
             1.14828 1.8843 -2 0.951206 1.87812 -2 1.15818 1.87924 -2 1.16797 1.87364 -2
             0.935733 1.8664 -2 1.17763 1.8675 -2 1.16607 1.78907 -2 1.17763 1.7787 -2
             1.1546 1.79819 -2 1.14323 1.80614 -2 1.132 1.81299 -2 1.12093 1.81878 -2
             1.11003 1.8236 -2 1.09934 1.82751 -2 1.08886 1.83056 -2 1.07863 1.83282 -2
             1.06867 1.83435 -2 0.922343 1.85284 -2 1.059 1.83523 -2 1.04963 1.8355 -2
             0.911144 1.83752 -2 1.03735 1.83496 -2 0.902245 1.82052 -2 1.01487 1.83078 -2
             1.02573 1.83337 -2 1.00483 1.82724 -2 0.987533 1.8175 -2 0.995696 1.8228 -2
             0.895755 1.80192 -2 0.980421 1.81141 -2 0.974433 1.80457 -2 0.969646 1.79703 -2
             0.891782 1.78182 -2 0.966133 1.78884 -2 0.96397 1.78005 -2 0.890433 1.7603 -2
             0.963233 1.7707 -2 0.967877 1.74656 -2 0.895076 1.72562 -2 0.980715 1.72587 -2
             1.00011 1.70765 -2 0.907914 1.69581 -2 1.02442 1.69094 -2 0.927308 1.67005 -2
             1.05201 1.67475 -2 1.08123 1.6581 -2 0.951618 1.64753 -2 1.11046 1.64002 -2
             0.979206 1.62742 -2 1.13805 1.61953 -2 1.00843 1.6089 -2 1.16236 1.59565 -2
             1.03766 1.59115 -2 1.18175 1.56741 -2 1.06525 1.57334 -2 1.08956 1.55465 -2
             1.19459 1.53382 -2 1.10895 1.53427 -2 1.12179 1.51136 -2 1.19923 1.4939 -2
             1.12643 1.4851 -2 1.19791 1.47301 -2 1.12585 1.47736 -2 1.12408 1.46945 -2
             1.19403 1.45322 -2 1.12111 1.46153 -2 1.11692 1.45376 -2 1.1115 1.44629 -2
             1.18773 1.43467 -2 1.10483 1.4393 -2 1.0969 1.43295 -2 1.17917 1.41749 -2
             1.08768 1.42738 -2 1.06532 1.41929 -2 1.07716 1.42278 -2 1.05215 1.41708 -2
             1.16849 1.40182 -2 1.03763 1.4163 -2 0.893633 1.3963 -2 1.0243 1.41689 -2
             1.01113 1.41861 -2 0.998145 1.42148 -2 0.985366 1.42546 -2 0.97282 1.43054 -2
             0.960533 1.4367 -2 0.948529 1.44393 -2 0.936833 1.45221 -2 0.92547 1.46153 -2
             0.914466 1.47186 -2 0.903845 1.48319 -2 0.893633 1.4955 -2 1.15583 1.3878 -2
             0.905088 1.38809 -2 0.916673 1.38063 -2 1.14134 1.37557 -2 0.92842 1.37392 -2
             1.12516 1.36525 -2 0.952519 1.36271 -2 0.940359 1.36795 -2 1.10743 1.35699 -2
             0.964933 1.3582 -2 0.97763 1.35441 -2 1.08831 1.35092 -2 0.99064 1.35133 -2
             1.00399 1.34894 -2 1.06792 1.34718 -2 1.01772 1.34725 -2 1.03186 1.34624 -2
             1.04643 1.3459 -2 0.611233 1.3483 -2 0.453633 1.8987 -2 0.375233 1.8987 -2
             0.617633 1.5155 -2 0.627233 1.3483 -2 0.858433 1.8987 -2 0.780833 1.8987 -2
             -1.72077 1.3531 -2 -1.64237 1.8987 -2 -1.72077 1.8987 -2 -1.64237 1.6611 -2
             -1.64237 1.5907 -2 -1.64237 1.3531 -2 -1.33677 1.6611 -2 -1.33677 1.5907 -2
             -1.33677 1.3531 -2 -1.25837 1.3531 -2 -1.25837 1.8987 -2 -1.33677 1.8987 -2
          }
          Array id=53 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 588
            Storage id=0
            Data 0 0.0002 1 0 0.0002 1 0 0.0002 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 0.0002 1 0 0.0002 1
             0 0.0002 1 0 -0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 0.0001 1 0 0.0001 1 0 0.0001 1 0 -0.0002 1
             0 -0.0002 1 0 -0.0002 1 0 -0.0002 1 0 -0.0002 1
             0 -0.0002 1 0 -0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 0.0002 1 0 0.0002 1 0 0.0002 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 0.0001 1 0 0.0001 1
             0 0.0001 1 0 -0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 0.0001 1 0 0.0001 1 0 0.0001 1 0 -0.0001 1
             0 -0.0001 1 0 -0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 -0.0001 1 0 -0.0005 1 0 -0.0005 1 0 -0.0005 1
             0 0.0002 1 0 0.0002 1 0 0.0002 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 0.0001 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 0.0001 1 0 0.0001 1
             0 -0.0001 1 0 -0.0001 1 0 -0.0001 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 -0.0001 1 0 -0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 -0.0004 1 0 -0.0004 1 0 -0.0004 1 0 -0.0004 1
             0 -0.0004 1 0 0.0001 1 0 0.0001 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 0.0001 1 0 0.0001 1 0 0.0001 1
             0 0.0001 1 0 0.0001 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0.0001 1 0 -0.0001 1 0 -0.0001 1 0 -0.0001 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
             0 -0 1 0 -0 1 0 -0 1 0 -0 1
          }
          Array id=54 vsg::vec2Array
          {
            NumUserObjects 0
            Layout 0 8 0 1 1 1 0 -1
            Size 588
            Storage id=0
            Data 0.653465 0 0.633663 0 0.643564 0 0.653465 0 0.60396 0 0.613861 0
             0.158416 0 1 0 0.148515 0 0.861386 0 0.940594 0 0.950495 0
             0.871287 0 0.920792 0 0.930693 0 0.871287 0 0.90099 0 0.910891 0
             0.107438 0 0.090909 0 0.099174 0 0.107438 0 0.082645 0 0.090909 0
             0.107438 0 0.07438 0 0.082645 0 0.115702 0 0.057851 0 0.066116 0
             0.173554 0 0.884298 0 0.892562 0 0.173554 0 0.859504 0 0.867769 0
             0.371901 0 0.669421 0 0.363636 0 0.53719 0 0.636364 0 0.528926 0
             0.578512 0 0.619835 0 0.570248 0 0.416667 0 0.383333 0 0.4 0
             0.433333 0 0.383333 0 0.416667 0 0.45 0 0.366667 0 0.433333 0
             0.5 0 0.333333 0 0.483333 0 0.073684 0 1 0 0.505263 0
             0.073684 0 0.989474 0 1 0 0.122449 0 0.744898 0 0.112245 0
             0.602041 0 0.673469 0 0.683673 0 0.612245 0 0.642857 0 0.653061 0
             0.39604 0 0.405941 0 0.693069 0 0.70297 0 0.386139 0 0.821782 0
             0.168317 0 0.178218 0 0.831683 0 0.158416 0 0.433333 0 0.366667 0
             0.383333 0 0.45 0 0.35 0 0.466667 0 0.333333 0 0.483333 0
             0.5 0 0.316667 0 0.516667 0 0.3 0 0.533333 0 0.283333 0
             0.55 0 0.566667 0 0.266667 0 0.583333 0 0.25 0 0.6 0
             0.233333 0 0.616667 0 0.633333 0 0.216667 0 0.65 0 0.666667 0
             0.2 0 0.683333 0 0.7 0 0.716667 0 0.183333 0 0.733333 0
             0.75 0 0.766667 0 0.783333 0 0.8 0 0.166667 0 0.816667 0
             0.833333 0 0.15 0 0.85 0 0.133333 0 0.866667 0 0.883333 0
             0.116667 0 0.9 0 0.1 0 0.916667 0 0.083333 0 0.933333 0
             0.95 0 0.066667 0 0.966667 0 0.05 0 0.983333 0 1 0
             0.033333 0 0 0 0.016667 0 0.189474 0 0.757895 0 0.768421 0
             0.315789 0 0.747368 0 0.871287 0 0.910891 0 0.920792 0 0.881188 0
             0.90099 0 0.891089 0 0.930693 0 0.861386 0 0.940594 0 0.950495 0
             0.960396 0 0.851485 0 0.970297 0 0.980198 0 0.841584 0 0.990099 0
             1 0 0.831683 0 0.158416 0 0.168317 0 0.821782 0 0.178218 0
             0.188119 0 0.19802 0 0.811881 0 0.207921 0 0.217822 0 0.80198 0
             0.227723 0 0.792079 0 0.237624 0 0.247525 0 0.782178 0 0.257426 0
             0.772277 0 0.267327 0 0.762376 0 0.277228 0 0.287129 0 0.752475 0
             0.29703 0 0.742574 0 0.306931 0 0.316832 0 0.732673 0 0.326733 0
             0.722772 0 0.336634 0 0.346535 0 0.712871 0 0.356436 0 0.366337 0
             0.70297 0 0.376238 0 0.386139 0 0.39604 0 0.693069 0 0.405941 0
             0.415842 0 0.524752 0 0.425743 0 0.435644 0 0.445545 0 0.455446 0
             0.465347 0 0.475248 0 0.485149 0 0.49505 0 0.504951 0 0.514852 0
             0.534653 0 0.683168 0 0.544554 0 0.554455 0 0.673267 0 0.564356 0
             0.574257 0 0.663366 0 0.584158 0 0.594059 0 0.60396 0 0.653465 0
             0.653465 0 0.623762 0 0.633663 0 0.613861 0 0.666667 0 0 0
             0.333333 0 1 0 0.666667 0 0 0 0.333333 0 1 0
             0.622449 0 0.632653 0 0.642857 0 0.612245 0 0.653061 0 0.663265 0
             0.673469 0 0.602041 0 0.683673 0 0.693878 0 0.591837 0 0.704082 0
             0.714286 0 0.581633 0 0.72449 0 0.734694 0 0.571429 0 0.744898 0
             0.561224 0 0.010204 0 0 0 0.020408 0 0.030612 0 0.040816 0
             0.05102 0 0.061224 0 0.071429 0 0.081633 0 0.091837 0 0.102041 0
             0.112245 0 0.122449 0 0.132653 0 0.142857 0 0.153061 0 0.55102 0
             0.163265 0 0.173469 0 0.183673 0 0.540816 0 0.193878 0 0.204082 0
             0.530612 0 0.214286 0 0.520408 0 0.22449 0 0.234694 0 0.510204 0
             0.244898 0 0.5 0 0.255102 0 0.265306 0 0.489796 0 1 0
             0.755102 0 0.989796 0 0.765306 0 0.27551 0 0.479592 0 0.979592 0
             0.77551 0 0.969388 0 0.785714 0 0.285714 0 0.469388 0 0.959184 0
             0.795918 0 0.94898 0 0.806122 0 0.295918 0 0.459184 0 0.938776 0
             0.816327 0 0.928571 0 0.826531 0 0.918367 0 0.836735 0 0.908163 0
             0.44898 0 0.306122 0 0.846939 0 0.897959 0 0.857143 0 0.887755 0
             0.867347 0 0.877551 0 0.438776 0 0.316327 0 0.428571 0 0.326531 0
             0.418367 0 0.336735 0 0.408163 0 0.346939 0 0.397959 0 0.357143 0
             0.387755 0 0.367347 0 0.377551 0 0.252632 0 0.263158 0 0.242105 0
             0.273684 0 0.231579 0 0.284211 0 0.221053 0 0.294737 0 0.210526 0
             0.305263 0 0.2 0 0.315789 0 0.189474 0 0.757895 0 0.768421 0
             0.778947 0 0.178947 0 0.789474 0 0.8 0 0.810526 0 0.168421 0
             0.821053 0 0.831579 0 0.157895 0 0.842105 0 0.147368 0 0.852632 0
             0.863158 0 0.136842 0 0.873684 0 0.884211 0 0.126316 0 0.894737 0
             0.115789 0 0.905263 0 0.915789 0 0.105263 0 0.926316 0 0.094737 0
             0.936842 0 0.947368 0 0.084211 0 0.957895 0 0.968421 0 0.978947 0
             0.073684 0 0.989474 0 0.505263 0 0.063158 0 0.431579 0 0.515789 0
             0.526316 0 0.536842 0 0.442105 0 0.052632 0 0.421053 0 0.452632 0
             0.547368 0 0.042105 0 0.557895 0 0.463158 0 0.568421 0 0.031579 0
             0.410526 0 0.473684 0 0.578947 0 0.021053 0 0.589474 0 0.484211 0
             0.4 0 0.010526 0 0.6 0 0.494737 0 0 0 0.389474 0
             0.610526 0 0.621053 0 0.378947 0 0.631579 0 0.368421 0 0.642105 0
             0.652632 0 0.357895 0 0.663158 0 0.673684 0 0.347368 0 0.684211 0
             0.336842 0 0.694737 0 0.705263 0 0.326316 0 0.715789 0 0.726316 0
             0.736842 0 0.747368 0 0.019802 0 0 0 0.009901 0 0.029703 0
             0.039604 0 0.049505 0 0.059406 0 0.069307 0 0.079208 0 0.089109 0
             0.09901 0 0.108911 0 0.118812 0 0.128713 0 0.138614 0 0.148515 0
             0.115702 0 0.07438 0 0.107438 0 0.066116 0 0.123967 0 0.057851 0
             0.049587 0 0.041322 0 0.132231 0 0.033058 0 0.024793 0 0.140496 0
             0.016529 0 0.008264 0 0.14876 0 0 0 0.991736 0 1 0
             0.983471 0 0.975207 0 0.966942 0 0.958678 0 0.950413 0 0.942149 0
             0.933884 0 0.92562 0 0.917355 0 0.157025 0 0.909091 0 0.900826 0
             0.165289 0 0.892562 0 0.173554 0 0.876033 0 0.884298 0 0.867769 0
             0.85124 0 0.859504 0 0.181818 0 0.842975 0 0.834711 0 0.826446 0
             0.190083 0 0.818182 0 0.809917 0 0.198347 0 0.801653 0 0.793388 0
             0.206612 0 0.785124 0 0.77686 0 0.214876 0 0.768595 0 0.22314 0
             0.760331 0 0.752066 0 0.231405 0 0.743802 0 0.239669 0 0.735537 0
             0.247934 0 0.727273 0 0.256198 0 0.719008 0 0.264463 0 0.272727 0
             0.710744 0 0.280992 0 0.289256 0 0.702479 0 0.297521 0 0.694215 0
             0.305785 0 0.31405 0 0.68595 0 0.322314 0 0.330579 0 0.338843 0
             0.677686 0 0.347107 0 0.355372 0 0.669421 0 0.363636 0 0.380165 0
             0.371901 0 0.38843 0 0.661157 0 0.396694 0 0.504132 0 0.404959 0
             0.413223 0 0.421488 0 0.429752 0 0.438017 0 0.446281 0 0.454545 0
             0.46281 0 0.471074 0 0.479339 0 0.487603 0 0.495868 0 0.652893 0
             0.512397 0 0.520661 0 0.644628 0 0.528926 0 0.636364 0 0.545455 0
             0.53719 0 0.628099 0 0.553719 0 0.561983 0 0.619835 0 0.570248 0
             0.578512 0 0.61157 0 0.586777 0 0.595041 0 0.603306 0 0.833333 0
             0.5 0 0.666667 0 0.333333 0 1 0 0 0 0.166667 0
             0.545455 0 0.363636 0 0.454545 0 0.272727 0 0.727273 0 0.636364 0
             0.181818 0 0.818182 0 0.909091 0 1 0 0 0 0.090909 0
          }
          Indices id=55 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0 -1
            Size 1479
            Storage id=0
            Data 0 1 2 3 4 5 6 7 8 9 10 11
             12 13 14 15 16 17 18 19 20 21 22 23
             24 25 26 27 28 29 30 31 32 33 34 35
             36 37 38 39 40 41 42 43 44 45 46 47
             48 49 50 51 52 53 54 55 56 57 58 59
             60 61 62 63 64 65 66 67 68 69 70 71
             72 73 74 75 76 72 77 78 79 80 81 78
             82 83 84 85 86 83 87 86 85 87 88 86
             89 88 87 90 91 88 92 91 90 92 93 91
             94 93 92 94 95 93 96 95 94 97 95 96
             97 98 95 99 98 97 99 100 98 101 100 99
             101 102 100 103 102 101 104 102 103 104 105 102
             106 105 104 107 105 106 107 108 105 109 108 107
             110 108 109 111 108 110 111 112 108 113 112 111
             114 112 113 115 112 114 116 112 115 116 117 112
             117 118 112 119 118 117 120 118 119 120 121 118
             122 121 120 122 123 121 124 123 122 125 123 124
             125 126 123 127 126 125 127 128 126 129 128 127
             129 130 128 131 130 129 132 130 131 132 133 130
             134 133 132 134 135 133 136 135 134 137 135 136
             137 138 135 139 138 137 139 140 138 141 142 143
             142 144 145 146 147 148 149 150 146 149 151 150
             146 152 153 153 152 154 153 155 156 153 156 157
             157 156 158 157 158 159 157 159 160 160 159 161
             160 161 162 160 162 163 163 162 164 163 165 166
             166 167 168 166 168 169 166 169 170 170 169 171
             170 171 172 170 172 173 173 172 174 173 174 175
             175 174 176 175 176 177 175 177 178 178 177 179
             178 179 180 180 179 181 180 181 182 182 181 183
             182 183 184 182 184 185 185 184 186 185 186 187
             187 186 188 187 188 189 187 189 190 190 189 191
             190 191 192 192 191 193 192 193 194 192 194 195
             195 194 196 195 196 197 195 197 198 198 197 199
             198 199 200 198 201 202 203 204 202 202 204 205
             204 206 205 206 207 205 207 208 205 208 209 205
             209 210 205 210 211 205 211 212 205 212 213 205
             213 214 205 214 215 205 202 205 216 202 216 217
             217 216 218 217 218 219 217 219 220 220 219 221
             220 221 222 220 222 223 223 222 224 223 224 225
             223 225 226 223 226 227 228 229 230 228 231 229
             232 233 234 232 235 233 236 237 238 236 239 237
             240 241 242 240 242 243 243 244 245 243 245 246
             243 246 247 247 248 249 247 249 250 250 249 251
             250 251 252 250 252 253 253 252 254 253 254 255
             253 255 256 256 255 257 256 257 258 259 257 260
             261 257 259 262 257 261 263 257 262 264 257 263
             265 257 264 266 257 265 267 257 266 268 257 267
             269 257 268 270 257 269 258 257 271 258 271 272
             258 272 273 258 273 274 258 274 275 275 274 276
             275 276 277 275 277 278 275 278 279 279 278 280
             279 280 281 279 281 282 282 281 283 282 283 284
             284 283 285 284 285 286 284 286 287 287 286 288
             287 288 289 288 290 289 289 290 291 289 291 292
             292 291 293 292 293 294 293 291 295 292 294 296
             295 291 297 292 296 298 295 297 299 298 296 300
             299 297 301 298 300 302 301 297 303 298 302 304
             301 303 305 304 302 306 305 303 307 304 306 308
             307 303 309 304 308 310 307 309 311 310 308 312
             311 309 313 310 312 314 313 309 315 310 314 316
             315 309 317 310 316 318 317 309 319 318 316 320
             317 319 321 318 320 322 321 319 323 318 322 324
             323 319 325 318 324 325 318 325 326 325 319 326
             326 319 327 326 327 328 328 327 329 328 329 330
             330 329 331 330 331 332 332 331 333 332 333 334
             334 333 335 334 335 336 336 335 337 336 337 338
             339 340 341 341 340 342 341 342 343 343 342 344
             343 344 345 345 344 346 345 346 347 347 346 348
             347 348 349 349 348 350 349 350 351 351 350 352
             351 353 354 351 354 355 355 354 356 355 356 357
             355 357 358 355 358 359 359 358 360 359 360 361
             359 361 362 362 361 363 362 363 364 364 363 365
             364 365 366 364 366 367 367 366 368 367 368 369
             367 369 370 370 369 371 370 371 372 372 371 373
             372 373 374 372 374 375 375 374 376 375 376 377
             377 376 378 377 378 379 377 379 380 380 379 381
             380 381 382 380 382 383 380 383 384 384 383 385
             384 386 387 386 388 387 389 388 386 390 388 389
             391 388 390 387 388 392 387 392 393 391 394 388
             393 392 395 396 394 391 393 395 397 398 394 396
             397 395 399 400 394 398 397 399 401 400 402 394
             401 399 403 404 402 400 401 403 405 406 402 404
             405 403 407 406 408 402 405 407 409 410 408 406
             409 407 411 409 411 412 410 413 408 414 413 410
             415 413 414 415 416 413 417 416 415 417 418 416
             419 418 417 420 418 419 420 421 418 422 421 420
             423 421 422 423 424 421 425 424 423 425 426 424
             427 426 425 428 426 427 428 429 426 430 429 428
             431 429 430 432 429 431 432 350 429 433 350 432
             434 435 436 434 437 435 437 162 435 438 162 437
             439 162 438 440 162 439 441 162 440 442 162 441
             443 162 442 444 162 443 445 162 444 446 162 445
             447 162 446 448 162 447 449 162 448 450 451 452
             450 453 451 454 455 450 454 456 455 454 457 456
             458 457 454 458 459 457 458 460 459 461 460 458
             461 462 460 461 463 462 464 463 461 464 465 463
             466 467 465 468 466 465 469 468 465 470 469 465
             471 470 465 472 471 465 473 472 465 474 473 465
             464 474 465 475 474 464 476 475 464 477 476 464
             478 476 477 479 478 477 480 479 477 481 479 480
             482 481 480 482 483 484 482 485 483 482 486 487
             488 486 482 488 489 486 488 490 489 488 491 490
             492 491 488 492 493 491 492 494 493 495 494 492
             495 496 494 495 497 496 498 497 495 498 499 497
             498 500 499 501 500 498 501 502 500 503 502 501
             503 504 502 503 505 504 506 505 503 506 507 505
             508 507 506 508 509 507 510 509 508 510 511 509
             512 511 510 512 513 511 514 513 512 515 513 514
             515 516 513 517 516 515 518 516 517 518 519 516
             520 519 518 520 521 519 522 521 520 523 521 522
             523 524 521 525 524 523 526 524 525 527 524 526
             527 528 524 529 528 527 530 528 529 530 531 528
             532 531 530 533 531 534 535 531 533 535 536 531
             537 536 535 538 536 537 538 537 539 538 539 540
             538 540 541 538 541 542 538 542 543 538 543 544
             538 544 545 538 545 546 538 546 547 538 547 548
             538 548 549 538 549 550 538 551 536 552 551 538
             553 551 552 553 554 551 555 554 553 555 556 554
             557 556 558 557 559 556 560 559 557 561 559 560
             561 562 559 563 562 561 564 565 562 566 565 564
             567 565 566 567 568 565 569 570 571 569 572 570
             569 573 572 572 573 574 572 574 575 576 577 578
             576 579 577 576 580 579 576 581 580 580 582 579
             580 583 582 584 585 583 583 585 582 582 585 586
             582 586 587
          }
          indexCount 1479
          instanceCount 1
          firstIndex 0
          vertexOffset 0
          firstInstance 0
        }
        NumStateCommands 1
        StateCommand id=10
      }
    }
  }
  Child id=56 vsg::Group
  {
    NumUserObjects 0
    NumChildren 1
    Child id=57 vsg::CullNode
    {
      NumUserObjects 0
      Bound -0.968529 0.49093 0 1.00319
      Child id=58 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=59 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 2
          Array id=60 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 600
            Storage id=0
            Data -0.957825 0.916907 0.532613 -0.849199 0.916907 0.519184 -0.910227 0.964515 0.471585 -1.00624 0.916907 0.518445
             -0.969267 0.898561 0.547284 -0.851186 0.982841 0.395887 -0.985925 0.982841 0.412545 -1.04497 0.923078 0.488243
             -1.04881 0.916907 0.493178 -1.06162 0.982841 0.353504 -1.12066 0.923078 0.429203 -1.12451 0.916907 0.434137
             -1.19636 0.923078 0.370162 -1.00258 0.999963 0.277806 -1.16179 0.822246 0.481928 -0.926885 0.999963 0.336847
             -1.08609 0.822246 0.540969 -1.23749 0.822246 0.422888 -0.99896 0.822246 0.585354 -1.20021 0.916907 0.375097
             -0.90824 0.822246 0.594882 -0.834529 0.898561 0.530626 -0.893569 0.776008 0.606324 -0.80569 0.916907 0.493651
             -0.882127 0.822246 0.591654 -0.796458 0.822246 0.560319 -0.781803 0.727584 0.571749 -0.722763 0.822246 0.496051
             -0.775488 0.923078 0.454928 -0.770554 0.916907 0.458776 -0.792146 0.982841 0.320189 -0.716448 0.923078 0.37923
             -0.867844 0.999963 0.261149 -0.733105 0.982841 0.244491 -0.943542 0.999963 0.202108 -0.808804 0.999963 0.18545
             -0.884502 0.999963 0.12641 -1.01924 0.999963 0.143068 -0.9602 0.999963 0.067369 -0.825461 0.999963 0.050712
             -1.07828 0.999963 0.218766 -0.749763 0.999963 0.109752 -1.13732 0.982841 0.294464 -1.15398 0.999963 0.159725
             -1.21302 0.982841 0.235423 -1.27206 0.923078 0.311122 -1.09494 0.999963 0.084027 -1.22968 0.999963 0.100685
             -1.0359 0.999963 0.008329 -1.17064 0.999963 0.024987 -1.1116 0.999963 -0.050712 -0.976858 0.999963 -0.067369
             -1.05256 0.999963 -0.12641 -1.18729 0.999963 -0.109752 -0.901159 0.999963 -0.008329 -0.917817 0.999963 -0.143068
             -0.842119 0.999963 -0.084027 -0.766421 0.999963 -0.024987 -0.993515 0.999963 -0.202108 -0.858777 0.999963 -0.218766
             -0.934475 0.999963 -0.277806 -1.06921 0.999963 -0.261149 -0.783078 0.999963 -0.159725 -1.12825 0.999963 -0.18545
             -1.01017 0.999963 -0.336847 -1.14491 0.982841 -0.320189 -1.08587 0.982841 -0.395887 -0.951133 0.982841 -0.412545
             -1.20395 0.982841 -0.244491 -1.16157 0.923078 -0.454928 -1.22061 0.923078 -0.37923 -1.26299 0.982841 -0.168793
             -1.27965 0.923078 -0.303531 -1.24634 0.999963 -0.034054 -1.32203 0.982841 -0.093094 -1.30538 0.999963 0.041644
             -1.28872 0.982841 0.176383 -1.33869 0.923078 -0.227833 -1.34776 0.923078 0.252081 -1.34362 0.916907 -0.231682
             -1.27591 0.916907 0.316056 -1.36442 0.982841 0.117342 -1.31318 0.822246 0.363847 -1.35161 0.916907 0.257016
             -1.38888 0.822246 0.304807 -1.42346 0.923078 0.193041 -1.4273 0.916907 0.197975 -1.46218 0.916907 0.162839
             -1.46458 0.822246 0.245766 -1.44011 0.964514 0.058302 -1.38107 0.982841 -0.017396 -1.48771 0.916907 0.119329
             -1.39773 0.923078 -0.152135 -1.50114 0.916907 0.010704 -1.45677 0.923078 -0.076437 -1.40267 0.916907 -0.155983
             -1.46171 0.916907 -0.080285 -1.48697 0.916907 -0.037714 -1.51581 0.898561 -0.000739 -1.5095 0.822246 -0.11756
             -1.45046 0.822246 -0.193258 -1.39142 0.822246 -0.268956 -1.46415 0.727584 -0.203937 -1.28458 0.916907 -0.30738
             -1.22554 0.916907 -0.383078 -1.33238 0.822246 -0.344654 -1.1665 0.916907 -0.458776 -1.27333 0.822246 -0.420353
             -1.2143 0.822246 -0.496051 -1.13137 0.916907 -0.49365 -1.02683 0.964514 -0.471585 -1.08786 0.916907 -0.519184
             -0.979233 0.916907 -0.532613 -1.10253 0.898561 -0.530626 -0.96779 0.898561 -0.547284 -0.892092 0.923078 -0.488243
             -0.930815 0.916907 -0.518445 -0.888243 0.916907 -0.493178 -0.875434 0.982841 -0.353504 -0.812545 0.916907 -0.434137
             -0.799736 0.982841 -0.294464 -0.816394 0.923078 -0.429203 -0.850969 0.822246 -0.540969 -0.724038 0.982841 -0.235423
             -0.740696 0.923078 -0.370162 -0.736847 0.916907 -0.375097 -0.775271 0.822246 -0.481928 -0.661149 0.916907 -0.316056
             -0.664997 0.923078 -0.311122 -0.699573 0.822246 -0.422888 -0.64834 0.982841 -0.176383 -0.585451 0.916907 -0.257016
             -0.70738 0.999963 -0.100685 -0.589299 0.923078 -0.252081 -0.690723 0.999963 0.034054 -0.631682 0.999963 -0.041644
             -0.674065 0.982841 0.168793 -0.615024 0.982841 0.093094 -0.572642 0.982841 -0.117342 -0.657407 0.923078 0.303531
             -0.598367 0.923078 0.227833 -0.555984 0.982841 0.017396 -0.711513 0.916907 0.383078 -0.652473 0.916907 0.30738
             -0.539326 0.923078 0.152135 -0.663722 0.822246 0.420353 -0.604682 0.822246 0.344654 -0.593432 0.916907 0.231682
             -0.545641 0.822246 0.268956 -0.590989 0.727584 0.355334 -0.531949 0.727584 0.279635 -0.486601 0.822246 0.193258
             -0.65003 0.727584 0.431032 -0.534392 0.916907 0.155984 -0.70907 0.727584 0.50673 -0.480286 0.923078 0.076437
             -0.475351 0.916907 0.080285 -0.496943 0.964515 -0.058302 -0.450084 0.916907 0.037714 -0.42756 0.822246 0.11756
             -0.421245 0.898561 0.000739 -0.435916 0.916907 -0.010704 -0.513601 0.923078 -0.193041 -0.449345 0.916907 -0.119329
             -0.509752 0.916907 -0.197975 -0.474878 0.916907 -0.162839 -0.548176 0.822246 -0.304807 -0.437903 0.898561 -0.134
             -0.472478 0.822246 -0.245766 -0.376875 0.822246 -0.086402 -0.623874 0.822246 -0.363847 -0.40821 0.822246 -0.17207
             -0.373647 0.822246 -0.060289 -0.362205 0.776008 -0.07496 -0.383175 0.822246 0.030431 -0.39678 0.727584 -0.186726
             -0.36852 0.727584 0.041862 -0.413868 0.727584 0.128239 -0.35727 0.727584 -0.071111 -0.472908 0.727584 0.203937
             -0.358356 0.727584 -0.079894 -0.35727 0.632922 -0.071111 -0.36852 0.632922 0.041862 -0.413868 0.632922 0.128239
             -0.472908 0.632922 0.203937 -0.531949 0.632922 0.279635 -0.413868 0.538261 0.128239 -0.590989 0.632922 0.355334
             -0.472908 0.538261 0.203937 -0.531949 0.538261 0.279635 -0.65003 0.632922 0.431032 -0.590989 0.538261 0.355334
             -0.70907 0.632922 0.50673 -0.65003 0.538261 0.431032 -0.531949 0.443599 0.279635 -0.781803 0.632922 0.571749
             -0.590989 0.443599 0.355334 -0.70907 0.538261 0.50673 -0.65003 0.443599 0.431032 -0.888635 0.727584 0.610173
             -0.781803 0.538261 0.571749 -0.888635 0.632922 0.610173 -0.897418 0.727584 0.611259 -0.70907 0.443599 0.50673
             -0.888635 0.538261 0.610173 -0.781803 0.443599 0.571749 -0.897418 0.632922 0.611259 -1.01039 0.727584 0.600009
             -1.01039 0.632922 0.600009 -0.897418 0.538261 0.611259 -1.09677 0.727584 0.554661 -1.01039 0.538261 0.600009
             -1.09677 0.632922 0.554661 -1.09677 0.538261 0.554661 -1.17247 0.727584 0.49562 -1.17247 0.632922 0.49562
             -1.24816 0.727584 0.43658 -1.24816 0.632922 0.43658 -1.17247 0.538261 0.49562 -1.24816 0.538261 0.43658
             -1.32386 0.632922 0.377539 -1.32386 0.727584 0.377539 -1.32386 0.538261 0.377539 -1.39956 0.727584 0.318499
             -1.39956 0.632922 0.318499 -1.47526 0.727584 0.259458 -1.47526 0.632922 0.259458 -1.39956 0.538261 0.318499
             -1.47526 0.538261 0.259458 -1.54028 0.632922 0.186726 -1.54028 0.538261 0.186726 -1.47526 0.443599 0.259458
             -1.54028 0.727584 0.186726 -1.39956 0.443599 0.318499 -1.32386 0.443599 0.377539 -1.47526 0.348938 0.259458
             -1.24816 0.443599 0.43658 -1.39956 0.348938 0.318499 -1.32386 0.348938 0.377539 -1.17247 0.443599 0.49562
             -1.24816 0.348938 0.43658 -1.09677 0.443599 0.554661 -1.17247 0.348938 0.49562 -1.01039 0.443599 0.600009
             -1.09677 0.348938 0.554661 -1.24816 0.254276 0.43658 -0.897418 0.443599 0.611259 -1.17247 0.254276 0.49562
             -1.32386 0.254276 0.377539 -1.01039 0.348938 0.600009 -1.09677 0.254276 0.554661 -0.897418 0.348938 0.611259
             -1.23749 0.159614 0.422888 -1.16179 0.159614 0.481928 -1.01039 0.254276 0.600009 -1.08609 0.159614 0.540969
             -1.20021 0.064953 0.375097 -0.897418 0.254276 0.611259 -1.12451 0.064953 0.434137 -0.888635 0.254276 0.610173
             -0.893569 0.205852 0.606324 -0.99896 0.159614 0.585354 -0.888635 0.348938 0.610173 -0.90824 0.159614 0.594882
             -0.888635 0.443599 0.610173 -0.781803 0.348938 0.571749 -0.781803 0.254276 0.571749 -0.70907 0.348938 0.50673
             -0.70907 0.254276 0.50673 -0.796458 0.159614 0.560319 -0.722763 0.159614 0.496051 -0.882127 0.159614 0.591654
             -0.834529 0.083299 0.530626 -0.770554 0.064953 0.458776 -0.969267 0.083299 0.547284 -0.80569 0.064953 0.493651
             -0.849199 0.064953 0.519184 -0.957825 0.064953 0.532613 -0.910227 0.017345 0.471585 -1.00624 0.064953 0.518445
             -0.985925 -0.000981 0.412545 -1.04881 0.064953 0.493178 -1.04497 0.058782 0.488243 -1.12066 0.058782 0.429203
             -1.19636 0.058782 0.370162 -1.06162 -0.000981 0.353504 -1.13732 -0.000981 0.294464 -1.27206 0.058782 0.311122
             -1.21302 -0.000981 0.235423 -1.07828 -0.018103 0.218766 -1.27591 0.064953 0.316056 -1.00258 -0.018103 0.277806
             -1.31318 0.159614 0.363847 -1.35161 0.064953 0.257016 -1.38888 0.159614 0.304807 -1.39956 0.254276 0.318499
             -1.47526 0.254276 0.259458 -1.46458 0.159614 0.245766 -1.54028 0.254276 0.186726 -1.4273 0.064953 0.197975
             -1.54028 0.348938 0.186726 -1.52885 0.159614 0.17207 -1.54028 0.443599 0.186726 -1.5787 0.254276 0.079894
             -1.5787 0.348938 0.079894 -1.5787 0.443599 0.079894 -1.57485 0.205853 0.07496 -1.57979 0.254276 0.071111
             -1.57979 0.348938 0.071111 -1.56854 0.254276 -0.041861 -1.5787 0.538261 0.079894 -1.57979 0.443599 0.071111
             -1.5787 0.632922 0.079894 -1.57979 0.538261 0.071111 -1.56854 0.443599 -0.041861 -1.56854 0.348938 -0.041861
             -1.57979 0.632922 0.071111 -1.52319 0.254276 -0.128239 -1.5787 0.727584 0.079894 -1.52319 0.348938 -0.128239
             -1.57485 0.776007 0.07496 -1.56854 0.538261 -0.041861 -1.57979 0.727584 0.071111 -1.56854 0.632922 -0.041861
             -1.56854 0.727584 -0.041861 -1.52319 0.632922 -0.128239 -1.52319 0.443599 -0.128239 -1.52319 0.538261 -0.128239
             -1.46415 0.348938 -0.203937 -1.46415 0.443599 -0.203937 -1.46415 0.538261 -0.203937 -1.46415 0.254276 -0.203937
             -1.40511 0.348938 -0.279635 -1.40511 0.254276 -0.279635 -1.45046 0.159614 -0.193258 -1.40511 0.443599 -0.279635
             -1.5095 0.159614 -0.11756 -1.39142 0.159614 -0.268956 -1.55388 0.159614 -0.030431 -1.40267 0.064953 -0.155983
             -1.56341 0.159614 0.060289 -1.56018 0.159614 0.086402 -1.51581 0.083299 -0.000739 -1.49916 0.083299 0.134
             -1.46171 0.064953 -0.080285 -1.46218 0.064953 0.162839 -1.48771 0.064953 0.119329 -1.42346 0.058782 0.193041
             -1.44011 0.017346 0.058302 -1.50114 0.064953 0.010704 -1.48697 0.064953 -0.037714 -1.45677 0.058782 -0.076437
             -1.39773 0.058782 -0.152135 -1.38107 -0.000981 -0.017396 -1.36442 -0.000981 0.117342 -1.34776 0.058782 0.252081
             -1.28872 -0.000981 0.176383 -1.30538 -0.018103 0.041644 -1.22968 -0.018103 0.100685 -1.32203 -0.000981 -0.093094
             -1.24634 -0.018103 -0.034054 -1.15398 -0.018103 0.159725 -1.17064 -0.018103 0.024987 -1.09494 -0.018103 0.084027
             -1.01924 -0.018103 0.143068 -1.1116 -0.018103 -0.050712 -1.18729 -0.018103 -0.109752 -1.26299 -0.000981 -0.168793
             -1.0359 -0.018103 0.008329 -1.33869 0.058782 -0.227833 -1.34362 0.064953 -0.231682 -1.27965 0.058782 -0.303531
             -1.20395 -0.000981 -0.244491 -1.28458 0.064953 -0.30738 -1.22061 0.058782 -0.37923 -1.12825 -0.018103 -0.18545
             -1.14491 -0.000981 -0.320189 -1.05256 -0.018103 -0.12641 -1.06921 -0.018103 -0.261149 -1.16157 0.058782 -0.454928
             -0.976858 -0.018103 -0.067369 -1.08587 -0.000981 -0.395887 -0.993515 -0.018103 -0.202108 -1.01017 -0.018103 -0.336847
             -0.9602 -0.018103 0.067369 -0.917817 -0.018103 -0.143068 -0.943542 -0.018103 0.202108 -0.901159 -0.018103 -0.008329
             -0.926885 -0.018103 0.336847 -0.884502 -0.018103 0.12641 -0.867844 -0.018103 0.261149 -0.851186 -0.000981 0.395887
             -0.775488 0.058782 0.454928 -0.792146 -0.000981 0.320189 -0.716448 0.058782 0.37923 -0.808804 -0.018103 0.18545
             -0.733105 -0.000981 0.244491 -0.711513 0.064953 0.383078 -0.657407 0.058782 0.303531 -0.663722 0.159614 0.420353
             -0.652473 0.064953 0.30738 -0.65003 0.254276 0.431032 -0.604682 0.159614 0.344654 -0.65003 0.348938 0.431032
             -0.590989 0.254276 0.355334 -0.590989 0.348938 0.355334 -0.545641 0.159614 0.268956 -0.531949 0.348938 0.279635
             -0.531949 0.254276 0.279635 -0.593432 0.064953 0.231682 -0.486601 0.159614 0.193258 -0.598367 0.058782 0.227833
             -0.534392 0.064953 0.155984 -0.539326 0.058782 0.152135 -0.674065 -0.000981 0.168793 -0.615024 -0.000981 0.093094
             -0.749763 -0.018103 0.109752 -0.690723 -0.018103 0.034054 -0.825461 -0.018103 0.050712 -0.766421 -0.018103 -0.024987
             -0.842119 -0.018103 -0.084027 -0.70738 -0.018103 -0.100685 -0.783078 -0.018103 -0.159725 -0.858777 -0.018103 -0.218766
             -0.631682 -0.018103 -0.041644 -0.724038 -0.000981 -0.235423 -0.64834 -0.000981 -0.176383 -0.555984 -0.000981 0.017396
             -0.572642 -0.000981 -0.117342 -0.480286 0.058782 0.076437 -0.475351 0.064953 0.080285 -0.450084 0.064953 0.037714
             -0.42756 0.159614 0.11756 -0.496943 0.017345 -0.058302 -0.435916 0.064953 -0.010704 -0.449345 0.064953 -0.119329
             -0.421245 0.083299 0.000739 -0.437903 0.083299 -0.134 -0.513601 0.058782 -0.193041 -0.474878 0.064953 -0.162839
             -0.589299 0.058782 -0.252081 -0.509752 0.064953 -0.197975 -0.664997 0.058782 -0.311122 -0.585451 0.064953 -0.257016
             -0.472478 0.159614 -0.245766 -0.740696 0.058782 -0.370162 -0.661149 0.064953 -0.316056 -0.799736 -0.000981 -0.294464
             -0.548176 0.159614 -0.304807 -0.736847 0.064953 -0.375097 -0.816394 0.058782 -0.429203 -0.875434 -0.000981 -0.353504
             -0.623874 0.159614 -0.363847 -0.812545 0.064953 -0.434137 -0.699573 0.159614 -0.422888 -0.613195 0.254276 -0.377539
             -0.892092 0.058782 -0.488243 -0.775271 0.159614 -0.481928 -0.934475 -0.018103 -0.277806 -0.951133 -0.000981 -0.412545
             -1.02683 0.017346 -0.471585 -0.888243 0.064953 -0.493178 -0.930815 0.064953 -0.518445 -0.850969 0.159614 -0.540969
             -0.688894 0.254276 -0.43658 -0.764592 0.254276 -0.49562 -0.613195 0.348938 -0.377539 -0.688894 0.348938 -0.43658
             -0.84029 0.254276 -0.554661 -0.764592 0.348938 -0.49562 -0.613195 0.443599 -0.377539 -0.688894 0.443599 -0.43658
             -0.84029 0.348938 -0.554661 -0.764592 0.443599 -0.49562 -0.613195 0.538261 -0.377539 -0.688894 0.538261 -0.43658
             -0.84029 0.443599 -0.554661 -0.764592 0.538261 -0.49562 -0.926667 0.254276 -0.600009 -0.926667 0.348938 -0.600009
             -0.84029 0.538261 -0.554661 -0.926667 0.443599 -0.600009 -1.03964 0.254276 -0.611259 -1.03964 0.348938 -0.611259
             -0.938098 0.159614 -0.585354 -1.04349 0.205853 -0.606324 -1.04842 0.254276 -0.610173 -0.96779 0.083299 -0.547284
             -1.02882 0.159614 -0.594882 -1.05493 0.159614 -0.591653 -0.979233 0.064953 -0.532613 -1.10253 0.083299 -0.530626
             -1.08786 0.064953 -0.519184 -1.13137 0.064953 -0.49365 -1.1406 0.159614 -0.560319 -1.1665 0.064953 -0.458776
             -1.2143 0.159614 -0.496051 -1.15525 0.254276 -0.571749 -1.22554 0.064953 -0.383078 -1.27333 0.159614 -0.420353
             -1.33238 0.159614 -0.344654 -1.28703 0.254276 -0.431032 -1.34607 0.254276 -0.355333 -1.34607 0.348938 -0.355333
             -1.22799 0.254276 -0.50673 -1.28703 0.348938 -0.431032 -1.34607 0.443599 -0.355333 -1.22799 0.348938 -0.50673
             -1.40511 0.538261 -0.279635 -1.28703 0.443599 -0.431032 -1.34607 0.538261 -0.355333 -1.46415 0.632922 -0.203937
             -1.40511 0.632922 -0.279635 -1.52319 0.727584 -0.128239 -1.55388 0.822246 -0.030431 -1.56341 0.822246 0.060289
             -1.49916 0.898561 0.134 -1.56018 0.822246 0.086402 -1.52885 0.822246 0.17207 -1.40511 0.727584 -0.279635
             -1.34607 0.632922 -0.355333 -1.34607 0.727584 -0.355333 -1.28703 0.538261 -0.431032 -1.28703 0.632922 -0.431032
             -1.28703 0.727584 -0.431032 -1.22799 0.443599 -0.50673 -1.22799 0.538261 -0.50673 -1.22799 0.632922 -0.50673
             -1.22799 0.727584 -0.50673 -1.15525 0.443599 -0.571749 -1.15525 0.538261 -0.571749 -1.15525 0.632922 -0.571749
             -1.15525 0.348938 -0.571749 -1.04842 0.348938 -0.610173 -1.04842 0.443599 -0.610173 -1.03964 0.443599 -0.611259
             -1.04842 0.538261 -0.610173 -1.03964 0.538261 -0.611259 -0.926667 0.538261 -0.600009 -1.03964 0.632922 -0.611259
             -1.04842 0.632922 -0.610173 -0.84029 0.632922 -0.554661 -0.926667 0.632922 -0.600009 -0.764592 0.632922 -0.49562
             -0.84029 0.727584 -0.554661 -0.688894 0.632922 -0.43658 -0.764592 0.727584 -0.49562 -0.688894 0.727584 -0.43658
             -0.613195 0.632922 -0.377539 -0.613195 0.727584 -0.377539 -0.926667 0.727584 -0.600009 -0.537497 0.727584 -0.318499
             -0.537497 0.632922 -0.318499 -0.461799 0.727584 -0.259458 -0.537497 0.538261 -0.318499 -0.461799 0.632922 -0.259458
             -0.537497 0.443599 -0.318499 -0.461799 0.538261 -0.259458 -0.39678 0.632922 -0.186726 -0.537497 0.348938 -0.318499
             -0.358356 0.632922 -0.079894 -0.537497 0.254276 -0.318499 -0.461799 0.443599 -0.259458 -0.461799 0.348938 -0.259458
             -0.461799 0.254276 -0.259458 -0.39678 0.538261 -0.186726 -0.39678 0.443599 -0.186726 -0.39678 0.348938 -0.186726
             -0.358356 0.538261 -0.079894 -0.35727 0.538261 -0.071111 -0.358356 0.443599 -0.079894 -0.35727 0.443599 -0.071111
             -0.36852 0.538261 0.041862 -0.36852 0.443599 0.041862 -0.35727 0.348938 -0.071111 -0.413868 0.443599 0.128239
             -0.36852 0.348938 0.041862 -0.358356 0.348938 -0.079894 -0.472908 0.443599 0.203937 -0.413868 0.348938 0.128239
             -0.472908 0.348938 0.203937 -0.413868 0.254276 0.128239 -0.472908 0.254276 0.203937 -0.36852 0.254276 0.041862
             -0.383175 0.159614 0.030431 -0.35727 0.254276 -0.071111 -0.362205 0.205852 -0.07496 -0.373647 0.159614 -0.060289
             -0.376875 0.159614 -0.086402 -0.40821 0.159614 -0.17207 -0.358356 0.254276 -0.079894 -0.39678 0.254276 -0.186726
             -1.15525 0.727584 -0.571749 -1.1406 0.822246 -0.560319 -1.04842 0.727584 -0.610173 -1.05493 0.822246 -0.591653
             -1.04349 0.776007 -0.606324 -1.02882 0.822246 -0.594882 -0.938098 0.822246 -0.585354 -1.03964 0.727584 -0.611259
          }
          Array id=61 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0 -1
            Size 600
            Storage id=0
            Data -0.0453 0.7214 0.691 0.2138 0.7273 0.6522 0.0632 0.8799 0.4709 -0.3493 0.6235 0.6995
             -0.148 0.5801 0.801 0.2261 0.932 0.2832 -0.1509 0.9326 0.3277 -0.3402 0.786 0.5162
             -0.4325 0.6201 0.6545 -0.2205 0.9335 0.2827 -0.3807 0.7853 0.4882 -0.4778 0.6295 0.6127
             -0.3807 0.7853 0.4882 -0.0542 0.9961 0.0695 -0.5722 0.3667 0.7336 0.0151 0.9923 0.1226
             -0.5029 0.3624 0.7847 -0.5722 0.3667 0.7336 -0.2744 0.3468 0.8969 -0.4778 0.6295 0.6127
             0.0139 0.3961 0.9181 0.3229 0.5821 0.7462 0.1267 0.1711 0.9771 0.4398 0.7055 0.5557
             0.2167 0.3895 0.8951 0.4802 0.3396 0.8087 0.5145 0.0865 0.8531 0.6816 0.3592 0.6374
             0.4557 0.786 0.4178 0.5829 0.6266 0.5173 0.2827 0.9335 0.2205 0.4882 0.7853 0.3807
             0.0695 0.9961 0.0542 0.2827 0.9335 0.2205 0 1 0 0.0695 0.9961 0.0542
             0 1 0 0 1 0 0 1 0 0 1 0
             -0.0542 0.9961 0.0695 0.0695 0.9961 0.0542 -0.2205 0.9335 0.2827 -0.0542 0.9961 0.0695
             -0.2205 0.9335 0.2827 -0.3807 0.7853 0.4882 0 1 0 -0.0542 0.9961 0.0695
             0 1 0 0 1 0 0 1 0 0 1 0
             0 1 0 -0.0695 0.9961 -0.0542 0 1 0 0 1 0
             0 1 0 0 1 0 0 1 0 0.0542 0.9961 -0.0695
             0.0542 0.9961 -0.0695 -0.0695 0.9961 -0.0542 0.0542 0.9961 -0.0695 -0.0695 0.9961 -0.0542
             -0.0151 0.9923 -0.1226 -0.2827 0.9335 -0.2205 -0.2262 0.9326 -0.2811 0.1508 0.9312 -0.3319
             -0.2827 0.9335 -0.2205 -0.4557 0.786 -0.4178 -0.4882 0.7853 -0.3807 -0.2827 0.9335 -0.2205
             -0.4882 0.7853 -0.3807 -0.0695 0.9961 -0.0542 -0.2827 0.9335 -0.2205 -0.1226 0.9923 0.0151
             -0.2205 0.9335 0.2827 -0.4882 0.7853 -0.3807 -0.3807 0.7853 0.4882 -0.6126 0.6295 -0.4778
             -0.4778 0.6295 0.6127 -0.2811 0.9326 0.2262 -0.5722 0.3667 0.7336 -0.4778 0.6295 0.6126
             -0.5722 0.3667 0.7336 -0.4178 0.786 0.4557 -0.5173 0.6266 0.5829 -0.5609 0.6972 0.4464
             -0.6384 0.3604 0.6801 -0.4721 0.8799 0.0533 -0.3319 0.9312 -0.1508 -0.6592 0.7214 0.2123
             -0.4882 0.7853 -0.3807 -0.6808 0.7304 -0.0549 -0.5184 0.7902 -0.3267 -0.6127 0.6295 -0.4778
             -0.6436 0.6266 -0.4394 -0.5857 0.7687 -0.2568 -0.803 0.5791 -0.1403 -0.7848 0.3604 -0.5042
             -0.7336 0.3667 -0.5722 -0.7336 0.3667 -0.5722 -0.7853 0.0906 -0.6125 -0.6127 0.6295 -0.4778
             -0.6126 0.6295 -0.4778 -0.7336 0.3667 -0.5722 -0.5829 0.6266 -0.5173 -0.7336 0.3667 -0.5722
             -0.6801 0.3604 -0.6384 -0.4464 0.6972 -0.5609 -0.0533 0.8799 -0.4721 -0.2123 0.7214 -0.6592
             0.0549 0.7304 -0.6808 -0.3316 0.5791 -0.7447 0.148 0.5801 -0.801 0.3267 0.7902 -0.5184
             0.3137 0.7025 -0.6388 0.4325 0.6201 -0.6545 0.2205 0.9335 -0.2827 0.4778 0.6295 -0.6127
             0.2205 0.9335 -0.2827 0.3807 0.7853 -0.4882 0.5029 0.3624 -0.7847 0.2205 0.9335 -0.2827
             0.3807 0.7853 -0.4882 0.4778 0.6295 -0.6127 0.5722 0.3667 -0.7336 0.4778 0.6295 -0.6126
             0.3807 0.7853 -0.4882 0.5722 0.3667 -0.7336 0.2205 0.9335 -0.2827 0.4778 0.6295 -0.6127
             0.0542 0.9961 -0.0695 0.3807 0.7853 -0.4882 0.0695 0.9961 0.0542 0.1226 0.9923 -0.0151
             0.2827 0.9335 0.2205 0.2827 0.9335 0.2205 0.2859 0.9309 -0.2271 0.4882 0.7853 0.3807
             0.4882 0.7853 0.3807 0.3327 0.9309 0.1507 0.6126 0.6295 0.4778 0.6126 0.6295 0.4778
             0.4882 0.7853 0.3807 0.7336 0.3667 0.5722 0.7336 0.3667 0.5722 0.6126 0.6295 0.4778
             0.7336 0.3667 0.5722 0.7853 0.0906 0.6125 0.7853 0.0906 0.6125 0.7336 0.3667 0.5722
             0.7853 0.0906 0.6125 0.6126 0.6295 0.4778 0.7277 0.0898 0.6799 0.5162 0.7898 0.3312
             0.6545 0.6201 0.4325 0.4703 0.8806 -0.0581 0.6902 0.6333 0.35 0.7847 0.3624 0.5029
             0.801 0.5801 0.148 0.691 0.7214 0.0453 0.42 0.7898 -0.4469 0.6592 0.7214 -0.2123
             0.5173 0.6266 -0.5829 0.5513 0.7062 -0.4441 0.5722 0.3667 -0.7336 0.7447 0.5791 -0.3316
             0.6384 0.3604 -0.6801 0.8938 0.3961 -0.2101 0.5722 0.3667 -0.7336 0.8031 0.3468 -0.4846
             0.9181 0.3961 -0.0139 0.9783 0.1681 -0.1209 0.8969 0.3468 0.2744 0.8526 0.0847 -0.5157
             0.9525 0.0847 0.2925 0.8367 0.0898 0.5403 0.9987 0.0503 -0.0091 0.7853 0.0906 0.6125
             0.9708 0.0503 -0.2344 0.9999 -0 -0.0119 0.9579 -0 0.2872 0.8403 -0 0.5421
             0.7885 -0 0.615 0.7885 -0 0.615 0.8403 -0 0.5421 0.7885 -0 0.615
             0.7885 -0 0.615 0.7885 -0 0.615 0.7885 -0 0.615 0.7885 -0 0.615
             0.7304 -0 0.683 0.7885 -0 0.615 0.7885 -0 0.615 0.5119 -0 0.859
             0.7885 -0 0.615 0.7304 -0 0.683 0.7885 -0 0.615 0.2344 0.0503 0.9708
             0.5119 -0 0.859 0.232 -0 0.9727 0.0091 0.0503 0.9987 0.7304 -0 0.683
             0.232 -0 0.9727 0.5119 -0 0.859 0.0119 -0 0.9999 -0.2925 0.0847 0.9525
             -0.2872 -0 0.9579 0.0119 -0 0.9999 -0.5403 0.0898 0.8367 -0.2872 -0 0.9579
             -0.5421 -0 0.8403 -0.5421 -0 0.8403 -0.6125 0.0906 0.7853 -0.615 -0 0.7885
             -0.6125 0.0906 0.7853 -0.615 -0 0.7885 -0.615 -0 0.7885 -0.615 -0 0.7885
             -0.615 -0 0.7885 -0.6125 0.0906 0.7853 -0.615 -0 0.7885 -0.6125 0.0906 0.7853
             -0.615 -0 0.7885 -0.6799 0.0898 0.7277 -0.683 -0 0.7304 -0.615 -0 0.7885
             -0.683 -0 0.7304 -0.859 -0 0.5119 -0.859 -0 0.5119 -0.683 -0 0.7304
             -0.8526 0.0847 0.5157 -0.615 -0 0.7885 -0.615 -0 0.7885 -0.683 -0 0.7304
             -0.615 -0 0.7885 -0.615 -0 0.7885 -0.615 -0 0.7885 -0.615 -0 0.7885
             -0.615 -0 0.7885 -0.5421 -0 0.8403 -0.615 -0 0.7885 -0.2872 -0 0.9579
             -0.5421 -0 0.8403 -0.6125 -0.0906 0.7853 0.0119 -0 0.9999 -0.6125 -0.0906 0.7853
             -0.6125 -0.0906 0.7853 -0.2872 -0 0.9579 -0.5403 -0.0898 0.8367 0.0119 -0 0.9999
             -0.5722 -0.3667 0.7336 -0.5722 -0.3667 0.7336 -0.2925 -0.0847 0.9525 -0.5042 -0.3604 0.7848
             -0.4778 -0.6295 0.6127 0.0091 -0.0503 0.9987 -0.4778 -0.6295 0.6127 0.2409 -0.0575 0.9688
             0.1328 -0.1619 0.9778 -0.2744 -0.3468 0.8969 0.232 -0 0.9727 0.0139 -0.3961 0.9181
             0.232 -0 0.9727 0.5119 -0 0.859 0.512 -0.088 0.8544 0.7304 -0 0.683
             0.7277 -0.0898 0.6799 0.4834 -0.3423 0.8057 0.6783 -0.363 0.6389 0.2101 -0.3961 0.8938
             0.3229 -0.5821 0.7462 0.5816 -0.6211 0.5253 -0.1403 -0.5791 0.803 0.4563 -0.694 0.5568
             0.2123 -0.7214 0.6592 -0.0486 -0.7273 0.6846 0.0533 -0.8799 0.4721 -0.2912 -0.7009 0.651
             -0.1503 -0.932 0.3297 -0.4394 -0.6266 0.6436 -0.3402 -0.786 0.5162 -0.3807 -0.7853 0.4882
             -0.3807 -0.7853 0.4882 -0.2205 -0.9335 0.2827 -0.2205 -0.9335 0.2827 -0.3807 -0.7853 0.4882
             -0.2205 -0.9335 0.2827 -0.0542 -0.9961 0.0695 -0.4778 -0.6295 0.6126 -0.0542 -0.9961 0.0695
             -0.5722 -0.3667 0.7336 -0.4778 -0.6295 0.6126 -0.5722 -0.3667 0.7336 -0.6125 -0.0906 0.7853
             -0.6799 -0.0898 0.7277 -0.6386 -0.3624 0.6788 -0.8526 -0.0847 0.5157 -0.5295 -0.6201 0.5788
             -0.859 -0 0.5119 -0.8031 -0.3468 0.4846 -0.859 -0 0.5119 -0.9708 -0.0503 0.2344
             -0.9727 -0 0.232 -0.9727 -0 0.232 -0.9783 -0.1681 0.1209 -0.9987 -0.0503 0.0091
             -0.9999 -0 0.0119 -0.9525 -0.0847 -0.2925 -0.9727 -0 0.232 -0.9999 -0 0.0119
             -0.9727 -0 0.232 -0.9999 -0 0.0119 -0.9579 -0 -0.2872 -0.9579 -0 -0.2872
             -0.9999 -0 0.0118 -0.8367 -0.0898 -0.5403 -0.9708 0.0503 0.2344 -0.8403 -0 -0.5421
             -0.9783 0.1681 0.1209 -0.9579 -0 -0.2872 -0.9987 0.0503 0.0091 -0.9579 -0 -0.2872
             -0.9525 0.0847 -0.2925 -0.8403 -0 -0.5421 -0.8403 -0 -0.5421 -0.8403 -0 -0.5421
             -0.7885 -0 -0.615 -0.7885 -0 -0.615 -0.7885 -0 -0.615 -0.7853 -0.0906 -0.6125
             -0.7885 -0 -0.615 -0.7853 -0.0906 -0.6125 -0.7336 -0.3667 -0.5722 -0.7885 -0 -0.615
             -0.7847 -0.3624 -0.5029 -0.7336 -0.3667 -0.5722 -0.8969 -0.3468 -0.2744 -0.6127 -0.6295 -0.4778
             -0.9181 -0.3961 0.0139 -0.8939 -0.3961 0.2101 -0.801 -0.5801 -0.148 -0.7408 -0.5801 0.3386
             -0.6545 -0.6201 -0.4325 -0.5842 -0.6333 0.5076 -0.6592 -0.7214 0.2123 -0.42 -0.7898 0.4469
             -0.4703 -0.8806 0.0581 -0.691 -0.7214 -0.0453 -0.6902 -0.6333 -0.35 -0.5162 -0.7898 -0.3312
             -0.4882 -0.7853 -0.3807 -0.3327 -0.9309 -0.1507 -0.2859 -0.9309 0.2271 -0.3807 -0.7853 0.4882
             -0.2205 -0.9335 0.2827 -0.1226 -0.9923 0.0151 -0.0542 -0.9961 0.0695 -0.2827 -0.9335 -0.2205
             -0.0695 -0.9961 -0.0542 -0.0542 -0.9961 0.0695 0 -1 0 0 -1 0
             0 -1 0 0 -1 0 -0.0695 -0.9961 -0.0542 -0.2827 -0.9335 -0.2205
             0 -1 0 -0.4882 -0.7853 -0.3807 -0.6127 -0.6295 -0.4778 -0.4882 -0.7853 -0.3807
             -0.2827 -0.9335 -0.2205 -0.6126 -0.6295 -0.4778 -0.4882 -0.7853 -0.3807 -0.0695 -0.9961 -0.0542
             -0.2827 -0.9335 -0.2205 0 -1 0 -0.0695 -0.9961 -0.0542 -0.4469 -0.7898 -0.42
             0 -1 0 -0.2271 -0.9309 -0.2859 0 -1 0 -0.0151 -0.9923 -0.1226
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             0.0151 -0.9923 0.1226 0 -1 0 0.0695 -0.9961 0.0542 0.2262 -0.9326 0.2811
             0.4557 -0.786 0.4178 0.2827 -0.9335 0.2205 0.4882 -0.7853 0.3807 0.0695 -0.9961 0.0542
             0.2827 -0.9335 0.2205 0.6126 -0.6295 0.4778 0.4882 -0.7853 0.3807 0.7336 -0.3667 0.5722
             0.6126 -0.6295 0.4778 0.7853 -0.0906 0.6125 0.7336 -0.3667 0.5722 0.7885 -0 0.615
             0.7853 -0.0906 0.6125 0.7885 -0 0.615 0.7336 -0.3667 0.5722 0.7885 -0 0.615
             0.7853 -0.0906 0.6125 0.6126 -0.6295 0.4778 0.7336 -0.3667 0.5722 0.4882 -0.7853 0.3807
             0.6126 -0.6295 0.4778 0.4882 -0.7853 0.3807 0.2827 -0.9335 0.2205 0.2827 -0.9335 0.2205
             0.0695 -0.9961 0.0542 0.0695 -0.9961 0.0542 0 -1 0 0 -1 0
             0 -1 0 0.0542 -0.9961 -0.0695 0.0542 -0.9961 -0.0695 0.0542 -0.9961 -0.0695
             0.1226 -0.9923 -0.0151 0.2205 -0.9335 -0.2827 0.2205 -0.9335 -0.2827 0.3277 -0.9326 0.1509
             0.2852 -0.9312 -0.2271 0.5162 -0.786 0.3402 0.6436 -0.6266 0.4394 0.6527 -0.6972 0.2964
             0.7848 -0.3604 0.5042 0.4709 -0.8799 -0.0632 0.691 -0.7214 0.0453 0.6469 -0.7304 -0.2191
             0.803 -0.5791 0.1403 0.7408 -0.5801 -0.3386 0.4232 -0.7902 -0.4432 0.5432 -0.7025 -0.4599
             0.3807 -0.7853 -0.4882 0.5295 -0.6201 -0.5788 0.3807 -0.7853 -0.4882 0.4778 -0.6295 -0.6126
             0.6386 -0.3624 -0.6788 0.3807 -0.7853 -0.4882 0.4778 -0.6295 -0.6126 0.2205 -0.9335 -0.2827
             0.5722 -0.3667 -0.7336 0.4778 -0.6295 -0.6126 0.3807 -0.7853 -0.4882 0.2205 -0.9335 -0.2827
             0.5722 -0.3667 -0.7336 0.4778 -0.6295 -0.6127 0.5722 -0.3667 -0.7336 0.6125 -0.0906 -0.7853
             0.3312 -0.7898 -0.5162 0.5722 -0.3667 -0.7336 0.0542 -0.9961 -0.0695 0.1507 -0.9309 -0.3327
             -0.0581 -0.8806 -0.4703 0.4394 -0.6266 -0.6436 0.2965 -0.7062 -0.6429 0.5042 -0.3604 -0.7848
             0.6125 -0.0906 -0.7853 0.6125 -0.0906 -0.7853 0.615 -0 -0.7885 0.615 -0 -0.7885
             0.5403 -0.0898 -0.8367 0.615 -0 -0.7885 0.615 -0 -0.7885 0.615 -0 -0.7885
             0.5421 -0 -0.8403 0.615 -0 -0.7885 0.615 -0 -0.7885 0.615 -0 -0.7885
             0.5421 -0 -0.8403 0.615 -0 -0.7885 0.2925 -0.0847 -0.9525 0.2872 -0 -0.9579
             0.5421 -0 -0.8403 0.2872 -0 -0.9579 -0.0091 -0.0503 -0.9987 -0.0119 -0 -0.9999
             0.2744 -0.3468 -0.8969 -0.1209 -0.1681 -0.9783 -0.2344 -0.0503 -0.9708 0.1403 -0.5791 -0.803
             -0.0139 -0.3961 -0.9181 -0.2101 -0.3961 -0.8938 0.0453 -0.7214 -0.691 -0.3386 -0.5801 -0.7408
             -0.2123 -0.7214 -0.6592 -0.5076 -0.6333 -0.5842 -0.4846 -0.3468 -0.8031 -0.5788 -0.6201 -0.5295
             -0.6788 -0.3624 -0.6386 -0.5157 -0.0847 -0.8526 -0.6127 -0.6295 -0.4778 -0.7336 -0.3667 -0.5722
             -0.7336 -0.3667 -0.5722 -0.7853 -0.0906 -0.6125 -0.7853 -0.0906 -0.6125 -0.7885 -0 -0.615
             -0.7277 -0.0898 -0.6799 -0.7885 -0 -0.615 -0.7885 -0 -0.615 -0.7304 -0 -0.683
             -0.7885 -0 -0.615 -0.7885 -0 -0.615 -0.7885 -0 -0.615 -0.7885 -0 -0.615
             -0.7885 -0 -0.615 -0.8367 0.0898 -0.5403 -0.8969 0.3468 -0.2744 -0.9181 0.3961 0.0139
             -0.7447 0.5791 0.3316 -0.8938 0.3961 0.2101 -0.8031 0.3468 0.4846 -0.7853 0.0906 -0.6125
             -0.7885 -0 -0.615 -0.7853 0.0906 -0.6125 -0.7885 -0 -0.615 -0.7885 -0 -0.615
             -0.7853 0.0906 -0.6125 -0.7304 -0 -0.683 -0.7304 -0 -0.683 -0.7304 -0 -0.683
             -0.7277 0.0898 -0.6799 -0.5119 -0 -0.859 -0.5119 -0 -0.859 -0.5119 -0 -0.859
             -0.5119 -0 -0.859 -0.232 -0 -0.9727 -0.232 -0 -0.9727 -0.0119 -0 -0.9999
             -0.232 -0 -0.9727 -0.0119 -0 -0.9999 0.2872 -0 -0.9579 -0.0119 -0 -0.9999
             -0.232 -0 -0.9727 0.5421 -0 -0.8403 0.2872 -0 -0.9579 0.615 -0 -0.7885
             0.5403 0.0898 -0.8367 0.615 -0 -0.7885 0.6125 0.0906 -0.7853 0.6125 0.0906 -0.7853
             0.615 -0 -0.7885 0.6125 0.0906 -0.7853 0.2925 0.0847 -0.9525 0.6125 0.0906 -0.7853
             0.615 -0 -0.7885 0.6799 0.0898 -0.7277 0.615 -0 -0.7885 0.683 -0 -0.7304
             0.615 -0 -0.7885 0.683 -0 -0.7304 0.859 -0 -0.5119 0.615 -0 -0.7885
             0.9727 -0 -0.232 0.6125 -0.0906 -0.7853 0.683 -0 -0.7304 0.683 -0 -0.7304
             0.6799 -0.0898 -0.7277 0.859 -0 -0.5119 0.859 -0 -0.5119 0.859 -0 -0.5119
             0.9727 -0 -0.232 0.9999 -0 -0.0119 0.9727 -0 -0.232 0.9999 -0 -0.0119
             0.9579 -0 0.2872 0.9579 -0 0.2872 0.9999 -0 -0.0119 0.8403 -0 0.5421
             0.9579 -0 0.2872 0.9727 -0 -0.232 0.7885 -0 0.615 0.8403 -0 0.5421
             0.7885 -0 0.615 0.8367 -0.0898 0.5403 0.7853 -0.0906 0.6125 0.9525 -0.0847 0.2925
             0.8969 -0.3468 0.2744 0.9987 -0.0503 -0.0091 0.9783 -0.1681 -0.1209 0.9181 -0.3961 -0.0139
             0.8938 -0.3961 -0.2101 0.8031 -0.3468 -0.4846 0.9708 -0.0503 -0.2344 0.8526 -0.0847 -0.5157
             -0.5157 0.0847 -0.8526 -0.4846 0.3468 -0.8031 -0.2344 0.0503 -0.9708 -0.2101 0.3961 -0.8938
             -0.1209 0.1681 -0.9783 -0.0139 0.3961 -0.9181 0.2744 0.3468 -0.8969 -0.0091 0.0503 -0.9987
          }
          Indices id=62 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0 -1
            Size 3588
            Storage id=0
            Data 0 1 2 3 0 2 1 0 4 0 3 4
             2 1 5 3 2 6 6 2 5 7 3 6
             8 3 7 3 8 4 7 6 9 8 7 10
             7 9 10 8 10 11 12 10 9 11 10 12
             6 13 9 8 11 14 6 15 13 6 5 15
             8 14 16 16 4 8 11 17 14 16 18 4
             11 19 17 11 12 19 18 20 4 20 21 4
             1 4 21 20 18 22 23 1 21 23 5 1
             20 24 21 24 20 22 24 25 21 23 21 25
             25 24 26 22 26 24 23 25 27 27 25 26
             23 28 5 29 23 27 23 29 28 28 30 5
             29 31 28 28 31 30 5 30 32 5 32 15
             31 33 30 15 32 34 15 34 13 30 35 32
             30 33 35 32 36 34 32 35 36 13 34 37
             34 36 38 34 38 37 35 39 36 13 37 40
             9 13 40 35 41 39 33 41 35 9 40 42
             12 9 42 42 40 43 12 42 44 42 43 44
             12 44 45 19 12 45 40 46 43 40 37 46
             44 43 47 37 48 46 37 38 48 43 46 49
             43 49 47 46 48 50 46 50 49 38 51 48
             48 52 50 48 51 52 49 50 53 38 54 51
             36 54 38 36 39 54 54 55 51 39 56 54
             54 56 55 39 57 56 41 57 39 51 55 58
             51 58 52 56 59 55 55 60 58 55 59 60
             52 58 61 56 62 59 57 62 56 50 52 63
             52 61 63 50 63 53 58 64 61 58 60 64
             63 61 65 61 64 66 61 66 65 60 67 64
             66 64 67 63 65 68 53 63 68 65 66 69
             68 65 70 65 69 70 53 68 71 68 70 72
             71 68 72 73 53 71 49 53 73 47 49 73
             73 71 74 47 73 75 75 73 74 76 47 75
             44 47 76 45 44 76 74 71 77 71 72 77
             45 76 78 79 77 72 80 45 78 19 45 80
             78 76 81 76 75 81 19 80 82 19 82 17
             80 78 83 80 84 82 80 83 84 83 78 85
             78 81 85 83 85 86 87 85 81 87 86 85
             83 86 88 88 86 87 83 88 84 87 81 89
             81 75 90 81 90 89 75 74 90 91 87 89
             90 74 92 74 77 92 93 89 90 93 91 89
             90 92 94 93 90 94 95 92 77 95 77 79
             96 94 92 96 92 95 97 93 94 96 97 94
             93 97 98 91 93 98 97 96 99 97 99 98
             96 95 100 96 100 99 95 101 100 95 79 101
             99 100 102 102 100 101 101 79 103 79 72 103
             103 72 70 103 70 104 104 70 69 101 103 105
             104 69 106 103 104 107 103 107 105 104 106 108
             104 108 107 109 106 69 108 106 109 109 69 66
             109 66 110 66 67 110 111 109 110 112 110 67
             112 111 110 109 111 113 108 109 113 111 112 114
             111 114 113 112 67 115 112 116 114 116 112 115
             116 117 114 117 116 115 118 115 67 60 118 67
             59 118 60 119 117 115 59 120 118 118 121 115
             120 121 118 119 115 121 62 120 59 119 122 117
             122 114 117 62 123 120 120 124 121 123 124 120
             125 119 121 125 121 124 125 126 119 119 126 122
             127 125 124 123 128 124 127 124 128 125 129 126
             127 129 125 130 128 123 62 130 123 131 127 128
             62 132 130 57 132 62 130 133 128 131 128 133
             134 132 57 41 134 57 135 130 132 134 135 132
             136 134 41 33 136 41 137 135 134 136 137 134
             135 138 130 138 133 130 139 136 33 31 139 33
             140 137 136 139 140 136 137 141 135 141 138 135
             142 139 31 29 142 31 143 140 139 142 143 139
             140 144 137 144 141 137 29 145 142 29 27 145
             142 146 143 142 145 146 143 147 140 147 144 140
             143 146 148 143 148 147 145 149 146 146 150 148
             146 149 150 147 148 151 145 152 149 27 152 145
             147 153 144 147 151 153 27 154 152 27 26 154
             153 155 144 144 155 141 153 156 155 141 155 157
             141 157 138 156 158 155 158 157 155 153 159 156
             153 151 159 158 156 160 159 160 156 158 161 157
             161 158 160 138 157 162 138 162 133 161 163 157
             164 133 162 164 131 133 157 165 162 165 164 162
             157 163 165 164 166 131 127 131 166 161 167 163
             165 163 167 161 160 167 168 164 165 168 165 167
             164 168 166 160 169 167 127 166 170 127 170 129
             169 171 167 171 168 167 160 172 169 171 169 173
             169 172 173 174 172 160 172 174 173 159 174 160
             171 173 175 168 171 175 159 176 174 176 173 174
             159 177 176 151 177 159 176 178 173 151 179 177
             148 179 151 148 150 179 178 180 173 180 175 173
             176 181 178 180 178 181 177 182 176 176 182 181
             179 183 177 177 183 182 150 184 179 179 184 183
             150 185 184 149 185 150 184 186 183 149 187 185
             152 187 149 185 188 184 184 188 186 187 189 185
             185 189 188 152 190 187 154 190 152 187 191 189
             190 191 187 154 192 190 26 192 154 190 193 191
             192 193 190 191 194 189 26 195 192 193 196 191
             191 196 194 192 197 193 195 197 192 193 198 196
             197 198 193 199 195 26 22 199 26 195 200 197
             199 201 195 201 200 195 202 199 22 202 201 199
             200 203 197 197 203 198 201 204 200 200 205 203
             204 205 200 202 206 201 206 204 201 207 202 22
             207 22 18 207 18 16 202 208 206 202 207 208
             206 209 204 207 16 210 206 211 209 206 208 211
             207 212 208 207 210 212 208 213 211 208 212 213
             16 214 210 16 14 214 210 215 212 210 214 215
             14 216 214 14 17 216 214 217 215 214 216 217
             212 215 218 212 218 213 215 217 219 215 219 218
             216 220 217 17 221 216 216 221 220 17 82 221
             217 220 222 217 222 219 82 223 221 82 84 223
             221 224 220 221 223 224 84 225 223 84 88 225
             223 226 224 223 225 226 220 224 227 220 227 222
             224 226 228 224 228 227 225 229 226 226 230 228
             226 229 230 227 228 231 225 232 229 88 232 225
             222 227 233 227 231 233 222 233 234 219 222 234
             233 231 235 219 234 236 218 219 236 234 233 237
             233 235 237 236 234 238 234 237 238 218 236 239
             213 218 239 236 238 240 239 236 240 213 239 241
             211 213 241 239 240 242 241 239 242 211 241 243
             209 211 243 241 242 244 243 241 244 242 240 245
             209 243 246 242 245 247 244 242 247 240 248 245
             240 238 248 243 244 249 246 243 249 249 244 250
             244 247 250 246 249 251 247 245 252 252 245 248
             250 247 253 247 252 253 249 250 254 251 249 254
             250 253 255 254 250 255 253 252 256 251 254 257
             253 256 258 255 253 258 251 257 259 257 254 260
             259 257 260 254 255 261 261 260 254 251 259 262
             246 251 262 261 263 260 246 262 264 209 246 264
             209 264 204 204 264 205 264 262 265 264 265 205
             262 259 266 262 266 265 205 265 267 205 267 203
             265 266 268 265 268 267 266 259 269 260 269 259
             266 270 268 266 269 270 260 271 269 263 271 260
             269 271 272 263 272 271 270 269 273 263 274 272
             263 261 274 261 255 274 269 272 275 269 275 273
             274 276 272 276 275 272 274 277 276 275 276 278
             276 277 278 279 277 274 255 279 274 278 277 280
             279 280 277 255 281 279 255 258 281 279 281 282
             279 282 280 281 258 283 281 283 282 258 284 283
             258 256 284 282 283 285 282 285 280 283 284 286
             283 286 285 256 287 284 284 288 286 284 287 288
             285 286 289 256 290 287 252 290 256 285 289 291
             280 285 291 252 292 290 252 248 292 292 293 290
             248 294 292 292 294 293 248 295 294 238 295 248
             238 237 295 237 296 295 237 235 296 295 297 294
             295 296 297 235 298 296 297 296 298 294 297 299
             294 299 293 235 300 298 231 300 235 297 298 301
             231 302 300 228 302 231 228 230 302 300 303 298
             302 304 300 300 304 303 230 305 302 302 305 304
             298 303 306 298 306 301 304 307 303 303 307 306
             304 308 307 305 308 304 306 307 309 308 309 307
             230 310 305 229 310 230 305 311 308 310 311 305
             229 312 310 232 312 229 310 313 311 312 313 310
             308 311 314 313 314 311 308 314 315 308 315 309
             312 316 313 315 317 309 318 316 312 232 318 312
             315 319 317 314 319 315 318 232 320 313 321 314
             316 321 313 318 322 316 322 318 320 322 323 316
             316 323 321 322 324 323 322 320 324 321 323 325
             324 325 323 321 326 314 314 326 319 321 325 327
             321 327 326 326 328 319 327 329 326 326 329 328
             325 330 327 327 330 329 319 328 331 319 331 317
             329 332 328 328 333 331 328 332 333 317 331 334
             329 335 332 330 335 329 317 334 336 309 317 336
             331 337 334 331 333 337 309 336 338 306 309 338
             336 334 339 339 334 337 340 306 338 340 341 306
             341 301 306 340 338 342 342 338 336 343 341 340
             301 341 343 343 340 342 297 301 343 297 343 299
             342 336 344 336 339 344 345 299 343 346 343 342
             346 345 343 299 345 347 293 299 347 345 346 348
             345 348 347 346 342 349 346 349 348 350 349 342
             348 349 350 350 342 344 348 350 351 350 344 351
             344 352 351 344 339 352 353 348 351 353 351 352
             354 347 348 354 348 353 293 347 355 355 347 354
             290 293 355 290 355 287 287 355 356 355 354 356
             287 356 288 356 354 357 354 353 357 288 356 358
             356 357 358 357 353 359 353 352 359 358 357 360
             357 359 360 288 358 361 286 288 361 286 361 289
             361 358 362 358 360 362 289 361 363 361 362 363
             289 363 364 291 289 364 363 362 365 362 360 366
             362 366 365 360 367 366 360 359 367 363 365 368
             364 363 368 359 369 367 359 352 369 339 369 352
             339 370 369 339 337 370 367 369 371 370 371 369
             367 371 372 366 367 372 370 373 371 337 373 370
             372 371 374 373 374 371 366 372 375 365 366 375
             372 374 376 375 372 376 365 375 377 368 365 377
             375 376 378 377 375 378 376 374 379 368 377 380
             378 376 381 376 379 381 377 378 382 380 377 382
             378 381 383 382 378 383 384 368 380 364 368 384
             380 382 385 386 364 384 291 364 386 384 380 387
             387 380 385 388 291 386 280 291 388 386 384 389
             389 384 387 388 386 390 390 386 389 391 280 388
             391 388 390 391 278 280 275 278 391 392 275 391
             273 275 392 391 390 393 392 391 393 273 392 394
             392 393 394 393 390 395 390 389 395 394 393 396
             393 395 396 273 394 397 270 273 397 394 396 398
             397 394 398 270 397 399 268 270 399 397 398 400
             399 397 400 268 399 401 267 268 401 399 400 402
             401 399 402 267 401 403 203 267 403 203 403 198
             403 401 404 401 402 404 198 403 405 403 404 405
             198 405 196 404 402 406 196 405 407 405 404 408
             405 408 407 404 406 408 196 407 194 402 409 406
             402 400 409 408 406 410 400 411 409 400 398 411
             406 409 412 406 412 410 409 411 413 409 413 412
             398 414 411 398 396 414 411 415 413 411 414 415
             396 416 414 396 395 416 414 417 415 414 416 417
             395 418 416 395 389 418 389 387 418 416 418 419
             416 419 417 418 387 420 418 420 419 387 385 420
             417 419 421 419 420 422 419 422 421 420 385 423
             420 423 422 417 421 424 415 417 424 421 422 425
             425 422 423 424 421 426 421 425 426 415 424 427
             413 415 427 424 426 428 427 424 428 413 427 429
             412 413 429 412 429 430 410 412 430 431 429 427
             431 430 429 410 430 432 432 430 431 431 427 433
             427 428 433 434 431 433 435 433 428 435 434 433
             431 434 436 432 431 436 434 435 437 434 437 436
             435 428 438 435 439 437 439 435 438 428 440 438
             428 426 440 441 439 438 441 438 440 439 441 437
             426 442 440 426 425 442 441 440 443 443 440 442
             444 437 441 444 441 443 425 445 442 443 442 446
             446 442 445 425 447 445 425 423 447 448 443 446
             444 443 448 446 445 449 447 450 445 449 445 450
             423 451 447 447 451 450 452 446 449 448 446 452
             449 450 453 452 449 454 454 449 453 455 448 452
             455 452 454 453 450 456 451 456 450 454 453 457
             423 458 451 385 458 423 385 382 458 382 383 458
             458 459 451 458 383 459 451 459 456 459 383 381
             459 381 460 459 460 456 381 379 460 453 456 461
             457 453 461 462 461 456 460 462 456 457 461 463
             463 461 462 464 454 457 455 454 464 465 457 463
             464 457 465 466 455 464 467 464 465 466 464 467
             465 463 468 467 465 469 469 465 468 470 466 467
             471 467 469 470 467 471 469 468 472 471 469 473
             473 469 472 474 470 471 475 471 473 474 471 475
             473 472 476 475 473 477 477 473 476 472 468 478
             478 468 463 476 472 479 472 478 479 477 476 480
             476 479 481 480 476 481 479 478 482 481 479 483
             479 482 483 478 463 484 482 478 485 484 485 478
             483 482 486 486 482 485 484 463 487 463 462 487
             484 488 485 488 484 487 488 489 485 489 488 487
             462 490 487 460 490 462 489 487 491 490 491 487
             492 490 460 490 492 491 493 492 460 492 493 491
             493 460 379 494 489 491 489 494 485 493 495 491
             495 493 379 496 494 491 496 491 495 497 485 494
             496 497 494 497 486 485 498 495 379 498 379 374
             373 498 374 499 495 498 499 496 495 500 498 373
             500 499 498 337 500 373 501 496 499 500 501 499
             337 502 500 500 502 501 337 333 502 332 502 333
             332 503 502 503 501 502 335 503 332 501 504 496
             496 504 497 503 505 501 505 504 501 335 506 503
             506 505 503 507 497 504 505 507 504 508 506 335
             330 508 335 506 509 505 509 507 505 508 510 506
             510 509 506 511 508 330 325 511 330 512 510 508
             511 512 508 513 511 325 324 513 325 102 512 511
             513 102 511 324 99 513 99 102 513 324 514 99
             514 98 99 320 514 324 514 515 98 320 515 514
             515 516 98 91 98 516 87 91 516 88 87 516
             517 515 320 515 517 516 518 517 320 517 518 516
             518 88 516 518 320 232 88 518 232 102 519 512
             102 101 519 519 520 512 512 520 510 101 521 519
             519 521 520 101 105 521 520 522 510 510 522 509
             521 523 520 520 523 522 105 524 521 521 524 523
             105 107 524 522 525 509 509 525 507 523 526 522
             522 526 525 524 527 523 523 527 526 107 528 524
             524 528 527 107 108 528 526 529 525 507 525 529
             527 530 526 526 530 529 528 531 527 527 531 530
             507 529 532 507 532 497 532 486 497 532 533 486
             529 533 532 483 486 533 529 534 533 530 534 529
             535 483 533 535 533 534 481 483 535 530 536 534
             531 536 530 537 535 534 537 534 536 538 481 535
             538 535 537 480 481 538 539 537 536 539 538 537
             531 540 536 539 536 540 541 480 538 539 542 538
             541 538 542 543 480 541 543 477 480 544 541 542
             545 477 543 545 475 477 546 543 541 546 541 544
             547 545 543 547 543 546 548 475 545 548 474 475
             549 545 547 549 548 545 129 547 546 129 546 126
             126 546 544 170 547 129 170 549 547 126 544 122
             166 549 170 122 544 550 544 542 550 550 542 539
             166 551 549 551 548 549 168 551 166 551 552 548
             552 474 548 168 553 551 553 552 551 168 175 553
             552 554 474 554 470 474 553 555 552 555 554 552
             175 555 553 554 556 470 556 466 470 555 557 554
             557 556 554 175 558 555 558 557 555 180 558 175
             556 559 466 559 455 466 180 560 558 180 181 560
             559 561 455 455 561 448 562 559 556 557 562 556
             563 561 559 562 563 559 564 448 561 563 564 561
             564 444 448 565 562 557 558 565 557 560 565 558
             566 563 562 565 566 562 567 564 563 566 567 563
             560 568 565 568 566 565 560 181 569 560 569 568
             182 569 181 568 570 566 570 567 566 568 569 571
             568 571 570 182 572 569 572 571 569 183 572 182
             183 186 572 572 573 571 186 573 572 570 571 574
             573 574 571 186 575 573 188 575 186 573 576 574
             575 576 573 570 574 577 570 577 567 188 578 575
             189 578 188 189 194 578 578 579 575 575 579 576
             194 580 578 578 580 579 194 407 580 580 581 579
             407 582 580 580 582 581 407 408 582 408 410 582
             582 410 432 582 432 581 579 581 583 583 581 432
             579 583 576 583 432 584 584 432 436 576 583 585
             576 585 574 577 574 585 584 586 583 585 583 586
             587 584 436 584 587 586 588 587 436 587 588 586
             588 436 437 589 588 437 588 589 586 444 589 437
             590 585 586 577 585 590 591 586 589 591 590 586
             591 589 444 577 590 591 591 444 564 577 591 567
             567 591 564 528 592 531 108 592 528 592 540 531
             108 593 592 593 108 113 592 594 540 595 593 113
             595 113 114 593 596 592 593 595 596 594 592 596
             595 114 597 595 597 596 598 597 114 597 598 596
             122 598 114 122 550 598 550 596 598 599 594 596
             550 599 596 599 540 594 550 539 599 599 539 540
          }
          indexCount 3588
          instanceCount 1
          firstIndex 0
          vertexOffset 0
          firstInstance 0
        }
        NumStateCommands 1
        StateCommand id=63 vsg::BindGraphicsPipeline
        {
          NumUserObjects 0
          Slot 0
          GraphicsPipeline id=64 vsg::GraphicsPipeline
          {
            NumUserObjects 0
            PipelineLayout id=65 vsg::PipelineLayout
            {
              NumUserObjects 0
              Flags 0
              NumDescriptorSetLayouts 1
              DescriptorSetLayout id=66 vsg::DescriptorSetLayout
              {
                NumUserObjects 0
                NumDescriptorSetLayoutBindings 0
              }
              NumPushConstantRanges 1
              stageFlags 1
              offset 0
              size 128
            }
            NumShaderStages 2
            ShaderStage id=67 vsg::ShaderStage
            {
              NumUserObjects 0
              Stage 1
              EntryPoint "main"
              ShaderModule id=68 vsg::ShaderModule
              {
                NumUserObjects 0
                Source "#version 450
#pragma import_defines ( VSG_NORMAL, VSG_TANGENT, VSG_COLOR, VSG_TEXCOORD0, VSG_LIGHTING, VSG_NORMAL_MAP, VSG_BILLBOARD, VSG_TRANSLATE )
#define VSG_NORMAL
#extension GL_ARB_separate_shader_objects : enable
layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
    //mat3 normal;
} pc;
layout(location = 0) in vec3 osg_Vertex;
#ifdef VSG_NORMAL
layout(location = 1) in vec3 osg_Normal;
layout(location = 1) out vec3 normalDir;
#endif
#ifdef VSG_TANGENT
layout(location = 2) in vec4 osg_Tangent;
#endif
#ifdef VSG_COLOR
layout(location = 3) in vec4 osg_Color;
layout(location = 3) out vec4 vertColor;
#endif
#ifdef VSG_TEXCOORD0
layout(location = 4) in vec2 osg_MultiTexCoord0;
layout(location = 4) out vec2 texCoord0;
#endif
#ifdef VSG_LIGHTING
layout(location = 5) out vec3 viewDir;
layout(location = 6) out vec3 lightDir;
#endif
#ifdef VSG_TRANSLATE
layout(location = 7) in vec3 translate;
#endif


out gl_PerVertex{ vec4 gl_Position; };

void main()
{
    mat4 modelView = pc.modelView;

#ifdef VSG_TRANSLATE
    mat4 translate_mat = mat4(1.0, 0.0, 0.0, 0.0,
                              0.0, 1.0, 0.0, 0.0,
                              0.0, 0.0, 1.0, 0.0,
                              translate.x,  translate.y,  translate.z, 1.0);

    modelView = modelView * translate_mat;
#endif

#ifdef VSG_BILLBOARD
    vec3 lookDir = vec3(-modelView[0][2], -modelView[1][2], -modelView[2][2]);

    // rotate around local z axis
    float l = length(lookDir.xy);
    if (l>0.0)
    {
        float inv = 1.0/l;
        float c = lookDir.y * inv;
        float s = lookDir.x * inv;

        mat4 rotation_z = mat4(c,   -s,  0.0, 0.0,
                               s,   c,   0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0);

        modelView = modelView * rotation_z;
    }
#endif

    gl_Position = (pc.projection * modelView) * vec4(osg_Vertex, 1.0);

#ifdef VSG_TEXCOORD0
    texCoord0 = osg_MultiTexCoord0.st;
#endif
#ifdef VSG_NORMAL
    vec3 n = (modelView * vec4(osg_Normal, 0.0)).xyz;
    normalDir = n;
#endif
#ifdef VSG_LIGHTING
    vec4 lpos = /*osg_LightSource.position*/ vec4(0.0, 0.25, 1.0, 0.0);
#ifdef VSG_NORMAL_MAP
    vec3 t = (modelView * vec4(osg_Tangent.xyz, 0.0)).xyz;
    vec3 b = cross(n, t);
    vec3 dir = -vec3(modelView * vec4(osg_Vertex, 1.0));
    viewDir.x = dot(dir, t);
    viewDir.y = dot(dir, b);
    viewDir.z = dot(dir, n);
    if (lpos.w == 0.0)
        dir = lpos.xyz;
    else
        dir += lpos.xyz;
    lightDir.x = dot(dir, t);
    lightDir.y = dot(dir, b);
    lightDir.z = dot(dir, n);
#else
    viewDir = -vec3(modelView * vec4(osg_Vertex, 1.0));
    if (lpos.w == 0.0)
        lightDir = lpos.xyz;
    else
        lightDir = lpos.xyz + viewDir;
#endif
#endif
#ifdef VSG_COLOR
    vertColor = osg_Color;
#endif
}

"
                SPIRVSize 390
                SPIRV 119734787 66560 524298 54 0 131089 1 393227 1 1280527431 1685353262 808793134
                 0 196622 0 1 655375 0 4 1852399981 0 13 21 29
                 42 52 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331 1868526181
                 1667590754 29556 262149 4 1852399981 0 327685 10 1701080941 1701402220 119 393221
                 11 1752397136 1936617283 1953390964 115 393222 11 0 1785688688 1769235301 28271 393222
                 11 1 1701080941 1701402220 119 196613 13 25456 393221 19 1348430951 1700164197
                 2019914866 0 393222 19 0 1348430951 1953067887 7237481 196613 21 0 327685
                 29 1600615279 1953654102 30821 196613 40 110 327685 42 1600615279 1836216142 27745
                 327685 52 1836216174 1766091873 114 262216 11 0 5 327752 11 0
                 35 0 327752 11 0 7 16 262216 11 1 5 327752
                 11 1 35 64 327752 11 1 7 16 196679 11 2
                 327752 19 0 11 0 196679 19 2 262215 29 30 0
                 262215 42 30 1 262215 52 30 1 131091 2 196641 3
                 2 196630 6 32 262167 7 6 4 262168 8 7 4
                 262176 9 7 8 262174 11 8 8 262176 12 9 11
                 262203 12 13 9 262165 14 32 1 262187 14 15 1
                 262176 16 9 8 196638 19 7 262176 20 3 19 262203
                 20 21 3 262187 14 22 0 262167 27 6 3 262176
                 28 1 27 262203 28 29 1 262187 6 31 1065353216 262176
                 37 3 7 262176 39 7 27 262203 28 42 1 262187
                 6 44 0 262176 51 3 27 262203 51 52 3 327734
                 2 4 0 3 131320 5 262203 9 10 7 262203 39
                 40 7 327745 16 17 13 15 262205 8 18 17 196670
                 10 18 327745 16 23 13 22 262205 8 24 23 262205
                 8 25 10 327826 8 26 24 25 262205 27 30 29
                 327761 6 32 30 0 327761 6 33 30 1 327761 6
                 34 30 2 458832 7 35 32 33 34 31 327825 7
                 36 26 35 327745 37 38 21 22 196670 38 36 262205
                 8 41 10 262205 27 43 42 327761 6 45 43 0
                 327761 6 46 43 1 327761 6 47 43 2 458832 7
                 48 45 46 47 44 327825 7 49 41 48 524367 27
                 50 49 49 0 1 2 196670 40 50 262205 27 53
                 40 196670 52 53 65789 65592
              }
              NumSpecializationConstants 0
            }
            ShaderStage id=69 vsg::ShaderStage
            {
              NumUserObjects 0
              Stage 16
              EntryPoint "main"
              ShaderModule id=70 vsg::ShaderModule
              {
                NumUserObjects 0
                Source "#version 450
#pragma import_defines ( VSG_NORMAL, VSG_COLOR, VSG_TEXCOORD0, VSG_LIGHTING, VSG_MATERIAL, VSG_DIFFUSE_MAP, VSG_OPACITY_MAP, VSG_AMBIENT_MAP, VSG_NORMAL_MAP, VSG_SPECULAR_MAP )
#define VSG_NORMAL
#extension GL_ARB_separate_shader_objects : enable
#ifdef VSG_DIFFUSE_MAP
layout(binding = 0) uniform sampler2D diffuseMap;
#endif
#ifdef VSG_OPACITY_MAP
layout(binding = 1) uniform sampler2D opacityMap;
#endif
#ifdef VSG_AMBIENT_MAP
layout(binding = 4) uniform sampler2D ambientMap;
#endif
#ifdef VSG_NORMAL_MAP
layout(binding = 5) uniform sampler2D normalMap;
#endif
#ifdef VSG_SPECULAR_MAP
layout(binding = 6) uniform sampler2D specularMap;
#endif

#ifdef VSG_MATERIAL
layout(binding = 10) uniform MaterialData
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    float shininess;
} material;
#endif

#ifdef VSG_NORMAL
layout(location = 1) in vec3 normalDir;
#endif
#ifdef VSG_COLOR
layout(location = 3) in vec4 vertColor;
#endif
#ifdef VSG_TEXCOORD0
layout(location = 4) in vec2 texCoord0;
#endif
#ifdef VSG_LIGHTING
layout(location = 5) in vec3 viewDir;
layout(location = 6) in vec3 lightDir;
#endif
layout(location = 0) out vec4 outColor;

void main()
{
#ifdef VSG_DIFFUSE_MAP
    vec4 base = texture(diffuseMap, texCoord0.st);
#else
    vec4 base = vec4(1.0,1.0,1.0,1.0);
#endif
#ifdef VSG_COLOR
    base = base * vertColor;
#endif
#ifdef VSG_MATERIAL
    vec3 ambientColor = material.ambientColor.rgb;
    vec3 diffuseColor = material.diffuseColor.rgb;
    vec3 specularColor = material.specularColor.rgb;
    float shininess = material.shininess;
#else
    vec3 ambientColor = vec3(0.1,0.1,0.1);
    vec3 diffuseColor = vec3(1.0,1.0,1.0);
    vec3 specularColor = vec3(0.3,0.3,0.3);
    float shininess = 16.0;
#endif
#ifdef VSG_AMBIENT_MAP
    ambientColor *= texture(ambientMap, texCoord0.st).r;
#endif
#ifdef VSG_SPECULAR_MAP
    specularColor = texture(specularMap, texCoord0.st).rrr;
#endif
#ifdef VSG_LIGHTING
#ifdef VSG_NORMAL_MAP
    vec3 nDir = texture(normalMap, texCoord0.st).xyz*2.0 - 1.0;
    nDir.g = -nDir.g;
#else
    vec3 nDir = normalDir;
#endif
    vec3 nd = normalize(nDir);
    vec3 ld = normalize(lightDir);
    vec3 vd = normalize(viewDir);
    vec4 color = vec4(0.01, 0.01, 0.01, 1.0);
    color.rgb += ambientColor;
    float diff = max(dot(ld, nd), 0.0);
    color.rgb += diffuseColor * diff;
    color *= base;
    if (diff > 0.0)
    {
        vec3 halfDir = normalize(ld + vd);
        color.rgb += base.a * specularColor *
            pow(max(dot(halfDir, nd), 0.0), shininess);
    }
#else
    vec4 color = base;
    color.rgb *= diffuseColor;
#endif
    outColor = color;
#ifdef VSG_OPACITY_MAP
    outColor.a *= texture(opacityMap, texCoord0.st).r;
#endif
    if (outColor.a==0.0) discard;
}

"
                SPIRVSize 319
                SPIRV 119734787 66560 524298 49 0 131089 1 393227 1 1280527431 1685353262 808793134
                 0 196622 0 1 458767 4 4 1852399981 0 34 48 196624
                 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331 1868526181
                 1667590754 29556 262149 4 1852399981 0 262149 9 1702060386 0 393221 14
                 1768058209 1131703909 1919904879 0 393221 17 1717987684 1130722165 1919904879 0 393221 19
                 1667592307 1918987381 1869377347 114 327685 23 1852401779 1936027241 115 262149 25 1869377379
                 114 327685 34 1131705711 1919904879 0 327685 48 1836216174 1766091873 114 262215
                 34 30 0 262215 48 30 1 131091 2 196641 3 2
                 196630 6 32 262167 7 6 4 262176 8 7 7 262187
                 6 10 1065353216 458796 7 11 10 10 10 10 262167 12
                 6 3 262176 13 7 12 262187 6 15 1036831949 393260 12
                 16 15 15 15 393260 12 18 10 10 10 262187 6
                 20 1050253722 393260 12 21 20 20 20 262176 22 7 6
                 262187 6 24 1098907648 262176 33 3 7 262203 33 34 3
                 262165 36 32 0 262187 36 37 3 262176 38 3 6
                 262187 6 41 0 131092 42 262176 47 1 12 262203 47
                 48 1 327734 2 4 0 3 131320 5 262203 8 9
                 7 262203 13 14 7 262203 13 17 7 262203 13 19
                 7 262203 22 23 7 262203 8 25 7 196670 9 11
                 196670 14 16 196670 17 18 196670 19 21 196670 23 24
                 262205 7 26 9 196670 25 26 262205 12 27 17 262205
                 7 28 25 524367 12 29 28 28 0 1 2 327813
                 12 30 29 27 262205 7 31 25 589903 7 32 31
                 30 4 5 6 3 196670 25 32 262205 7 35 25
                 196670 34 35 327745 38 39 34 37 262205 6 40 39
                 327860 42 43 40 41 196855 45 0 262394 43 44 45
                 131320 44 65788 131320 45 65789 65592
              }
              NumSpecializationConstants 0
            }
            NumPipelineStates 6
            PipelineState id=71 vsg::VertexInputState
            {
              NumUserObjects 0
              NumBindings 2
              binding 0
              stride 12
              inputRate 0
              binding 1
              stride 12
              inputRate 0
              NumAttributes 2
              location 0
              binding 0
              format 106
              offset 0
              location 1
              binding 1
              format 106
              offset 0
            }
            PipelineState id=72 vsg::InputAssemblyState
            {
              NumUserObjects 0
              topology 3
              primitiveRestartEnable 0
            }
            PipelineState id=73 vsg::RasterizationState
            {
              NumUserObjects 0
              depthClampEnable 0
              rasterizerDiscardEnable 0
              polygonMode 0
              cullMode 2
              frontFace 0
              depthBiasEnable 0
              depthBiasConstantFactor 1
              depthBiasClamp 0
              depthBiasSlopeFactor 1
              lineWidth 1
            }
            PipelineState id=74 vsg::MultisampleState
            {
              NumUserObjects 0
              rasterizationSamples 1
              sampleShadingEnable 0
              minSampleShading 0
              NumSampleMask 0
              alphaToCoverageEnable 0
              alphaToOneEnable 0
            }
            PipelineState id=75 vsg::ColorBlendState
            {
              NumUserObjects 0
              logicOp 3
              logicOpEnable 0
              NumColorBlendAttachments 1
              blendEnable 0
              srcColorBlendFactor 0
              dstColorBlendFactor 0
              colorBlendOp 0
              srcAlphaBlendFactor 0
              dstAlphaBlendFactor 0
              alphaBlendOp 0
              colorWriteMask 15
              blendConstants 0 0 0 0
            }
            PipelineState id=76 vsg::DepthStencilState
            {
              NumUserObjects 0
              depthTestEnable 1
              depthWriteEnable 1
              depthCompareOp 1
              depthBoundsTestEnable 0
              stencilTestEnable 0
              front.failOp 0
              front.passOp 0
              front.depthFailOp 0
              front.compareOp 0
              front.compareMask 0
              front.writeMask 0
              front.reference 0
              back.failOp 0
              back.passOp 0
              back.depthFailOp 0
              back.compareOp 0
              back.compareMask 0
              back.writeMask 0
              back.reference 0
              minDepthBounds 0
              maxDepthBounds 1
            }
            subpass 0
          }
        }
      }
    }
  }
}
)");
vsg::VSG io;
return io.read_cast<vsg::Group>(str);
};
