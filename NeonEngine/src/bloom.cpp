#include "bloom.h"

#include "opengl_utils.h"
#include "shader.h"

#include <iostream>

unsigned int initialize_bloom(int num_bloom_textures, std::vector<TextureAndSize>& bloom_textures, int width, int height) {
    bloom_textures = std::vector<TextureAndSize>(num_bloom_textures);

    unsigned int bloom_fbo;
    glGenFramebuffers(1, &bloom_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
    glm::vec2 texture_size(width, height);

    for (int i = 0; i < num_bloom_textures; i++) {
        texture_size /= 2.0f;

        TextureAndSize mip;
        mip.size = texture_size;

        glGenTextures(1, &mip.texture_id);
        glBindTexture(GL_TEXTURE_2D, mip.texture_id);
        // we are downscaling an HDR color buffer, so we need a float texture format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mip.size.x, mip.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        bloom_textures[i] = mip;
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_textures[0].texture_id, 0);

    // setup attachments
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return bloom_fbo;
}

void bloom_downsampling(Shader* shader, unsigned int starting_texture_id, std::vector<TextureAndSize>& bloom_textures, int starting_width, int starting_height) {
    shader->use();
    shader->setVec2("srcResolution", glm::vec2(starting_width, starting_height));
    shader->setInt("srcTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, starting_texture_id);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Progressively downsample through the mip chain
    for (int i = 0; i < bloom_textures.size(); i++) {
        const TextureAndSize& mip = bloom_textures[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture_id, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render screen-filled quad of resolution of current mip
        renderQuad();

        // Set current mip resolution as srcResolution for next iteration
        shader->setVec2("srcResolution", mip.size);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture_id);
        // Disable Karis average for subsequent downsamples
        if (i == 0) {
            shader->setInt("mipLevel", 1);
        }
    }
}

void bloom_upsampling(Shader* shader, std::vector<TextureAndSize>& bloom_textures, float filter_radius) {
    shader->use();
    shader->setFloat("filterRadius", filter_radius);

    // Enable additive blending
    glBlendFunc(GL_ONE, GL_ONE);

    shader->setInt("srcTexture", 0);
    glActiveTexture(GL_TEXTURE0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = bloom_textures.size() - 1; i > 0; i--) {
        const TextureAndSize& mip = bloom_textures[i];
        const TextureAndSize& nextMip = bloom_textures[i - 1];

        // Bind viewport and texture from where to read
        glBindTexture(GL_TEXTURE_2D, mip.texture_id);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture_id, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render screen-filled quad of resolution of current mip
        renderQuad();
    }

    // Set blend func to default values
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}