#ifndef __BaseApplication_h_
#define __BaseApplication_h_

#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>
#include <OgreAnimationState.h>

class BaseApplication : public Ogre::WindowEventListener, public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
    public:
        BaseApplication(void);
        virtual ~BaseApplication(void);
        virtual bool go(void);
    protected:
        // Ogre::WindowEventListener
        virtual void windowResized(Ogre::RenderWindow* rw);
        virtual void windowClosed(Ogre::RenderWindow* rw);
        // Ogre::FrameListener
        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
        // OIS::KeyListener
        virtual bool keyPressed( const OIS::KeyEvent &arg );
        virtual bool keyReleased( const OIS::KeyEvent &arg );
        // OIS::MouseListener
        virtual bool mouseMoved( const OIS::MouseEvent &arg );
        virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
        virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
        //Eigene
        virtual void createCamera(void);
        virtual void createViewports(void);
        virtual void createScene(void) = 0; // Override me!
        //virtual void destroyScene(void) = 0;
        virtual void createOIS(void);
        virtual void createTrayMgr(void);
        virtual void loadRessources(void);

        Ogre::Root*         mRoot;
        Ogre::String        mPluginsCfg;
        Ogre::String        mResourcesCfg;
        Ogre::RenderWindow* mWindow;
        Ogre::SceneManager* mSceneMgr;
        Ogre::Camera*       mCamera;
        OIS::InputManager*  mInputManager;
        OIS::Mouse*         mMouse;
        OIS::Keyboard*      mKeyboard;

        // OgreBites
        OgreBites::SdkTrayManager* mTrayMgr;
        OgreBites::SdkCameraMan* mCameraMan;       // basic camera controller
        OgreBites::ParamsPanel* mDetailsPanel;     // sample details panel

	    //Fix for 1.9:
	    OgreBites::InputContext mInputContext;
	    //Fix for 1.9
	    Ogre::OverlaySystem *mOverlaySystem;

        bool mCursorWasVisible;                    // was cursor visible before dialog appeared
        bool mShutDown;
};

#endif // #ifndef __BaseApplication_h_