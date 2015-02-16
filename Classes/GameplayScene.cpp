#include "GameplayScene.h"

USING_NS_CC;

Scene* GameplayScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setSpeed(2.0f);
    // 'layer' is an autorelease object
    auto layer = GameplayScene::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

void GameplayScene::jump(Sprite* s){
    /*
    auto moveBy = MoveBy::create(2, Vec2(50,10));
    s->runAction(moveBy);
    */
    s->getPhysicsBody()->setVelocity(Vec2(0,100));
}

bool GameplayScene::onContactBegin(PhysicsContact& contact)
{
    CCLOG("onContactBegin!");
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    return true;
}


// on "init" you need to initialize your instance
bool GameplayScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
    
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(GameplayScene::menuCloseCallback, this));
    
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));
    
    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    /////////////////////////////
    // 3. add your codes below...
    
    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Gameplay Scene", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));
    
    // add the label as a child to this layer
    this->addChild(label, 1);
    
    /*
    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    
    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    
    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    */
    
    #pragma mark - bounding box
    auto node = Node::create();
    CCSize winSize = CCDirector::getInstance()->getWinSize();
    auto borderPhysicsBody = PhysicsBody::createEdgeBox(Size(winSize.width, winSize.height));
    borderPhysicsBody->setDynamic(false);
    borderPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    node->setPosition(Vec2(winSize.width/2 + origin.x, winSize.height/2 + origin.y));
    node->setPhysicsBody(borderPhysicsBody);
    this->addChild(node, 0);
    
    
    #pragma mark - mySprite
    
    // create a static PhysicsBody
    auto mySpritePhysicsBody = PhysicsBody::createBox(Size(65.0f , 81.0f ), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    //mySpritePhysicsBody->setDynamic(true);
    mySpritePhysicsBody->setGravityEnable(true);
    mySpritePhysicsBody->setRotationEnable(false);
    mySpritePhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    //initialize my sprite
    auto mySprite = Sprite::create("megaman.png");
    mySprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    
    // sprite will use physicsBody
    mySprite->setPhysicsBody(mySpritePhysicsBody);
    
    //add contact event listener
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameplayScene::onContactBegin, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
    
    
    //  Create a "one by one" touch event listener
    // (processes one touch at a time)
    auto listener1 = EventListenerTouchOneByOne::create();
    
    // trigger when you push down
    listener1->onTouchBegan = [=](Touch* touch, Event* event){
        CCLOG("onTouchBegan");
        /*
        auto moveBy = MoveBy::create(2, Vec2(50,10));
        mySprite->runAction(moveBy);
        */
        mySprite->getPhysicsBody()->setVelocity(Vec2(0,250));
        return true; // if you are consuming it
    };
    
    // trigger when moving touch
    listener1->onTouchMoved = [](Touch* touch, Event* event){
        // your code
    };
    
    // trigger when you let up
    listener1->onTouchEnded = [=](Touch* touch, Event* event){
        // your code
    };
    
    // Add listener
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);
    
    
    
    
    this->addChild(mySprite, 0);
    
    
    return true;
}


void GameplayScene::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif
    
    Director::getInstance()->end();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
