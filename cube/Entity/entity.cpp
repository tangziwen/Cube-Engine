#include "entity.h"
#include <iostream>
#include "geometry/mesh.h"
#include "utility.h"
#include "string.h"
#include "base/bonedata.h"
#include "material/materialpool.h"
#include "texture/texturepool.h"
#include "shader/shader_program.h"
#include "external/TUtility/TUtility.h"
#include "external/converter/Loader.h"
#include <QDebug>
Entity::Entity()
{
    this->m_pScene = nullptr;
    this->mesh_list.clear();

    this->m_numBones = 0;
    this->m_animateTime = 0;
    this->m_hasAnimation = 0;
    this->setIsEnableShadow(true);
    this->onRender = nullptr;
    this->setNodeType (NODE_TYPE_ENTITY);
    this->setShaderProgram (ShaderPool::getInstance ()->get ("default"));
    m_isAABBDirty = true;
}

Entity::Entity(const char *file_name,LoadPolicy policy)
{
    this->m_pScene = nullptr;
    this->mesh_list.clear();
    this->m_numBones = 0;
    this->m_animateTime = 0;
    this->setIsEnableShadow(true);
    this->onRender = nullptr;
    this->setShaderProgram (ShaderPool::getInstance ()->get ("default"));
    this->setNodeType (NODE_TYPE_ENTITY);
    switch(policy)
    {
    case LoadPolicy::LoadFromAssimp:
    {
        loadModelData(file_name);
        m_model = nullptr;
    }
        break;
    case LoadPolicy::LoadFromLoader:
    {

        tzw::Loader loader;
        loader.loadFromModel (file_name);
        loadModelDataFromTZW (loader.model (),file_name);
        m_model = loader.model ();
    }
        break;
    case LoadPolicy::LoadFromTzw:
    {
        tzw::CMC_Model * model = new tzw::CMC_Model;
        model->loadFromTZW (file_name);
        loadModelDataFromTZW (model,file_name);
        m_model = model;
    }
        break;
    }
    m_isAABBDirty = true;
}

void Entity::addMesh(TMesh *mesh)
{
    this->mesh_list.push_back(mesh);
}

TMesh *Entity::getMesh(int index)
{
    return mesh_list[index];
}

void Entity::draw(bool withoutexture)
{
    for(int i=0;i<mesh_list.size();i++)
    {
        TMesh * mesh =mesh_list[i];
        MeshDrawComand * command = mesh->getCommand();
        command->synchronizeData(program,mesh->getMaterial(),mesh->getVerticesVbo(),mesh->getIndicesVbo());
        command->CommandSetRenderState(NULL,NULL,-1,-1,withoutexture);
        command->Draw();
    }
}

void Entity::setShaderProgram(ShaderProgram *program)
{
    this->program = program;
}

void Entity::setCamera(Camera *camera)
{
    this->camera = camera;
}

Camera *Entity::getCamera()
{
    return this->camera;
}

ShaderProgram *Entity::getShaderProgram()
{
    return this->program;
}

void Entity::bonesTransform(float TimeInSeconds, std::vector<Matrix4f> &Transforms,std::string animation_name)
{
    if(m_model)
    {
        //bonesTransformTZW(TimeInSeconds,Transforms,animation_name);
    }else
    {
        bonesTransformAssimp(TimeInSeconds,Transforms,animation_name);
    }
}

void Entity::bonesTransformAssimp(float TimeInSeconds, std::vector<Matrix4f> &Transforms, string animation_name)
{
    if(!m_pScene  || m_pScene->mNumAnimations<=0)
    {
        Transforms.clear();
        return;
    }
    Matrix4f Identity;
    Identity.InitIdentity();
    float TicksPerSecond =(float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, (float)m_pScene->mAnimations[0]->mDuration);
    readNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);
    Transforms.resize(m_numBones);
    for (uint i = 0 ; i < m_numBones ; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}

void Entity::bonesTransformTZW(float TimeInSeconds, std::vector<QMatrix4x4> &Transforms, string animation_name)
{
    if(!hasAnimation ())
    {
        Transforms.clear ();
        return;
    }

    int numOfBones = m_model->m_boneMetaInfoList.size ();
    Transforms.resize(numOfBones);
    QMatrix4x4 Identity;
    Identity.setToIdentity ();
    float TicksPerSecond =(float)(m_model->m_animate.m_ticksPerSecond != 0 ? m_model->m_animate.m_ticksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, m_model->m_animate.m_duration);
    readNodeHeirarchyTZW (AnimationTime, m_model->rootBone (), Identity,Transforms);


}

void Entity::animate(float time, const char *animation_name)
{
    m_animateTime = time;
}
float Entity::animateTime() const
{
    return m_animateTime;
}

void Entity::setAnimateTime(float animateTime)
{
    m_animateTime = animateTime;
}
std::string Entity::animationName() const
{
    return m_animationName;
}

void Entity::setAnimationName(const std::string &animationName)
{
    m_animationName = animationName;
}
bool Entity::hasAnimation() const
{
    return m_hasAnimation;
}

void Entity::setHasAnimation(bool hasAnimation)
{
    m_hasAnimation = hasAnimation;
}
bool Entity::isEnableShadow() const
{
    return m_isEnableShadow;
}

void Entity::setIsEnableShadow(bool isEnableShadow)
{
    m_isEnableShadow = isEnableShadow;
}

AABB Entity::getAABB()
{
    if(m_isAABBDirty)
    {
        for(int i =0;i<mesh_list.size ();i++)
        {
            m_aabb.merge (mesh_list[i]->aabb());
        }
        m_aabb.transForm (getModelTrans ());
        m_isAABBDirty = false;
    }
    return m_aabb;
}

float Entity::getDistToCamera()
{
    QMatrix4x4 viewMat  = camera->getViewMatrix ();
    QMatrix4x4 transform = getModelTrans ();
    float globalZ = -1*(viewMat.data ()[2] * transform.data ()[12] + viewMat.data ()[6] * transform.data ()[13] + viewMat.data ()[10] * transform.data ()[14] + viewMat.data ()[14]);
    return globalZ;
}

void Entity::adjustVertices()
{

}

uint Entity::findBoneInterpoScaling(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);
    
    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }
    assert(0);

    return 0;
}

uint Entity::findBoneInterpoRotation(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}

uint Entity::findBoneInterpoTranslation(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }
    assert(0);

    return 0;
}

const aiNodeAnim *Entity::findNodeAnim(const aiAnimation *pAnimation, const std::string NodeName)
{
    for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }
    return NULL;
}

void Entity::readNodeHeirarchy(float AnimationTime, const aiNode *pNode, const Matrix4f &ParentTransform)
{
    string NodeName(pNode->mName.data);
    const aiAnimation* pAnimation = m_pScene->mAnimations[0];
    Matrix4f NodeTransformation(pNode->mTransformation);
    const aiNodeAnim * pNodeAnim = findNodeAnim(pAnimation, NodeName);
    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        aiVector3D Scaling;
        CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
        Matrix4f RotationM = Matrix4f(RotationQ.GetMatrix());

        // Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
        NodeTransformation =  TranslationM * RotationM * ScalingM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        auto globalInverse = m_globalInverseTransform;
        uint BoneIndex = m_BoneMapping[NodeName];
        auto offset =  m_BoneInfo[BoneIndex].BoneOffset;
        auto resultMatrix = globalInverse*GlobalTransformation*offset;
        m_BoneInfo[BoneIndex].FinalTransformation =  resultMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        readNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}

void Entity::readNodeHeirarchyTZW(float AnimationTime, const tzw::CMC_Bone *node, QMatrix4x4 parentTransform,std::vector<QMatrix4x4> &Transforms)
{
    std::string NodeName = node->info ()->name ();
    QMatrix4x4 NodeTransformation(node->m_localTransform);
    const tzw::CMC_AnimateBone * pNodeAnim = m_model->m_animate.findAnimateBone (NodeName);
    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        QVector3D Scaling;
        pNodeAnim->calcInterpolatedScaling (Scaling,AnimationTime);
        QMatrix4x4 ScalingM;
        ScalingM.setToIdentity ();
        ScalingM.scale (Scaling.x (),Scaling.y (),Scaling.z ());
        ScalingM = ScalingM.transposed ();

        // Interpolate rotation and generate rotation transformation matrix
        QQuaternion RotationQ;
        pNodeAnim->calcInterpolatedRotation (RotationQ,AnimationTime);
        QMatrix4x4 RotationM;
        RotationM.setToIdentity ();
        RotationM.rotate(RotationQ);
        RotationM = RotationM.transposed ();

        // Interpolate translation and generate translation transformation matrix
        QVector3D Translation;
        pNodeAnim->calcInterpolatedPosition (Translation,AnimationTime);
        QMatrix4x4 TranslationM;
        TranslationM.setToIdentity ();
        TranslationM.translate (Translation.x(),Translation.y (),Translation.z ());
        TranslationM = TranslationM.transposed ();

        // Combine the above transformations
        NodeTransformation =    ScalingM *RotationM *TranslationM;
    }

    QMatrix4x4 GlobalTransformation = NodeTransformation *parentTransform;

    if (m_model->m_BoneMetaInfoMapping.find(node->info ()->name ()) != m_model->m_BoneMetaInfoMapping.end ()) {
        auto globalInverse = m_model->globalInverseTransform () ;
        uint BoneIndex = m_model->m_BoneMetaInfoMapping[node->info ()->name ()];
        auto offsetMatrix = node->info ()->defaultOffset ();
        auto resultMatrix = offsetMatrix* GlobalTransformation * globalInverse ;
        Transforms[BoneIndex] =  resultMatrix;
    }

    for (uint i = 0 ; i < node->m_children.size (); i++) {
        readNodeHeirarchyTZW(AnimationTime, node->m_children[i], GlobalTransformation,Transforms);
    }
    return ;
}

void Entity::CalcInterpolatedScaling(aiVector3D &Out, float AnimationTime, const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    uint ScalingIndex = findBoneInterpoScaling(AnimationTime, pNodeAnim);
    uint NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

void Entity::CalcInterpolatedRotation(aiQuaternion &Out, float AnimationTime, const aiNodeAnim *pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    uint RotationIndex = findBoneInterpoRotation(AnimationTime, pNodeAnim);
    uint NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}

void Entity::CalcInterpolatedPosition(aiVector3D &Out, float AnimationTime, const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    uint PositionIndex = findBoneInterpoTranslation(AnimationTime, pNodeAnim);
    uint NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

void Entity::loadModelData(const char *file_name)
{
    const aiScene* pScene = m_Importer.ReadFile(file_name,aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |aiProcess_CalcTangentSpace);
    this->m_pScene = (aiScene*)pScene;
    if(pScene==NULL)
    {
        T_LOG<<"INVALID MODEL FILE OR INVALID PATH";
        return;
    }
    m_globalInverseTransform = pScene->mRootNode->mTransformation;
    m_globalInverseTransform = m_globalInverseTransform.Inverse();
    char str[100]={'\0'};
    utility::FindPrefix(file_name,str);
    LoadMaterial(pScene,file_name,str);
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    for(int i =0 ;i< pScene->mNumMeshes ;i++)
    {
        const aiMesh* the_mesh = pScene->mMeshes[i];
        TMesh * mesh =new TMesh();
        this->addMesh(mesh);
        //set material
        mesh->setMaterial(this->material_list[the_mesh->mMaterialIndex]);
        for(int j =0; j<the_mesh->mNumVertices;j++)
        {
            const aiVector3D* pPos = &(the_mesh->mVertices[j]);
            const aiVector3D* pNormal = &(the_mesh->mNormals[j]);
            const aiVector3D* pTexCoord = the_mesh->HasTextureCoords(0) ? &(the_mesh->mTextureCoords[0][j]) : &Zero3D;
            const aiVector3D* pTangent = &(the_mesh->mTangents[i]);
            VertexData vec;
            vec.position = QVector3D(pPos->x,pPos->y,pPos->z);
            vec.normalLine = QVector3D(pNormal->x,pNormal->y,pNormal->z);
            vec.texCoord = QVector2D(pTexCoord->x,pTexCoord->y);
	    if(pTangent)
	    {
		     vec.tangent = QVector3D(pTangent->x,pTangent->y,pTangent->z);
	    }

            mesh->pushVertex(vec);
        }

        for (unsigned int k = 0 ; k < the_mesh->mNumFaces ; k++) {
            const aiFace& Face = the_mesh->mFaces[k];
            assert(Face.mNumIndices == 3);
            mesh->pushIndex(Face.mIndices[0]);
            mesh->pushIndex(Face.mIndices[1]);
            mesh->pushIndex(Face.mIndices[2]);
        }
        //load bones
        loadBones(the_mesh,mesh);
        mesh->finish();
    }

}

void Entity::loadModelDataFromTZW(tzw::CMC_Model * cmc_model,const char * file_name)
{
    m_hasAnimation = cmc_model->m_hasAnimation;
    char str[100]={'\0'};
    utility::FindPrefix(file_name,str);
    //load material
    loadMaterialFromTZW(cmc_model,file_name,str);

    //global inverse transform(not supported yet)
    m_globalInverseTransform.InitIdentity ();

    for(int i =0 ;i< cmc_model->m_meshList.size ();i++)
    {
        auto the_mesh = cmc_model->m_meshList[i];
        TMesh * mesh =new TMesh();
        this->addMesh(mesh);
        //set material
        mesh->setMaterial(this->material_list[the_mesh->materialIndex ()]);
        for(int j =0; j<the_mesh->m_vertices.size();j++)
        {
            tzw::CMC_Vertex v = the_mesh->m_vertices[j];
            VertexData vec;
            vec.position = v.pos ();
            vec.normalLine = v.normal ();
            vec.texCoord = v.UV ();
            vec.tangent = v.tangent ();
            if(m_hasAnimation)
            {
                for(int i =0;i<4;i++)
                {
                    vec.boneId[i] = v.m_boneIds[i];
                    vec.boneWeight[i] = v.m_boneWeights[i];
                }
            }
            mesh->pushVertex(vec);
        }
        for (unsigned int k = 0 ; k < the_mesh->m_indices.size (); k++) {
            mesh->pushIndex (the_mesh->m_indices[k]);
        }
        mesh->finish();
    }
}

void Entity::LoadMaterial(const aiScene *pScene, const char * file_name, const char *pre_fix)
{
    //store material
    for(int i = 0 ;i<pScene->mNumMaterials;i++)
    {
        aiColor3D dif(0.f,0.f,0.f);
        aiColor3D amb(0.f,0.f,0.f);
        aiColor3D spec(0.f,0.f,0.f);
        aiMaterial * the_material = pScene->mMaterials[i];
        char file_postfix[100];
        sprintf(file_postfix,"%s_%d",file_name,i+1);
        Material * material = MaterialPool::getInstance()->createOrGetMaterial(file_postfix);
        MaterialChannel * ambient_channel =  material->getAmbient();
        MaterialChannel * diffuse_channel =  material->getDiffuse();
        MaterialChannel * specular_channel =  material->getSpecular();
        the_material->Get(AI_MATKEY_COLOR_AMBIENT,amb);
        the_material->Get(AI_MATKEY_COLOR_DIFFUSE,dif);
        the_material->Get(AI_MATKEY_COLOR_SPECULAR,spec);

        ambient_channel->color = QVector3D(amb.r,amb.g,amb.b);
        diffuse_channel->color = QVector3D(dif.r,dif.g,dif.b);
        specular_channel->color = QVector3D(spec.r,spec.g,spec.b);


        aiString normalPath;
        the_material->GetTexture (aiTextureType_NORMALS,0,&normalPath);
        auto normalTexture = loadTextureFromMaterial(normalPath.C_Str (),pre_fix);
        // we don't check the normal texture whether is a nullptr, because models which don't use normal mapping technique are very common.
        material->setNormalMap (normalTexture);

        //now ,we only use the first diffuse texture, will fix it later
        aiString diffuse_Path;
        the_material->GetTexture(aiTextureType_DIFFUSE,0,&diffuse_Path);
        auto diffuseTexture = loadTextureFromMaterial(diffuse_Path.C_Str (),pre_fix);
        if(!diffuseTexture) diffuseTexture = TexturePool::getInstance ()->createOrGetTexture ("default");
        diffuse_channel->texture = diffuseTexture;

        this->material_list.push_back(material);
    }
}

void Entity::loadMaterialFromTZW(tzw::CMC_Model *model, const char *file_name, const char *pre_fix)
{
    //store material
    for(int i = 0 ;i<model->m_materialList.size();i++)
    {
        auto the_material = model->m_materialList[i];
        char file_postfix[100];
        sprintf(file_postfix,"%s_%d",file_name,i+1);
        Material * material = MaterialPool::getInstance()->createOrGetMaterial(file_postfix);
        MaterialChannel * ambient_channel =  material->getAmbient();
        MaterialChannel * diffuse_channel =  material->getDiffuse();
        MaterialChannel * specular_channel =  material->getSpecular();

        ambient_channel->color = the_material->ambientColor;
        diffuse_channel->color = the_material->diffuseColor;
        specular_channel->color = the_material->specularColor;

        auto normalPath = the_material->normalTexturePath;
        auto normalTexture = loadTextureFromMaterial(normalPath,pre_fix);
        // we don't check the normal texture whether is a nullptr, because models which don't use normal mapping technique are very common.
        material->setNormalMap (normalTexture);

        //now ,we only use the first diffuse texture, will fix it later
        auto diffuse_Path = the_material->diffuseTexturePath;
        auto diffuseTexture = loadTextureFromMaterial(diffuse_Path,pre_fix);
        //if the diffuseTexture is not exist, we will use a default texture to replace.
        if(!diffuseTexture) diffuseTexture = TexturePool::getInstance ()->createOrGetTexture ("default");
        diffuse_channel->texture = diffuseTexture;

        this->material_list.push_back(material);
    }
}

void Entity::loadBones( const aiMesh *pMesh,TMesh * mesh)
{
    this->m_hasAnimation = m_pScene->HasAnimations();
    if(!m_hasAnimation)
    {
        return;
    }
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
        uint BoneIndex = 0;
        string BoneName(pMesh->mBones[i]->mName.data);
        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
            // Allocate an index for a new bone
            BoneIndex = m_numBones;
            m_numBones++;
            BoneInfo bi;
            m_BoneInfo.push_back(bi);
            m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;
            m_BoneMapping[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }
        for (uint j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
            uint VertexID = pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;
            mesh->getVertex (VertexID)->addBoneData (BoneIndex,Weight);
        }
    }
}

Texture *Entity::loadTextureFromMaterial(std::string fileName, const char *pre_fix)
{
    Texture * result = nullptr;
    if(fileName.empty ())
    {
        return result;
    }else
    {
        char str[100];
        Texture * tmp_tex = TexturePool::getInstance()->createOrGetTexture(fileName.c_str ());
        if(tmp_tex)
        {
            result =tmp_tex;
        }
        sprintf(str,"%s%s",pre_fix,fileName.c_str());
        tmp_tex = TexturePool::getInstance()->createOrGetTexture(str);
        if(tmp_tex)
        {
            result =tmp_tex;
        }
        sprintf(str,"%s%s","res/texture/",fileName.c_str ());
        tmp_tex = TexturePool::getInstance()->createOrGetTexture(str);
        if(tmp_tex)
        {
            result =tmp_tex;
        }
    }
    return result;
}

bool Entity::isSetDrawWire() const
{
    return m_isSetDrawWire;
}

void Entity::setIsSetDrawWire(bool isSetDrawWire)
{
    m_isSetDrawWire = isSetDrawWire;
}

