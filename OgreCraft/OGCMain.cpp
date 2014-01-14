#include "OGCMain.h"

OGCMain::OGCMain(void)
{
}
 
//-------------------------------------------------------------------------------------
OGCMain::~OGCMain(void)
{
}
 
//-------------------------------------------------------------------------------------
void OGCMain::createScene(void)
{
    //mCameraMan->setStyle(OgreBites::CameraStyle::CS_ORBIT);
    mCamera->lookAt(Ogre::Vector3(0,0,0));
    mCameraMan->setTopSpeed(500);
    //Ambient Licht einstellen
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.25, 0.25, 0.25));
    //Ninja 3D Objekt laden
    Ogre::Entity* ninjaEntity = mSceneMgr->createEntity("Ninja", "ninja.mesh");
    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
    node->attachObject(ninjaEntity);
    //PointLight erstellen
    Ogre::Light* pointLight = mSceneMgr->createLight("pointLight");
    pointLight->setType(Ogre::Light::LT_POINT);
    pointLight->setPosition(Ogre::Vector3(250, 150, 250));
    pointLight->setDiffuseColour(Ogre::ColourValue::White);
    pointLight->setSpecularColour(Ogre::ColourValue::White);
    //Boden erstellen
    Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
    //Aus dem Plane ein Mesh erzeugen
    Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,plane, 15000, 15000, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
    Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");                //Aus dem Mesh ein Entity erzeugen
    mSceneMgr->getRootSceneNode()->createChildSceneNode("GroundNode")->attachObject(entGround); //Entity in Scene laden
    entGround->setMaterialName("Examples/Rockwall");        //Material des Modens festlegen
    entGround->setCastShadows(false);                       //Boden wirft keine Schatten
}
//-------------------------------------------------------------------------------------
bool OGCMain::processUnbufferedInput(const Ogre::FrameEvent& evt)
{
    static bool mMouseDown = false;     // If a mouse button is depressed
    static Ogre::Real mToggle = 0.0;    // The time left until next toggle
    static Ogre::Real mRotate = 0.13;   // The rotate constant
    static Ogre::Real mMove = 250;      // The movement constant

    //--------------------------------------

    //Den Status der Linken Maustaste abfragen. true = gedrückt
    bool currMouse = mMouse->getMouseState().buttonDown(OIS::MB_Left);

    //Wenn L-Maus gedrückt wurde das PointLight de-/aktivieren
    if (currMouse && !mMouseDown)
    {
        Ogre::Light* light = mSceneMgr->getLight("pointLight");
        light->setVisible(!light->isVisible());
    }

    //Die Kontrollvariable mMouseDown aktualisieren
    mMouseDown = currMouse;

    //--------------------------------------

    //mToggle dekrementieren für die zeit
    mToggle -= evt.timeSinceLastFrame;

    //Mit der Taste 1 kann man das Licht jetzt auch an/ausschalten. Jedoch nur jede halbe Sekunde.
    if ((mToggle < 0.0f ) && mKeyboard->isKeyDown(OIS::KC_1))
    {
        mToggle  = 0.5;
        Ogre::Light* light = mSceneMgr->getLight("pointLight");
        light->setVisible(!light->isVisible());
    }

    //--------------------------------------

    Ogre::Vector3 transVector = Ogre::Vector3::ZERO;

    if (mKeyboard->isKeyDown(OIS::KC_I)) // Forward
    {
        transVector.z -= mMove;
    }

    if (mKeyboard->isKeyDown(OIS::KC_K)) // Backward
    {
        transVector.z += mMove;
    }

    if (mKeyboard->isKeyDown(OIS::KC_U)) // Up
    {
        transVector.y += mMove;
    }
    if (mKeyboard->isKeyDown(OIS::KC_O)) // Down
    {
        transVector.y -= mMove;
    }

    if (mKeyboard->isKeyDown(OIS::KC_J)) // Left - yaw or strafe
    {
        if(mKeyboard->isKeyDown( OIS::KC_LSHIFT ))
        {
            // Yaw left
            mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(mRotate * 5));
        } else {
            transVector.x -= mMove; // Strafe left
        }
    }
    if (mKeyboard->isKeyDown(OIS::KC_L)) // Right - yaw or strafe
    {
        if(mKeyboard->isKeyDown( OIS::KC_LSHIFT ))
        {
            // Yaw right
            mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(-mRotate * 5));
        } else {
            transVector.x += mMove; // Strafe right
        }
    }

    mSceneMgr->getSceneNode("NinjaNode")->translate(transVector * evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);

    return true;
}
//-------------------------------------------------------------------------------------
bool OGCMain::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);
 
    if(!processUnbufferedInput(evt)) return false;
 
    return ret;
}
//-------------------------------------------------------------------------------------