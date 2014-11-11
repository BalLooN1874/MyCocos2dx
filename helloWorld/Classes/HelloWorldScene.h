#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

	void setViewPointCenter(cocos2d::Point position);
	void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	void setPlayerPosition(cocos2d::Point position);
	cocos2d::Point titleCoordForPosition(cocos2d::Point position);
private:
	cocos2d::TMXTiledMap* _tileMap;
	cocos2d::TMXLayer* _background;
	cocos2d::Sprite* _player;
	cocos2d::TMXLayer*  _blockage;
	cocos2d::TMXLayer* _foreground;
};

#endif // __HELLOWORLD_SCENE_H__
