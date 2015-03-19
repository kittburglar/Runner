#include "GameplayScene.h"

USING_NS_CC;

Scene* GameplayScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setSpeed(3.0f);
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
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    //0 = Floor
    //1 = Player
    //2 = Enemy
    //3 = Left Wall
    if ((bodyA->getTag() == 1) && (bodyB->getTag() == 0)) {
        bodyA->setVelocity(Vec2(0.0f, 0.0f));
        touchedGround = true;
    }
    else if ((bodyA->getTag() == 0) && (bodyB->getTag() == 1)) {
        bodyB->setVelocity(Vec2(0.0f, 0.0f));
        touchedGround = true;
    }
    else if ((bodyA->getTag() == 2) && (bodyB->getTag() == 3)) {
        removeChild(bodyA->getNode());
    }
    else if ((bodyA->getTag() == 3) && (bodyB->getTag() == 2)) {
        removeChild(bodyB->getNode());
    }
    else if ((bodyA->getTag() == 2) && (bodyB->getTag() == 1)) {
        removeChild(bodyA->getNode());
        bodyB->getNode()->setPosition(Vec2(visibleSize.width/2 + origin.x, bodyB->getPosition().y));
        
    }
    else if ((bodyA->getTag() == 1) && (bodyB->getTag() == 2)) {
        removeChild(bodyB->getNode());
        bodyA->getNode()->setPosition(Vec2(visibleSize.width/2 + origin.x, bodyA->getPosition().y));
    }
    
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
    
    label = Label::createWithTTF("Gameplay Scene", "fonts/Marker Felt.ttf", 24);
    
    touchedGround = false;
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));
    
    // add the label as a child to this layer
    this->addChild(label, 1);
    
    //initalize score
    score = 0;
    
    #pragma mark - bounding box
    auto floorNode = Node::create();
    auto leftWallNode = Node::create();
    auto roofNode = Node::create();
    CCSize winSize = CCDirector::getInstance()->getWinSize();
    
    auto floorPhysicsBody = PhysicsBody::createEdgeBox(Size(winSize.width, 1));
    floorPhysicsBody->setDynamic(false);
    floorPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    floorPhysicsBody->setTag(0);
    floorNode->setPosition(Vec2(winSize.width/2 + origin.x, origin.y));
    floorNode->setPhysicsBody(floorPhysicsBody);
    this->addChild(floorNode, 0);
    
    auto leftWallPhysicsBody = PhysicsBody::createEdgeBox(Size(1, winSize.height));
    leftWallPhysicsBody->setDynamic(false);
    leftWallPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    leftWallPhysicsBody->setTag(3);
    leftWallNode->setPosition(Vec2(origin.x, winSize.height/2 + origin.y));
    leftWallNode->setPhysicsBody(leftWallPhysicsBody);
    this->addChild(leftWallNode, 0);
    
    auto roofPhysicsBody = PhysicsBody::createEdgeBox(Size(winSize.width, 1));
    roofPhysicsBody->setDynamic(false);
    roofPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    roofPhysicsBody->setTag(4);
    roofNode->setPosition(Vec2(winSize.width/2 + origin.x, origin.y + winSize.height));
    roofNode->setPhysicsBody(roofPhysicsBody);
    this->addChild(roofNode, 0);
    
    #pragma mark - mySprite
    auto mySprite = Sprite::create("megaman.png");
    // create a static PhysicsBody
    auto mySpritePhysicsBody = PhysicsBody::createBox(Size(mySprite->getSpriteFrame()->getRect().getMaxX() * mySprite->getScale(), mySprite->getSpriteFrame()->getRect().getMaxY() * mySprite->getScale()), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    mySpritePhysicsBody->setGravityEnable(true);
    mySpritePhysicsBody->setRotationEnable(false);
    mySpritePhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    mySpritePhysicsBody->setTag(1);
    //initialize my sprite
    
    mySprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    
    // sprite will use physicsBody
    mySprite->setPhysicsBody(mySpritePhysicsBody);
    
    //make sprite bigger
    mySprite->setScale(2.0);
    
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
        mySprite->getPhysicsBody()->setVelocity(Vec2(0,170));
        
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
    
    //Trigger spriteSpawn at interval
    this->schedule(schedule_selector(GameplayScene::spawnRandomSprite), 2.0);
    
    this->schedule(schedule_selector(GameplayScene::scoreTimer), 0.01);
    
    
    return true;
}

void GameplayScene::scoreTimer(float delta){
    label->setString(std::to_string(score));
    score = score + 10;
}

void GameplayScene::rightButtonCallback(Ref* pSender){
    Director::getInstance()->pushScene(GameplayScene::createScene());
}


void GameplayScene::spawnRandomSprite(float delta)
{
    CCLOG("spawnRandomSprite!");
    auto enemySprite = Sprite::create("megaman.png");
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // create a static PhysicsBody
    auto enemyPhysicsBody = PhysicsBody::createBox(Size(enemySprite->getSpriteFrame()->getRect().getMaxX() * enemySprite->getScale() ,
                                                        enemySprite->getSpriteFrame()->getRect().getMaxY() * enemySprite->getScale()),
                                                   PhysicsMaterial(0.1f, 1.0f, 0.0f));
    //mySpritePhysicsBody->setDynamic(true);
    enemyPhysicsBody->setGravityEnable(false);
    enemyPhysicsBody->setRotationEnable(false);
    enemyPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    enemyPhysicsBody->setTag(2);
    //initialize my sprite
    
    enemySprite->setPosition(Vec2(visibleSize.width,  (arc4random() % (int)visibleSize.height - enemySprite->getSpriteFrame()->getRect().getMaxY() * enemySprite->getScale())));
    
    // sprite will use physicsBody
    enemySprite->setPhysicsBody(enemyPhysicsBody);
    
    //make sprite bigger
    enemySprite->setScale(2.0);

    auto moveBy = MoveBy::create(4, Vec2(-visibleSize.width,0));
    enemySprite->runAction(moveBy);
    enemySprite->setAnchorPoint(Vec2(1, 0.5));
    
    this->addChild(enemySprite, 0);
    
    
    
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
