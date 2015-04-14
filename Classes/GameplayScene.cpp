#include "GameplayScene.h"

USING_NS_CC;

static const int kScrollSpeed = 3;

Scene* GameplayScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setSpeed(3.0f);
    scene->getPhysicsWorld()->setGravity(Vect(0,-50.0f));
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
    //CCLOG("onContactBegin!");
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    #define FLOOR_TAG 0
    #define PLAYER_TAG 1
    #define FOOD_TAG 2
    #define LEFT_WALL_TAG 3
    #define TURD_TAG 4
    #define ENEMY_TAG 5
    
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
    else if ((bodyA->getTag() == FOOD_TAG) && (bodyB->getTag() == TURD_TAG)) {
        return false;
    }
    else if ((bodyA->getTag() == TURD_TAG) && (bodyB->getTag() == FOOD_TAG)) {
        return false;
    }
    else if ((bodyA->getTag() == FLOOR_TAG) && (bodyB->getTag() == TURD_TAG)) {
        bodyB->setVelocity(Vec2(bodyB->getVelocity().x, 0.0f));
    }
    else if ((bodyA->getTag() == TURD_TAG) && (bodyB->getTag() == FLOOR_TAG)) {
        bodyA->setVelocity(Vec2(bodyA->getVelocity().x, 0.0f));
    }
    else if ((bodyA->getTag() == FOOD_TAG) && (bodyB->getTag() == ENEMY_TAG)) {
        return false;
    }
    else if ((bodyA->getTag() == ENEMY_TAG) && (bodyB->getTag() == FOOD_TAG)) {
        return false;
    }
    else if ((bodyA->getTag() == ENEMY_TAG) && (bodyB->getTag() == TURD_TAG)) {
        removeChild(bodyB->getNode());
        hitLogic();
    }
    else if ((bodyA->getTag() == TURD_TAG) && (bodyB->getTag() == ENEMY_TAG)) {
        removeChild(bodyA->getNode());
        hitLogic();
    }
    else if ((bodyA->getTag() == ENEMY_TAG) && (bodyB->getTag() == ENEMY_TAG)) {
        return false;
    }
    else if ((bodyA->getTag() == ENEMY_TAG) && (bodyB->getTag() == ENEMY_TAG)) {
        return false;
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
    touchedGround = false;
    
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
    
    // create and initialize a label
    label = Label::createWithTTF("Gameplay Scene", "fonts/Marker Felt.ttf", 24);
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);
    
    //initalize score
    score = 0;
    timeOnScreen = 3;
    enemySpawnRate = 30;
    runningSpeed = 10;
    
    #pragma mark - bounding box
    auto floorNode = Node::create();
    auto leftWallNode = Node::create();
    auto roofNode = Node::create();
    CCSize winSize = CCDirector::getInstance()->getWinSize();
    
    auto floorPhysicsBody = PhysicsBody::createEdgeBox(Size(winSize.width, 5));
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
    //this->addChild(leftWallNode, 1);
    
    auto roofPhysicsBody = PhysicsBody::createEdgeBox(Size(winSize.width, 1));
    roofPhysicsBody->setDynamic(false);
    roofPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    roofPhysicsBody->setTag(4);
    roofNode->setPosition(Vec2(winSize.width/2 + origin.x, origin.y + winSize.height));
    roofNode->setPhysicsBody(roofPhysicsBody);
    this->addChild(roofNode, 1);
    
    #pragma mark - mySprite
    //Player sprite
    auto mySprite = Sprite::create("megaman.png");
    auto mySpritePhysicsBody = PhysicsBody::createBox(Size(mySprite->getSpriteFrame()->getRect().getMaxX() * mySprite->getScale(), mySprite->getSpriteFrame()->getRect().getMaxY() * mySprite->getScale()), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    mySpritePhysicsBody->setGravityEnable(true);
    mySpritePhysicsBody->setRotationEnable(false);
    mySpritePhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    mySpritePhysicsBody->setTag(1);
    mySprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    mySprite->setPhysicsBody(mySpritePhysicsBody);
    mySprite->setScale(2.0);
    this->addChild(mySprite, 1);
    
    //add contact event listener
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameplayScene::onContactBegin, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
    
    
    
    //Tap event
    auto listener1 = EventListenerTouchOneByOne::create();
    // trigger when you push down
    listener1->onTouchBegan = [=](Touch* touch, Event* event){
        CCLOG("onTouchBegan");
        mySprite->getPhysicsBody()->setVelocity(Vec2(0,100));
        return true;
    };
    // trigger when moving touch
    listener1->onTouchMoved = [](Touch* touch, Event* event){
    };
    // trigger when you let up
    listener1->onTouchEnded = [=](Touch* touch, Event* event){
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);
    
    //Scrolling background
    _bg1 = Sprite::create("cloudbackground.png");
    _bg1->setPosition(Vec2(0, visibleSize.height * 0.5f));
    _bg1->setAnchorPoint(Vec2(0,0.5f));
    this->addChild(_bg1, 0);
    
    _bg2 = Sprite::create("cloudbackground.png");
    _bg2->setPosition(Vec2(_bg2->getContentSize().width, _bg1->getPosition().y));
    _bg2->setAnchorPoint(Vec2(0,0.5f));
    this->addChild(_bg2, 0);
    
    //Schedule events
    this->schedule(schedule_selector(GameplayScene::scoreTimer), 0.001);
    this->scheduleUpdate();
   
    spawnEnemySprite();
    
    return true;
}

void GameplayScene::update(float delta){
    //CCLOG("updating");
    
    cocos2d::Vec2 bg1Pos = _bg1->getPosition();
    cocos2d::Vec2 bg2Pos = _bg2->getPosition();
    int speed = (getContentSize().width/(timeOnScreen/2))/100;
    
    bg1Pos.x -= speed;
    bg2Pos.x -= speed;
    //CCLOG("%f / %f = %f",getContentSize().width, timeOnScreen, (getContentSize().width/timeOnScreen));
    
    // move scrolling background back from left to right end to achieve "endless" scrolling
    if (bg1Pos.x < -(_bg1->getContentSize().width))
    {
        bg1Pos.x += _bg1->getContentSize().width;
        bg2Pos.x += _bg2->getContentSize().width;
    }
    
    // remove any inaccuracies by assigning only int values (this prevents floating point rounding errors accumulating over time)
    bg1Pos.x = (int)bg1Pos.x;
    bg2Pos.x = (int)bg2Pos.x;
    _bg1->setPosition(bg1Pos);
    _bg2->setPosition(bg2Pos);
}

void GameplayScene::scoreTimer(float delta){
    time = time + 1;
    //CCLOG("%d", time);
    if (touchedGround) {
        label->setString(std::to_string(score));
        score = score + 10;
    }
    if (time % enemySpawnRate == 0) {
        CCLOG("Spawn enemy");
        spawnRandomSprite(delta);
        //spawnEnemySprite();
    }
    if (time % runningSpeed == 0) {
        decreaseTimeOnScreen(delta);
    }
    
}

void GameplayScene::decreaseTimeOnScreen(float delta){
    #define changeInSpeed 0.05
    if (timeOnScreen - changeInSpeed > 1) {
       timeOnScreen = timeOnScreen - changeInSpeed;
    }
    else{
        CCLOG("max speed reached!");
    }
    
}

void GameplayScene::hitLogic(){
    
    CCLOG("Turd Hit!");
    enemyHitpoints = enemyHitpoints - 1;
    if (enemyHitpoints == 0) {
        removeChild(enemySprite);
        spawnEnemySprite();
    }
}

void GameplayScene::spawnEnemySprite(){
    CCLOG("Spawning enemy");
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Size visibleSize = Director::getInstance()->getVisibleSize();
    enemyHitpoints = 3;
    //Enemy Sprite
    enemySprite = Sprite::create("megaman.png");
    auto enemyPhysicsBody = PhysicsBody::createBox(Size(enemySprite->getSpriteFrame()->getRect().getMaxX(), enemySprite->getSpriteFrame()->getRect().getMaxY()), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    enemyPhysicsBody->setGravityEnable(true);
    enemyPhysicsBody->setRotationEnable(false);
    enemyPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    enemyPhysicsBody->setTag(5);
    enemySprite->setPosition(Vec2(origin.x + enemySprite->getSpriteFrame()->getRect().getMaxX(), origin.y + enemySprite->getSpriteFrame()->getRect().getMaxY()));
    enemySprite->setPhysicsBody(enemyPhysicsBody);
    enemySprite->setScale(2.0);
    
    auto moveBy = MoveBy::create(0.5, Vec2(10, 0));
    auto repeatForever = cocos2d::RepeatForever::create(moveBy);
    enemySprite->runAction(repeatForever);
    
    this->addChild(enemySprite, 1);
}

void GameplayScene::spawnTurdSprite(cocos2d::PhysicsBody* playerBody){
    CCLOG("Spawning turd!");
    auto turdSprite = Sprite::create("poopy.png");
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // create a static PhysicsBody
    auto turdPhysicsBody = PhysicsBody::createBox(Size(turdSprite->getSpriteFrame()->getRect().getMaxX() * turdSprite->getScale(), turdSprite->getSpriteFrame()->getRect().getMaxY() * turdSprite->getScale()), PhysicsMaterial(0.1f, 1.0f, 0.1f));
    turdPhysicsBody->setGravityEnable(true);
    turdPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    turdPhysicsBody->setTag(4);
    turdSprite->setPhysicsBody(turdPhysicsBody);
    turdSprite->setScale(2.0);
    turdSprite->setPosition(Vec2(playerBody->getPosition().x - turdSprite->getSpriteFrame()->getRect().getMaxX() * turdSprite->getScale(), playerBody->getPosition().y));
    auto moveTo = MoveTo::create(timeOnScreen, Vec2(-turdSprite->getSpriteFrame()->getRect().getMaxX() * turdSprite->getScale(),turdSprite->getPosition().y));
    //CCLOG("",visibleSize.width,timeOnScreen,visibleSize.width/timeOnScreen);
    auto actionMoveDone = CallFunc::create([=](){
        log("actionDone!");
        removeChild(turdSprite);
    });
    auto seq = Sequence::create(moveTo, actionMoveDone, NULL);
    turdSprite->runAction(seq);
    this->addChild(turdSprite, 1);
    
    }

void GameplayScene::spawnRandomSprite(float delta)
{
    CCLOG("spawnRandomSprite!");
    int foodnumber = arc4random() % 14;
    auto foodSprite = Sprite::create();
    switch (foodnumber) {
        case 0:
            foodSprite = Sprite::create("icecream2.png");
            break;
        case 1:
            foodSprite = Sprite::create("icecream.png");
            break;
        case 2:
            foodSprite = Sprite::create("cake2.png");
            break;
        case 3:
            foodSprite = Sprite::create("cake.png");
            break;
        case 4:
            foodSprite = Sprite::create("pizza.png");
            break;
        case 5:
            foodSprite = Sprite::create("shrimp4.png");
            break;
        case 6:
            foodSprite = Sprite::create("shrimp3.png");
            break;
        case 7:
            foodSprite = Sprite::create("shrimp2.png");
            break;
        case 8:
            foodSprite = Sprite::create("eggs.png");
            break;
        case 9:
            foodSprite = Sprite::create("sushishrimp.png");
            break;
        case 10:
            foodSprite = Sprite::create("chicken.png");
            break;
        case 11:
            foodSprite = Sprite::create("burger.png");
            break;
        case 12:
            foodSprite = Sprite::create("fries.png");
            break;
        case 13:
            foodSprite = Sprite::create("donut.png");
            break;
        default:
            break;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // create a static PhysicsBody
    auto foodPhysicsBody = PhysicsBody::createBox(Size(foodSprite->getSpriteFrame()->getRect().getMaxX() * foodSprite->getScale(), foodSprite->getSpriteFrame()->getRect().getMaxY() * foodSprite->getScale()), PhysicsMaterial(0.1f, 1.0f, 0.0f));
    foodPhysicsBody->setGravityEnable(false);
    foodPhysicsBody->setRotationEnable(false);
    foodPhysicsBody->setContactTestBitmask(0xFFFFFFFF);
    foodPhysicsBody->setTag(2);
    foodSprite->setPosition(Vec2(visibleSize.width,  (arc4random() % (int)visibleSize.height/2 - foodSprite->getSpriteFrame()->getRect().getMaxY() * foodSprite->getScale())));
    foodSprite->setPhysicsBody(foodPhysicsBody);
    foodSprite->setScale(2.0);

    auto moveBy = MoveBy::create(timeOnScreen, Vec2(-(visibleSize.width + visibleSize.width/2),0));
    auto moveTo = MoveTo::create(timeOnScreen*2, Vec2(-foodSprite->getSpriteFrame()->getRect().getMaxX() * foodSprite->getScale(),foodSprite->getPosition().y));
    auto actionMoveDone = CallFunc::create([=](){
        log("actionDone!");
        removeChild(foodSprite);
    });
    auto seq = Sequence::create(moveTo, actionMoveDone, NULL);
    foodSprite->runAction(seq);
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
