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

#pragma mark Collisions
bool GameplayScene::onContactBegin(PhysicsContact& contact)
{
    CCLOG("onContactBegin!");
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    //0 = Floor
    //1 = Player
    //2 = food
    //3 = Left Wall
    //4 = turd
    #define FLOOR_TAG 0
    #define PLAYER_TAG 1
    #define FOOD_TAG 2
    #define LEFT_WALL_TAG 3
    #define TURD_TAG 4
    
    if ((bodyA->getTag() == PLAYER_TAG) && (bodyB->getTag() == FLOOR_TAG)) {
        bodyA->setVelocity(Vec2(0.0f, 0.0f));
        touchedGround = true;
    }
    else if ((bodyA->getTag() == FLOOR_TAG) && (bodyB->getTag() == PLAYER_TAG)) {
        bodyB->setVelocity(Vec2(0.0f, 0.0f));
        touchedGround = true;
    }
    else if ((bodyA->getTag() == FOOD_TAG) && (bodyB->getTag() == LEFT_WALL_TAG)) {
        removeChild(bodyA->getNode());
    }
    else if ((bodyA->getTag() == LEFT_WALL_TAG) && (bodyB->getTag() == FOOD_TAG)) {
        removeChild(bodyB->getNode());
    }
    else if ((bodyA->getTag() == FOOD_TAG) && (bodyB->getTag() == PLAYER_TAG)) {
        removeChild(bodyA->getNode());
        bodyB->getNode()->setPosition(Vec2(visibleSize.width/2 + origin.x, bodyB->getPosition().y));
        spawnTurdSprite(bodyB);
        
        
    }
    else if ((bodyA->getTag() == PLAYER_TAG) && (bodyB->getTag() == FOOD_TAG)) {
        removeChild(bodyB->getNode());
        bodyA->getNode()->setPosition(Vec2(visibleSize.width/2 + origin.x, bodyA->getPosition().y));
        spawnTurdSprite(bodyA);
    }
    else if ((bodyA->getTag() == TURD_TAG) && (bodyB->getTag() == LEFT_WALL_TAG)) {
        removeChild(bodyA->getNode());
    }
    else if ((bodyA->getTag() == LEFT_WALL_TAG) && (bodyB->getTag() == TURD_TAG)) {
        removeChild(bodyB->getNode());

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
    timeOnScreen = 4;
    enemySpawnRate = 30;
    runningSpeed = 10;
    
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
    this->addChild(floorNode, 1);
    
    auto leftWallPhysicsBody = PhysicsBody::createEdgeBox(Size(1, winSize.height));
    leftWallPhysicsBody->setDynamic(false);
    leftWallPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    leftWallPhysicsBody->setTag(3);
    leftWallNode->setPosition(Vec2(origin.x, winSize.height/2 + origin.y));
    leftWallNode->setPhysicsBody(leftWallPhysicsBody);
    this->addChild(leftWallNode, 1);
    
    auto roofPhysicsBody = PhysicsBody::createEdgeBox(Size(winSize.width, 1));
    roofPhysicsBody->setDynamic(false);
    roofPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    roofPhysicsBody->setTag(4);
    roofNode->setPosition(Vec2(winSize.width/2 + origin.x, origin.y + winSize.height));
    roofNode->setPhysicsBody(roofPhysicsBody);
    this->addChild(roofNode, 1);
    
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
    
    this->addChild(mySprite, 1);
    
    //Trigger spriteSpawn at interval
    //this->schedule(schedule_selector(GameplayScene::spawnRandomSprite), 0.5);
    
    this->schedule(schedule_selector(GameplayScene::scoreTimer), 0.001);
    
    //this->schedule(schedule_selector(GameplayScene::decreaseTimeOnScreen), 5.0);
    
    return true;
}

void GameplayScene::scoreTimer(float delta){
    time = time + 1;
    CCLOG("%d", time);
    if (touchedGround) {
        label->setString(std::to_string(score));
        score = score + 10;
    }

    if (time % enemySpawnRate == 0) {
        //int number = arc4random() % 3;
        //if (number == 0) {
        //    spawnRandomSprite(delta);
        //}
        CCLOG("Spawn enemy");
        spawnRandomSprite(delta);
    }
    
    if (time % runningSpeed == 0) {
        decreaseTimeOnScreen(delta);
    }
    
}

void GameplayScene::decreaseTimeOnScreen(float delta){
    #define changeInSpeed 0.01
    if (timeOnScreen - changeInSpeed > 1) {
       timeOnScreen = timeOnScreen - changeInSpeed;
    }
    else{
        CCLOG("max speed reached!");
    }
    
}

void GameplayScene::spawnTurdSprite(cocos2d::PhysicsBody* playerBody){
    CCLOG("Spawning turd!");
    auto turdSprite = Sprite::create("megaman.png");
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // create a static PhysicsBody
    auto turdPhysicsBody = PhysicsBody::createBox(Size(turdSprite->getSpriteFrame()->getRect().getMaxX() * turdSprite->getScale(), turdSprite->getSpriteFrame()->getRect().getMaxY() * turdSprite->getScale()), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    turdPhysicsBody->setGravityEnable(true);
    turdPhysicsBody->setRotationEnable(false);
    turdPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    turdPhysicsBody->setTag(4);
    
    
    
    // sprite will use physicsBody
    turdSprite->setPhysicsBody(turdPhysicsBody);
    
    //make sprite bigger
    turdSprite->setScale(2.0);
    
    //set position
    turdSprite->setPosition(Vec2(playerBody->getPosition().x - turdSprite->getSpriteFrame()->getRect().getMaxX() * turdSprite->getScale(), playerBody->getPosition().y));
    
    auto moveTo = MoveBy::create(timeOnScreen, Vec2(-visibleSize.width,0));
    turdSprite->runAction(moveTo);

    this->addChild(turdSprite, 1);
    
    }

void GameplayScene::spawnRandomSprite(float delta)
{
    CCLOG("spawnRandomSprite!");
    auto foodSprite = Sprite::create("megaman.png");
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // create a static PhysicsBody
    auto foodPhysicsBody = PhysicsBody::createBox(Size(foodSprite->getSpriteFrame()->getRect().getMaxX() * foodSprite->getScale(), foodSprite->getSpriteFrame()->getRect().getMaxY() * foodSprite->getScale()), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    foodPhysicsBody->setGravityEnable(false);
    foodPhysicsBody->setRotationEnable(false);
    foodPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    foodPhysicsBody->setTag(2);
    
    //set position
    foodSprite->setPosition(Vec2(visibleSize.width,  (arc4random() % (int)visibleSize.height - foodSprite->getSpriteFrame()->getRect().getMaxY() * foodSprite->getScale())));
    
    // sprite will use physicsBody
    foodSprite->setPhysicsBody(foodPhysicsBody);
    
    //make sprite bigger
    foodSprite->setScale(2.0);

    auto moveBy = MoveBy::create(timeOnScreen, Vec2(-visibleSize.width,0));
    foodSprite->runAction(moveBy);
    foodSprite->setAnchorPoint(Vec2(1, 0.5));
    
    this->addChild(foodSprite, 1);
    
    
    
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
