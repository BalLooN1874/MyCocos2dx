#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace CocosDenshion;

HelloWorld::HelloWorldHud* HelloWorld::_hud = NULL;
Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

	auto hud = HelloWorldHud::create();
	_hud = hud;

	scene->addChild(hud);
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
	_numCollected = 0;
	//初始化音乐
	SimpleAudioEngine* ptrAudio = SimpleAudioEngine::getInstance();
	ptrAudio->preloadEffect("error.mp3");
	ptrAudio->preloadEffect("item.mp3");
	ptrAudio->preloadEffect("step.mp3");

	ptrAudio->preloadEffect("wade.mp3");
	ptrAudio->playBackgroundMusic("background.mp3");
	ptrAudio->setBackgroundMusicVolume(0.1);
	//添加地图
	std::string file = "01.tmx";
	auto str  = String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename(file.c_str()).c_str());
	_tileMap = TMXTiledMap::createWithXML(str->getCString(), "");
	_background = _tileMap->layerNamed("Background");

	addChild(_tileMap, -1);

	//显示地图
	TMXObjectGroup* objects = _tileMap->getObjectGroup("Object-Player");
	CCASSERT(NULL != objects, "'Objects player' object group not find");
	 
	auto  playerShowPoint = objects->getObject("PlayerShowUpPoint");
	CCASSERT(!playerShowPoint.empty(), "PlayerShowUpPoint object not found");

	int x = playerShowPoint["x"].asInt();
	int y = playerShowPoint["y"].asInt();

	//添加精灵
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

	//添加敌人
	for (auto& eSpawnPoint: objects->getObjects())
	{
		ValueMap& dict = eSpawnPoint.asValueMap();
		if (dict["Enemy"].asInt() == 1)
		{
			x = dict["x"].asInt();
			y = dict["y"].asInt();
			this->addEnemyAtPos(Point(x, y));
			
		}
	}
	this->schedule(schedule_selector(HelloWorld::testCollisions));
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
//添加敌人
void HelloWorld::addEnemyAtPos(cocos2d::Point pos)
{
	auto enemy = Sprite::create("030.png");
	enemy->setPosition(pos);
	enemy->setScale(0.5);
	
	this->animateEnemy(enemy);
	this->addChild(enemy);
	_enemies.pushBack(enemy);
}
void HelloWorld::enemyMoveFinished(cocos2d::Object* pSender)
{
	Sprite* enemy = (Sprite*)pSender;
	this->animateEnemy(enemy);
}
void HelloWorld::animateEnemy(cocos2d::Sprite* enemy)
{

	//控制怪物转身
	auto actionTo1 = RotateTo::create(0, 0, 180);
	auto actionTo2 = RotateTo::create(0, 0, 0);
	auto diff = ccpSub(_player->getPosition(), enemy->getPosition());
	if (diff.x < 0)
	{
		enemy->runAction(actionTo2);
	}
	if (diff.x > 0)
	{
		enemy->runAction(actionTo1);
	}
	
	float actualDuration = 0.3f;
	const Vec2& playPos = _player->getPosition();
	const Vec2& enemyPos = enemy->getPosition();
	const Vec2& position = (playPos - enemyPos).getNormalized() * 2;
	auto actionmove = MoveBy::create(actualDuration, position);
	auto actionMoveDone = CallFuncN::create(CC_CALLBACK_1(HelloWorld::enemyMoveFinished, this));
	enemy->runAction(Sequence::create(actionmove, actionMoveDone, NULL));
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

//--------------------------
	 touchLocation = touch->getLocation();
	touchLocation = this->convertToNodeSpace(touchLocation);

	auto projectile = Sprite::create("bullet.png");
	projectile->setPosition(_player->getPosition());
	projectile->setScale(0.25);
	this->addChild(projectile);

	int realX;

	auto diff2 = touchLocation - _player->getPosition();
	if (diff.x > 0)
	{
		realX = (_tileMap->getMapSize().width * _tileMap->getTileSize().width) + (projectile->getContentSize().width / 2);
	}
	else
	{
		realX = (_tileMap->getMapSize().width * _tileMap->getTileSize().width) - (projectile->getContentSize().width / 2);
	}
	float ratio = (float)diff2.y / (float)diff.x;
	int realY = ((realX - projectile->getPosition().x) * ratio) + projectile->getPosition().y;
	auto realDest = Point(realX, realY);
	
	int offRealX = realX - projectile->getPosition().x;
	int offRealY = realY - projectile->getPosition().y;
	float length = sqrtf((offRealX * offRealX) + (offRealY * offRealY));
	float velocity = 480 / 1;// 480pixels/1sec
	float realMoveDuration = length / velocity;

	auto actionMoveDone = CallFuncN::create(CC_CALLBACK_1(HelloWorld::projectileMoveFinished, this));
	projectile->runAction(Sequence::create(MoveTo::create(realMoveDuration, realDest), actionMoveDone, NULL));

	_projectiles.pushBack(projectile);
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
				SimpleAudioEngine::getInstance()->playEffect("error.mp3");
				return;
			}
		}
		SimpleAudioEngine::getInstance()->playEffect("step.mp3");
		auto collectable = propertis["Collectable"].asString();
		if ("true" == collectable)
		{
			_blockage->removeTileAt(tileCoord);
			this->_numCollected++;
			this->_hud->numCollectedChanged(_numCollected);
			SimpleAudioEngine::getInstance()->playEffect("item.mp3");
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

void HelloWorld::projectileMoveFinished(cocos2d::Object* pSender)
{
	Sprite* sprite = (Sprite*)pSender;
	this->removeChild(sprite);
	_projectiles.eraseObject(sprite);
}

#define DEF_VALUE 10
void HelloWorld::testCollisions(float dt)
{
	Vector<cocos2d::Sprite*> projectilesToDelete;
	for (cocos2d::Sprite* projectile : _projectiles)
	{
		auto projectileRect = Rect(projectile->getPositionX() * DEF_VALUE - projectile->getContentSize().width / 2,
			projectile->getPositionY() * DEF_VALUE - projectile->getContentSize().height / 4,
												projectile->getContentSize().width,
												projectile->getContentSize().height);

	
		Vector<cocos2d::Sprite*> targetsToDelete;
		for (cocos2d::Sprite* target : _enemies)
		{
			auto targetRect = Rect(target->getPositionX() *DEF_VALUE - target->getContentSize().width / 2,
												target->getPositionY() *DEF_VALUE- target->getContentSize().height / 2,
												target->getContentSize().width,
												target->getContentSize().height);

			if (projectileRect.intersectsRect(targetRect))
			{
				targetsToDelete.pushBack(target);
			}
		}

		for (cocos2d::Sprite* target : targetsToDelete)
		{
			_enemies.eraseObject(target);
			this->removeChild(target);
		}

		if (targetsToDelete.size() > 0)
		{
			projectilesToDelete.pushBack(projectile);
		}
		targetsToDelete.clear();
	}
}

bool HelloWorld::HelloWorldHud::init()
{
	if (!Layer::init())
	{
		return false;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	label = LabelTTF::create("0", "微软雅黑", 18.0f, Size(50, 20), TextHAlignment::RIGHT);
	label->setColor(Color3B(255, 0, 0));
	int margin = 15;
	//label->setPosition(visibleSize.width - (label->getDimensions().width / 2) - margin, label->getDimensions().height / 2 + margin);
	label->setPosition(0, visibleSize.height - margin);
	this->addChild(label);
	return true;
}

void HelloWorld::HelloWorldHud::numCollectedChanged(int numCollected)
{
	char showStr[20] = { 0 };
	sprintf(showStr, "%d", numCollected);
	label->setString(showStr);
}

