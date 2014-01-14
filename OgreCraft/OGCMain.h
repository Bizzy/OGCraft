#ifndef __OGCMain_h_
#define __OGCMain_h_

#include "BaseApplication.h"

class OGCMain : public BaseApplication
{
public:
    OGCMain(void);
    virtual ~OGCMain(void);
protected:
    virtual void createScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
private:
    bool processUnbufferedInput(const Ogre::FrameEvent& evt);
};

#endif // #ifndef __OGCMain_h_
