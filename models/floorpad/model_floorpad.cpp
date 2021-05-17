#include <vsg/io/ReaderWriter_vsg.h>
static auto model_floorpad = []() {std::istringstream str(
R"(#vsga 0.0.3
Root id=1 vsg::Group
{
  NumUserObjects 0
  NumChildren 6
  Child id=2 vsg::MatrixTransform
  {
    NumUserObjects 0
    NumChildren 1
    Child id=3 vsg::Group
    {
      NumUserObjects 0
      NumChildren 1
      Child id=4 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=5 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 2
          Array id=6 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data -1 1 1 1 -1 1 1 1 1 -1 -1 1
             1 -1 1 -1 -1 -1 1 -1 -1 -1 -1 1
             -1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1
             1 1 -1 -1 -1 -1 -1 1 -1 1 -1 -1
             1 1 1 1 -1 -1 1 1 -1 1 -1 1
             -1 1 1 1 1 -1 -1 1 -1 1 1 1
          }
          Array id=7 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data 0 0 1 0 0 1 0 0 1 0 0 1
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 0 0 -1 0 0 -1 0 0 -1 0 0
             0 0 -1 0 0 -1 0 0 -1 0 0 -1
             1 0 0 1 0 0 1 0 0 1 0 0
             0 1 0 0 1 0 0 1 0 0 1 0
          }
          Indices id=8 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0
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
        StateCommand id=9 vsg::BindGraphicsPipeline
        {
          NumUserObjects 0
          Slot 0
          GraphicsPipeline id=10 vsg::GraphicsPipeline
          {
            NumUserObjects 0
            PipelineLayout id=11 vsg::PipelineLayout
            {
              NumUserObjects 0
              Flags 0
              NumDescriptorSetLayouts 1
              DescriptorSetLayout id=12 vsg::DescriptorSetLayout
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
            ShaderStage id=13 vsg::ShaderStage
            {
              NumUserObjects 0
              Stage 1
              EntryPoint "main"
              ShaderModule id=14 vsg::ShaderModule
              {
                NumUserObjects 0
                Source "#version 450
#pragma import_defines ( VSG_NORMAL, VSG_TANGENT, VSG_COLOR, VSG_TEXCOORD0, VSG_LIGHTING, VSG_NORMAL_MAP, VSG_BILLBOARD, VSG_TRANSLATE )
#define VSG_NORMAL
#define VSG_LIGHTING
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
                SPIRVSize 589
                SPIRV 119734787 65536 524298 88 0 131089 1 393227 1 1280527431 1685353262 808793134
                 0 196622 0 1 720911 0 4 1852399981 0 21 29 42
                 52 58 80 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331
                 1868526181 1667590754 29556 262149 4 1852399981 0 327685 10 1701080941 1701402220 119
                 393221 11 1752397136 1936617283 1953390964 115 393222 11 0 1785688688 1769235301 28271
                 393222 11 1 1701080941 1701402220 119 196613 13 25456 393221 19 1348430951
                 1700164197 2019914866 0 393222 19 0 1348430951 1953067887 7237481 196613 21 0
                 327685 29 1600615279 1953654102 30821 196613 40 110 327685 42 1600615279 1836216142
                 27745 327685 52 1836216174 1766091873 114 262149 55 1936683116 0 262149 58
                 2003134838 7498052 327685 80 1751607660 1919501428 0 262216 11 0 5 327752
                 11 0 35 0 327752 11 0 7 16 262216 11 1
                 5 327752 11 1 35 64 327752 11 1 7 16 196679
                 11 2 327752 19 0 11 0 196679 19 2 262215 29
                 30 0 262215 42 30 1 262215 52 30 1 262215 58
                 30 5 262215 80 30 6 131091 2 196641 3 2 196630
                 6 32 262167 7 6 4 262168 8 7 4 262176 9
                 7 8 262174 11 8 8 262176 12 9 11 262203 12
                 13 9 262165 14 32 1 262187 14 15 1 262176 16
                 9 8 196638 19 7 262176 20 3 19 262203 20 21
                 3 262187 14 22 0 262167 27 6 3 262176 28 1
                 27 262203 28 29 1 262187 6 31 1065353216 262176 37 3
                 7 262176 39 7 27 262203 28 42 1 262187 6 44
                 0 262176 51 3 27 262203 51 52 3 262176 54 7
                 7 262187 6 56 1048576000 458796 7 57 44 56 31 44
                 262203 51 58 3 262165 71 32 0 262187 71 72 3
                 262176 73 7 6 131092 76 262203 51 80 3 327734 2
                 4 0 3 131320 5 262203 9 10 7 262203 39 40
                 7 262203 54 55 7 327745 16 17 13 15 262205 8
                 18 17 196670 10 18 327745 16 23 13 22 262205 8
                 24 23 262205 8 25 10 327826 8 26 24 25 262205
                 27 30 29 327761 6 32 30 0 327761 6 33 30
                 1 327761 6 34 30 2 458832 7 35 32 33 34
                 31 327825 7 36 26 35 327745 37 38 21 22 196670
                 38 36 262205 8 41 10 262205 27 43 42 327761 6
                 45 43 0 327761 6 46 43 1 327761 6 47 43
                 2 458832 7 48 45 46 47 44 327825 7 49 41
                 48 524367 27 50 49 49 0 1 2 196670 40 50
                 262205 27 53 40 196670 52 53 196670 55 57 262205 8
                 59 10 262205 27 60 29 327761 6 61 60 0 327761
                 6 62 60 1 327761 6 63 60 2 458832 7 64
                 61 62 63 31 327825 7 65 59 64 327761 6 66
                 65 0 327761 6 67 65 1 327761 6 68 65 2
                 393296 27 69 66 67 68 262271 27 70 69 196670 58
                 70 327745 73 74 55 72 262205 6 75 74 327860 76
                 77 75 44 196855 79 0 262394 77 78 83 131320 78
                 262205 7 81 55 524367 27 82 81 81 0 1 2
                 196670 80 82 131321 79 131320 83 262205 7 84 55 524367
                 27 85 84 84 0 1 2 262205 27 86 58 327809
                 27 87 85 86 196670 80 87 131321 79 131320 79 65789
                 65592
              }
              NumSpecializationConstants 0
            }
            ShaderStage id=15 vsg::ShaderStage
            {
              NumUserObjects 0
              Stage 16
              EntryPoint "main"
              ShaderModule id=16 vsg::ShaderModule
              {
                NumUserObjects 0
                Source "#version 450
#pragma import_defines ( VSG_NORMAL, VSG_COLOR, VSG_TEXCOORD0, VSG_LIGHTING, VSG_MATERIAL, VSG_DIFFUSE_MAP, VSG_OPACITY_MAP, VSG_AMBIENT_MAP, VSG_NORMAL_MAP, VSG_SPECULAR_MAP )
#define VSG_NORMAL
#define VSG_LIGHTING
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
                SPIRVSize 660
                SPIRV 119734787 65536 524298 104 0 131089 1 393227 1 1280527431 1685353262 808793134
                 0 196622 0 1 589839 4 4 1852399981 0 27 33 37
                 95 196624 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449
                 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 262149 9 1702060386 0
                 393221 14 1768058209 1131703909 1919904879 0 393221 17 1717987684 1130722165 1919904879 0
                 393221 19 1667592307 1918987381 1869377347 114 327685 23 1852401779 1936027241 115 262149
                 25 1919501422 0 327685 27 1836216174 1766091873 114 196613 29 25710 196613
                 32 25708 327685 33 1751607660 1919501428 0 196613 36 25718 262149 37
                 2003134838 7498052 262149 40 1869377379 114 262149 49 1717987684 0 262149 71
                 1718378856 7498052 327685 95 1131705711 1919904879 0 262215 27 30 1 262215
                 33 30 6 262215 37 30 5 262215 95 30 0 131091
                 2 196641 3 2 196630 6 32 262167 7 6 4 262176
                 8 7 7 262187 6 10 1065353216 458796 7 11 10 10
                 10 10 262167 12 6 3 262176 13 7 12 262187 6
                 15 1036831949 393260 12 16 15 15 15 393260 12 18 10
                 10 10 262187 6 20 1050253722 393260 12 21 20 20 20
                 262176 22 7 6 262187 6 24 1098907648 262176 26 1 12
                 262203 26 27 1 262203 26 33 1 262203 26 37 1
                 262187 6 41 1008981770 458796 7 42 41 41 41 10 262187
                 6 53 0 131092 67 262165 76 32 0 262187 76 77
                 3 262176 94 3 7 262203 94 95 3 262176 97 3
                 6 327734 2 4 0 3 131320 5 262203 8 9 7
                 262203 13 14 7 262203 13 17 7 262203 13 19 7
                 262203 22 23 7 262203 13 25 7 262203 13 29 7
                 262203 13 32 7 262203 13 36 7 262203 8 40 7
                 262203 22 49 7 262203 13 71 7 196670 9 11 196670
                 14 16 196670 17 18 196670 19 21 196670 23 24 262205
                 12 28 27 196670 25 28 262205 12 30 25 393228 12
                 31 1 69 30 196670 29 31 262205 12 34 33 393228
                 12 35 1 69 34 196670 32 35 262205 12 38 37
                 393228 12 39 1 69 38 196670 36 39 196670 40 42
                 262205 12 43 14 262205 7 44 40 524367 12 45 44
                 44 0 1 2 327809 12 46 45 43 262205 7 47
                 40 589903 7 48 47 46 4 5 6 3 196670 40
                 48 262205 12 50 32 262205 12 51 29 327828 6 52
                 50 51 458764 6 54 1 40 52 53 196670 49 54
                 262205 12 55 17 262205 6 56 49 327822 12 57 55
                 56 262205 7 58 40 524367 12 59 58 58 0 1
                 2 327809 12 60 59 57 262205 7 61 40 589903 7
                 62 61 60 4 5 6 3 196670 40 62 262205 7
                 63 9 262205 7 64 40 327813 7 65 64 63 196670
                 40 65 262205 6 66 49 327866 67 68 66 53 196855
                 70 0 262394 68 69 70 131320 69 262205 12 72 32
                 262205 12 73 36 327809 12 74 72 73 393228 12 75
                 1 69 74 196670 71 75 327745 22 78 9 77 262205
                 6 79 78 262205 12 80 19 327822 12 81 80 79
                 262205 12 82 71 262205 12 83 29 327828 6 84 82
                 83 458764 6 85 1 40 84 53 262205 6 86 23
                 458764 6 87 1 26 85 86 327822 12 88 81 87
                 262205 7 89 40 524367 12 90 89 89 0 1 2
                 327809 12 91 90 88 262205 7 92 40 589903 7 93
                 92 91 4 5 6 3 196670 40 93 131321 70 131320
                 70 262205 7 96 40 196670 95 96 327745 97 98 95
                 77 262205 6 99 98 327860 67 100 99 53 196855 102
                 0 262394 100 101 102 131320 101 65788 131320 102 65789 65592
              }
              NumSpecializationConstants 0
            }
            NumPipelineStates 6
            PipelineState id=17 vsg::VertexInputState
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
            PipelineState id=18 vsg::InputAssemblyState
            {
              NumUserObjects 0
              topology 3
              primitiveRestartEnable 0
            }
            PipelineState id=19 vsg::RasterizationState
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
            PipelineState id=20 vsg::MultisampleState
            {
              NumUserObjects 0
              rasterizationSamples 1
              sampleShadingEnable 0
              minSampleShading 0
              NumSampleMask 0
              alphaToCoverageEnable 0
              alphaToOneEnable 0
            }
            PipelineState id=21 vsg::ColorBlendState
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
            PipelineState id=22 vsg::DepthStencilState
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
    Matrix -0.1 0 0 0 0 0 0.1 0 0 1.5 0 0
     2 1.5 1 1
    SubgraphRequiresLocalFrustum 0
  }
  Child id=23 vsg::MatrixTransform
  {
    NumUserObjects 0
    NumChildren 1
    Child id=24 vsg::Group
    {
      NumUserObjects 0
      NumChildren 1
      Child id=25 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=26 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 2
          Array id=27 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data -1 1 1 1 -1 1 1 1 1 -1 -1 1
             1 -1 1 -1 -1 -1 1 -1 -1 -1 -1 1
             -1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1
             1 1 -1 -1 -1 -1 -1 1 -1 1 -1 -1
             1 1 1 1 -1 -1 1 1 -1 1 -1 1
             -1 1 1 1 1 -1 -1 1 -1 1 1 1
          }
          Array id=28 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data 0 0 1 0 0 1 0 0 1 0 0 1
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 0 0 -1 0 0 -1 0 0 -1 0 0
             0 0 -1 0 0 -1 0 0 -1 0 0 -1
             1 0 0 1 0 0 1 0 0 1 0 0
             0 1 0 0 1 0 0 1 0 0 1 0
          }
          Indices id=29 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0
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
        StateCommand id=9
      }
    }
    Matrix -0.1 0 0 0 0 0 0.1 0 0 1 0 0
     -2 1 -1 1
    SubgraphRequiresLocalFrustum 0
  }
  Child id=30 vsg::MatrixTransform
  {
    NumUserObjects 0
    NumChildren 1
    Child id=31 vsg::Group
    {
      NumUserObjects 0
      NumChildren 1
      Child id=32 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=33 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 2
          Array id=34 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data -1 1 1 1 -1 1 1 1 1 -1 -1 1
             1 -1 1 -1 -1 -1 1 -1 -1 -1 -1 1
             -1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1
             1 1 -1 -1 -1 -1 -1 1 -1 1 -1 -1
             1 1 1 1 -1 -1 1 1 -1 1 -1 1
             -1 1 1 1 1 -1 -1 1 -1 1 1 1
          }
          Array id=35 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data 0 0 1 0 0 1 0 0 1 0 0 1
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 0 0 -1 0 0 -1 0 0 -1 0 0
             0 0 -1 0 0 -1 0 0 -1 0 0 -1
             1 0 0 1 0 0 1 0 0 1 0 0
             0 1 0 0 1 0 0 1 0 0 1 0
          }
          Indices id=36 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0
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
        StateCommand id=9
      }
    }
    Matrix -0.1 0 0 0 0 0 0.1 0 0 2 0 0
     2 2 -1 1
    SubgraphRequiresLocalFrustum 0
  }
  Child id=37 vsg::MatrixTransform
  {
    NumUserObjects 0
    NumChildren 1
    Child id=38 vsg::Group
    {
      NumUserObjects 0
      NumChildren 0
    }
    Matrix -0.685921 0 0.727676 0 0.324014 0.895396 0.305421 0 -0.651558 0.445271 -0.61417 0
     -7.35889 4.95831 -6.92579 1
    SubgraphRequiresLocalFrustum 0
  }
  Child id=39 vsg::MatrixTransform
  {
    NumUserObjects 0
    NumChildren 1
    Child id=40 vsg::Group
    {
      NumUserObjects 0
      NumChildren 0
    }
    Matrix 0.290865 -0.0551891 0.955171 0 0.771101 0.604525 -0.199883 0 -0.566393 0.794672 0.218391 0
     -4.07625 5.90386 1.00545 1
    SubgraphRequiresLocalFrustum 0
  }
  Child id=41 vsg::MatrixTransform
  {
    NumUserObjects 0
    NumChildren 1
    Child id=42 vsg::Group
    {
      NumUserObjects 0
      NumChildren 1
      Child id=43 vsg::StateGroup
      {
        NumUserObjects 0
        NumChildren 1
        Child id=44 vsg::VertexIndexDraw
        {
          NumUserObjects 0
          firstBinding 0
          NumArrays 2
          Array id=45 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data -1 1 1 1 -1 1 1 1 1 -1 -1 1
             1 -1 1 -1 -1 -1 1 -1 -1 -1 -1 1
             -1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1
             1 1 -1 -1 -1 -1 -1 1 -1 1 -1 -1
             1 1 1 1 -1 -1 1 1 -1 1 -1 1
             -1 1 1 1 1 -1 -1 1 -1 1 1 1
          }
          Array id=46 vsg::vec3Array
          {
            NumUserObjects 0
            Layout 0 12 0 1 1 1 0
            Size 24
            Storage id=0
            Data 0 0 1 0 0 1 0 0 1 0 0 1
             0 -1 0 0 -1 0 0 -1 0 0 -1 0
             -1 0 0 -1 0 0 -1 0 0 -1 0 0
             0 0 -1 0 0 -1 0 0 -1 0 0 -1
             1 0 0 1 0 0 1 0 0 1 0 0
             0 1 0 0 1 0 0 1 0 0 1 0
          }
          Indices id=47 vsg::ushortArray
          {
            NumUserObjects 0
            Layout 0 2 0 1 1 1 0
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
        StateCommand id=9
      }
    }
    Matrix -0.1 0 0 0 0 0 0.1 0 0 2 0 0
     -2 2 1 1
    SubgraphRequiresLocalFrustum 0
  }
}
)");
vsg::ReaderWriter_vsg io;
return io.read_cast<vsg::Group>(str);
};
