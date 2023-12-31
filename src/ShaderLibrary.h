#pragma once

#include <Shader.h>
#include <LightingShaderRoutine.h>

class ShaderLibrary
{
public:
    enum class STAGE
    {
        DEPTH,
        EDITOR_ID,
        ALBEDO,
        SHADOWS,
        BOKE,
        BLOOM,
        DEFERRED,
        DEFERRED_LIGHT,
        PARTICLES,
    };

public:
    ShaderLibrary();

    Shader& getCurrShader();

    Shader& getDepthShader();

    Shader& getAlbedoShader();

    Shader& getShadowsShader();

    Shader& getBlurShader();

    Shader& getTextureCombinerShader();

	Shader& getEditorIdShader();

	Shader& getBokeShader();

	Shader& getGBufferShader();

    Shader& getParticlesShader();

    LightingShaderRoutine& getShaderRoutine(); 

    void checkForShaderReload(); 

    STAGE stage;

    unsigned int depthMap = 0;
private:

    LightingShaderRoutine routine;

    Shader lightingVertex;
    Shader lightingFragment;

    Shader depthVertex;
    Shader depthFragment;

    Shader shadowVertex;
    Shader shadowFragment;

    Shader blurVertex;
    Shader blurFragment;

    Shader textureCombinerVertex;
    Shader textureCombinerFragment;

    Shader editorIdVertex;
    Shader editorIdFragment;

    Shader bokeVertex;
    Shader bokeFragment;

    Shader gBufferVertex;
    Shader gBufferFragment;

    Shader deferredLightPasVertex;
    Shader deferredLightPasFragment;

    Shader particlesVertex;
    Shader particlesFragment;
};

