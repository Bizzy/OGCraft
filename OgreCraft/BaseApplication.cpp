#include "BaseApplication.h"
 
//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void) 
    : mRoot(0), 
    mCursorWasVisible(false),
    mShutDown(false),
    mPluginsCfg(Ogre::StringUtil::BLANK), 
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mCameraMan(0),
    mTrayMgr(0),
    mSceneMgr(0),
    mWindow(0),
    mInputManager(0),
    mDetailsPanel(0),
    mMouse(0),
    mKeyboard(0),
    mOverlaySystem(0)
{	
}
//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    //Fix for 1.9
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;
    if (mOverlaySystem) delete mOverlaySystem;

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}
 
bool BaseApplication::go(void)
{
    //====================================================Pfade zu Ressourcen/PlugIn Configs laden und parsen==============================
    //Hier werden die Pfade zu PlugIns und anderen Ressourcen in unsere Engine eingebunden
    loadRessources();
    //====================================================RENDERSYSTEM AKTIVIEREN===========================================================
    if(!(/*mRoot->restoreConfig() ||*/ mRoot->showConfigDialog()))    // configure // Show the configuration dialog and initialise the system
        return false;

    //RenderSystem *rs = mRoot->getRenderSystemByName("Direct3D9 Rendering Subsystem");
    //mRoot->setRenderSystem(rs);
    //rs->setConfigOption("Full Screen", "No");
    //rs->setConfigOption("Video Mode", "800 x 600 @ 32-bit colour");

    mWindow = mRoot->initialise(true, "OgreCraft");
    //====================================================Einen Scenemanager erstellen=====================================================
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC); //"DefaultSceneManager"     // Create the SceneManager, in this case a generic one
    mOverlaySystem = new Ogre::OverlaySystem(); 	//Fix for 1.9
    mSceneMgr->addRenderQueueListener(mOverlaySystem); 	//Fix for 1.9
    //====================================================Eine Kamera erstellen============================================================
    createCamera();
    //====================================================Einen ViewPort erstellen=========================================================
    createViewports();
    //====================================================Die Oben geladenen/geparsten Ressourcen initialisieren=====================================
    //TODO? createRessourceListener()
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5); // Set default mipmap level (note: some APIs ignore this)
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(); // initialise all resource groups
    //====================================================Eine Scene erstellen==================================================================
    createScene(); //Abstract, override me!
    //====================================================OIS Input System inintialisieren (Maus/Keyboard)=======================================
    createOIS();
    //====================================================Einen WindowEventListener erstellen============================================================
    windowResized(mWindow); //Set initial mouse clipping size
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this); //Register as a Window listener
    //==================================================== TrayMgr erstellen  ===========================================================================
    createTrayMgr();
    //====================================================Einen Render Loop erstellen============================================================
    mRoot->addFrameListener(this);
    mRoot->startRendering();

    return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
 
//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if(rw == mWindow)
    {
        if(mInputManager)
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );
            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed() || !mWindow->isActive())
        return false;
 
    if(mShutDown)
        return false;

    if(mKeyboard->isKeyDown(OIS::KC_ESCAPE))
        return false;

    mInputContext.capture(); //Maus und Tastatur Input erfassen

    mTrayMgr->frameRenderingQueued(evt);
    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }
 
    return true;
}

//=================================================================================================================================================
//=================================================================================================================================================
//=================================================================================================================================================
bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    mCameraMan->injectKeyDown(arg);
    return true;
}

bool BaseApplication::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool BaseApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    //mCameraMan->injectMouseMove(arg);
    if (arg.state.Z.rel != 0)  // move the camera toward or away from the target
    {
        //Ogre::Real dist = (mCamera->getPosition() - Ogre::Vector3(0,0,0)).length();
        //mCamera->moveRelative(Ogre::Vector3(0, 0, -arg.state.Z.rel * 0.004f * dist));
        if (arg.state.Z.rel > 0)
        {
            //zoom in
            mCameraMan->setZoom(true);
        }
        else if (arg.state.Z.rel < 0)
        {
            //zoom out
            mCameraMan->setZoom(false);
        }
    }
    return true;
}

bool BaseApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);

    return true;
}

bool BaseApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}
//=================================================================================================================================================
//=================================================================================================================================================
//=================================================================================================================================================
void BaseApplication::createCamera(void)
{
    mCamera = mSceneMgr->createCamera("PlayerCam");
     mCamera->setPosition(Ogre::Vector3(0,1000,500));
    mCamera->lookAt(Ogre::Vector3(0,0,-300)); // Look back along -Z
    mCamera->setNearClipDistance(5);
    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}

void BaseApplication::createViewports(void)
{
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);    // Create one viewport, entire window
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0)); 
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight())); //Alter the camera aspect ratio to match the viewport
}

void BaseApplication::createOIS(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr; 
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str())); 
    mInputManager = OIS::InputManager::createInputSystem( pl );
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

	mInputContext.mKeyboard = mKeyboard; //Fix for 1.9
    mInputContext.mMouse = mMouse; //Fix for 1.9
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
}

void BaseApplication::createTrayMgr(void)
{
	//Fix for 1.9 - put this in:
	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    //mTrayMgr->hideCursor();
    // create a params panel for displaying sample details
    Ogre::StringVector items;    items.push_back("cam.pX");    items.push_back("cam.pY");    items.push_back("cam.pZ");    items.push_back("");
    items.push_back("cam.oW");    items.push_back("cam.oX");    items.push_back("cam.oY");    items.push_back("cam.oZ");    items.push_back("");
    items.push_back("Filtering");    items.push_back("Poly Mode");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

    //OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
    //mutableMouseState.X.abs = mCamera->getViewport()->getActualWidth() / 2;
    //mutableMouseState.Y.abs = mCamera->getViewport()->getActualHeight() / 2;
}

void BaseApplication::loadRessources(void)
{
    #ifdef _DEBUG
        mResourcesCfg = "resources_d.cfg";
        mPluginsCfg = "plugins_d.cfg";
    #else
        mResourcesCfg = "resources.cfg";
        mPluginsCfg = "plugins.cfg";
    #endif 
    //Ein neues Ogre Root Objekt erstellen
    mRoot = new Ogre::Root(mPluginsCfg);
    // Ein Parser parst die Config Dateien     //set up resources. Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
    // Go through all sections & settings in the file und übergeben jede Ressource die wir finden an den RessourceGroupManager
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
 
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
}