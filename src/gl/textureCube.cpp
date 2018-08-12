#include <iostream>
#include <cstring>

#include "textureCube.h"

#include "tools/fs.h"

TextureCube::TextureCube() {
}

TextureCube::~TextureCube() {
    glDeleteTextures(1, &m_id);
}

bool TextureCube::load(const std::string &_path, bool _vFlip) {
    m_path = _path;

    // Init
    glGenTextures(1, &m_id);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef PLATFORM_RPI
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

    
    if (haveExt(_path,"png") || haveExt(_path,"PNG") ||
        haveExt(_path,"jpg") || haveExt(_path,"JPG") ||
        haveExt(_path,"jpeg") || haveExt(_path,"JPEG")) {

        unsigned char* data = loadPixels(_path, &m_width, &m_height, RGB, false);

        // LOAD FACES
        Face<unsigned char> **faces = new Face<unsigned char>*[6];

        int faceWidth = m_width / 3;
        int faceHeight = m_height / 4;

        for (int i = 0; i < 6; i++) {
            faces[i] = new Face<unsigned char>();
            faces[i]->data = new unsigned char[3 * faceWidth * faceHeight];
            faces[i]->width = faceWidth;
            faces[i]->height = faceHeight;
            faces[i]->currentOffset = 0;
        }

        for (int l = 0; l < m_height; ++l) {
            int jFace = (l - (l % faceHeight)) / faceHeight;

            for (int iFace = 0; iFace < 3; ++iFace) {
                Face<unsigned char> *face = NULL;
                int offset = 3 * (faceWidth * iFace + l * m_width);

                if (iFace == 2 && jFace == 1) face = faces[0]; // POS_Y
                if (iFace == 0 && jFace == 1) face = faces[1]; // NEG_Y
                if (iFace == 1 && jFace == 0) face = faces[2]; // POS_X
                if (iFace == 1 && jFace == 2) face = faces[3]; // NEG_X
                if (iFace == 1 && jFace == 1) face = faces[4]; // POS_Z
                if (iFace == 1 && jFace == 3) face = faces[5]; // NEG_Z

                if (face) {
                    // the number of components to copy
                    int n = sizeof(unsigned char) * faceWidth * 3;

                    std::memcpy(face->data + face->currentOffset, data + offset, n);
                    face->currentOffset += (3 * faceWidth);
                }
            }
        }

        // adjust NEG_Z face
        if (_vFlip) {
            faces[5]->flipHorizontal();
            faces[5]->flipVertical();
        }

        for (int i = 0; i < 6; i++) {
            faces[i]->upload(CubeMapFace[i], GL_UNSIGNED_BYTE);
        }

        delete[] data;
        for(int i = 0; i < 6; ++i) {
            delete[] faces[i]->data;
            delete faces[i];
        }
        delete[] faces;

    }
    else if (haveExt(_path, "hdr") || haveExt(_path,"HDR")) {
        FILE* file = fopen(_path.c_str(), "rb");

        RGBE_ReadHeader(file, &m_width, &m_height, NULL);
        float* data = new float[3 * m_width * m_height];
        RGBE_ReadPixels_RLE(file, data, m_width, m_height);

        fclose(file);

        // LOAD FACES
        Face<float> **faces = new Face<float>*[6];

        int faceWidth = m_width / 3;
        int faceHeight = m_height / 4;

        for (int i = 0; i < 6; i++) {
            faces[i] = new Face<float>();
            faces[i]->data = new float[3 * faceWidth * faceHeight];
            faces[i]->width = faceWidth;
            faces[i]->height = faceHeight;
            faces[i]->currentOffset = 0;
        }

        for (int l = 0; l < m_height; ++l) {
            int jFace = (l - (l % faceHeight)) / faceHeight;

            for (int iFace = 0; iFace < 3; ++iFace) {
                Face<float> *face = NULL;
                int offset = 3 * (faceWidth * iFace + l * m_width);

                if (iFace == 2 && jFace == 1) face = faces[0]; // POS_Y
                if (iFace == 0 && jFace == 1) face = faces[1]; // NEG_Y
                if (iFace == 1 && jFace == 0) face = faces[2]; // POS_X
                if (iFace == 1 && jFace == 2) face = faces[3]; // NEG_X
                if (iFace == 1 && jFace == 1) face = faces[4]; // POS_Z
                if (iFace == 1 && jFace == 3) face = faces[5]; // NEG_Z

                if (face) {
                    // the number of components to copy
                    int n = sizeof(float) * faceWidth * 3;

                    std::memcpy(face->data + face->currentOffset, data + offset, n);
                    face->currentOffset += (3 * faceWidth);
                }
            }
        }

        // adjust NEG_Z face
        if (_vFlip) {
            faces[5]->flipHorizontal();
            faces[5]->flipVertical();
        }

        for (int i = 0; i < 6; i++) {
            faces[i]->upload(CubeMapFace[i], GL_FLOAT);
        }

        delete[] data;
        for(int i = 0; i < 6; ++i) {
            delete[] faces[i]->data;
            delete faces[i];
        }
        delete[] faces;

    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    
    return true;
}

void TextureCube::bind() {
    // glActiveTexture(GL_TEXTURE0 + textureIndex);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    // return m_id;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}