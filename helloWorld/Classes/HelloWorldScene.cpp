#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
	std::string file = "01.tmx";
	auto str  = String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename(file.c_str()).c_str());
	_tileMap = TMXTiledMap::createWithXML(str->getCString(), "");
	_background = _tileMap->layerNamed("Background");

	addChild(_tileMap, -1);

	TMXObjectGroup* objects = _tileMap->getObjectGroup("Object-Player");
	CCASSERT(NULL != objects, "'Objects player' object group not find");
	 
	auto  playerShowPoint = objects->getObject("PlayerShowUpPoint");
	CCASSERT(!playerShowPoint.empty(), "PlayerShowUpPoint object not found");

	int x = playerShowPoint["x"].asInt();
	int y = playerShowPoint["y"].asInt();

	_player = Sprite::create("029.png");
	_player->setPosition(x + _tileMap->getTileSize().width / 2, y + _tileMap->getTileSize().height / 2);
	_player->setScale(0.5);

	addChild(_player);
	setViewPointCenter(_player->getPosition());
	
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = [&](Touch *touch, Event* unused_event)->bool{return true;};
	listener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	_blockage = _tileMap->layerNamed("Blockage01");
	_blockage->setVisible(false);

	_foreground = _tileMap->getLayer("Foreground01");
	return true;
#if 0
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
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
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
#endif
    
}
cocos2d::Point HelloWorld::titleCoordForPosition(cocos2d::Point position)
{
	int x = position.x / _tileMap->getTileSize().width;
	int y = ((_tileMap->getMapSize().height * _tileMap->getTileSize().height) - position.y) / _tileMap->getTileSize().height;
	return Point(x, y);
}
void HelloWorld::onTouchEnded(Touch *touch, Event *unused_event)
{
	auto actionTo1 = RotateTo::create(0, 0, 180);
	auto actionTo2 = RotateTo::create(0, 0, 0);
	auto touchLocation = touch->getLocation();

	touchLocation = this->convertToNodeSpace(touchLocation);

	auto playerPos = _player->getPosition();
	auto diff = touchLocation - playerPos;
	if (abs(diff.x)  >  abs(diff.y))
	{
		if (diff.x > 0)
		{
			playerPos.x += _tileMap->getTileSize().width / 2;
			_player->runAction(actionTo2); 
		}
		else
		{
			playerPos.x -= _tileMap->getTileSize().width / 2;
			_player->runAction(actionTo1);
		}
	}
	else 
	{
		if (diff.y > 0)
		{
			playerPos.y += _tileMap->getTileSize().height / 2;
		}
		else
		{
			playerPos.y -= _tileMap->getTileSize().height / 2;
		}
	}

	if (playerPos.x <=(_tileMap->getMapSize().width * _tileMap->getMapSize().width) &&
		playerPos.y <=(_tileMap->getMapSize().height * _tileMap->getMapSize().height)&& 
		playerPos.x >= 0 &&
		playerPos.y >= 0)
	{
		this->setPlayerPosition(playerPos);
	}
}
void HelloWorld::setPlayerPosition(cocos2d::Point position)
{
	Point tileCoord = this->titleCoordForPosition(position);
	int tileGid = _blockage->getTileGIDAt(tileCoord);
	if (tileGid)
	{
		auto propertis = _tileMap->getPropertiesForGID(tileGid).asValueMap();
		if (!propertis.empty())
		{
			auto collision = propertis["Blockage"].asString();
			CCLOG("log: %s", collision.c_str());
			if ("true" == collision)
			{
				return;
			}
		}
		auto collectable = propertis["Collectable"].asString();
		if ("True" == collectable)
		{
			_blockage->removeTileAt(tileCoord);
		}
		_foreground->removeTileAt(tileCoord);
	}	
	_player->setPosition(position);
}
void HelloWorld::setViewPointCenter(cocos2d::Point position)
{
	auto winsize = Director::getInstance()->getWinSize();

	int x = MAX(position.x, winsize.width / 2);
	int y = MAX(position.y, winsize.height / 2);
	x = MIN(x, (_tileMap->getMapSize().width * this->_tileMap->getTileSize().width) - winsize.width / 2);
	y = MIN(y, (_tileMap->getMapSize().height * _tileMap->getTileSize().height) - winsize.height / 2);
	auto actualPosition = Point(x, y);
	auto centerofView = Point(winsize.width / 2, winsize.height / 2);
	auto viewPoint = centerofView - actualPosition;
	this->setPosition(viewPoint);
}
void HelloWorld::menuCloseCallback(Ref* pSender)
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
