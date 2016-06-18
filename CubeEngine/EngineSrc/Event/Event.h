#ifndef TZW_EVENT_H
#define TZW_EVENT_H
#include "../Math/vec2.h"
#include <string>

namespace tzw {
class EventMgr;
class Event
{
public:
    virtual bool onKeyPress(std::string keyCode);
    virtual ~Event();
    virtual bool onKeyRelease(std::string keyCode);
    virtual bool onMouseRelease(int button,vec2 pos);
    virtual bool onMousePress(int button,vec2 pos);
    virtual bool onMouseMove(vec2 pos);
    virtual void onFrameUpdate(float delta);
    Event();
    unsigned int eventPiority() const;
    void setEventPiority(unsigned int eventPiority);

    bool isSwallow() const;
    void setIsSwallow(bool isSwallow);

    EventMgr *parent() const;
    void setParent(EventMgr *parent);

protected:
    EventMgr * m_parent;
    bool m_isSwallow;
    unsigned int m_eventPiority;
};

} // namespace tzw

#endif // TZW_EVENT_H