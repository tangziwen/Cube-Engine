#pragma once
#include "2D/GUISystem.h"
#include "Base/GuidObj.h"
#include "rapidjson/document.h"
#define Node_TYPE_TRIGGER 0
#define Node_TYPE_RES 1
#define Node_TYPE_BEHAVIOR 2
#define Node_TYPE_OTHERS 3
#define Node_TYPE_KEY_TRIGGER 3
namespace tzw {

struct NodeAttrValue
{
  enum class Type
  {
    INT,
    FLOAT,
  	USER_PTR,
  };
	NodeAttrValue(int val);
	NodeAttrValue(float val);
	NodeAttrValue(void * val);
	NodeAttrValue();
	Type getType();
	int int_val;
	float float_val;
	void * usrPtr;
	Type m_type;
};
	
struct NodeAttr
{
	enum class Type
	{
	INPUT_ATTR,
	OUTPUT_ATTR,
	INVALID
	};
	std::string m_name;
	int gID;
	int tag;
	Type type;
	NodeAttr();
	NodeAttrValue eval();
	NodeAttrValue m_localAttrValue{};
};

struct GameNodeEditorNode : public GuidObj
{
public:
  NodeAttr* addIn(std::string attrName);
  NodeAttr* addOut(std::string attrName);
  std::string name;
  GameNodeEditorNode();
  std::vector<NodeAttr*>& getInAttrs();
  std::vector<NodeAttr*>& getOuAttrs();
  virtual void privateDraw();
  NodeAttr* checkInNodeAttr(int gid);
  NodeAttr* checkOutNodeAttr(int gid);
  virtual void onLinkOut(int startID, int endID, GameNodeEditorNode* other);
  virtual void onRemoveLinkOut(int startID,
                               int endID,
                               GameNodeEditorNode* other);
  virtual void onRemoveLinkIn(int startID,
                              int endID,
                              GameNodeEditorNode* other);
  virtual void onLinkIn(int startID, int endID, GameNodeEditorNode* other);
  NodeAttr* getAttrByGid(int GID);
  virtual ~GameNodeEditorNode();
  int getInputAttrLocalIndexByGid(int GID);
  int getOutputAttrLocalIndexByGid(int GID);
  NodeAttr* getInByIndex(int localIndex);
  NodeAttr* getOutByIndex(int localIndex);
  NodeAttr* getInByGid(int GID);
  NodeAttr* getOutByGid(int GID);
  virtual vec3 getNodeColor();
  virtual void load(rapidjson::Value& partData);
  virtual void dump(rapidjson::Value& partDocObj,
                    rapidjson::Document::AllocatorType& allocator);
  int m_nodeID;
  vec2 m_origin;
  bool isShowed;
  virtual int getType();
protected:
  std::vector<NodeAttr*> m_inAttr;
  std::vector<NodeAttr*> m_outAttr;
  std::map<int, int> m_inGlobalMap;
  std::map<int, int> m_OutGlobalMap;
};
}
