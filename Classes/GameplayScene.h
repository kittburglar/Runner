#ifndef __GAMEPLAY_SCENE_H__
#define __GAMEPLAY_SCENE_H__

#include "cocos2d.h"

class GameplayScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    int score;
    int time;
    int enemySpawnRate;
    int runningSpeed;
    int enemyHitpoints;
    
    float timeOnScreen;
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    bool onContactBegin(cocos2d::PhysicsContact& contact);
    
    void spawnRandomSprite(float delta);
    
    void spawnTurdSprite(cocos2d::PhysicsBody* playerBody);
    
    void spawnEnemySprite();
    
    void scoreTimer(float delta);
    
    void decreaseTimeOnScreen(float delta);
    
    void actionEnd(void);
    
    bool touchedGround;
    
    void hitLogic(void);
    
    cocos2d::Label *label;
    
    void jump(cocos2d::Sprite* s);
    // implement the "static create()" method manually
    
    void rightButtonCallback(cocos2d::Ref* pSender);
    
    cocos2d::CCSprite* _bg1;
    cocos2d::CCSprite* _bg2;
    
    cocos2d::CCSprite* enemySprite;
    
    void update(float delta);

    CREATE_FUNC(GameplayScene);
    
};

#endif // __GAMEPLAY_SCENE_H__