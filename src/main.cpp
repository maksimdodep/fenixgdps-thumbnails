#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>

using namespace geode::prelude;

std::string getThumbnailUrl(int levelID) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    
    return "https://githubusercontent.com" 
           + std::to_string(levelID) 
           + ".png?t=" 
           + std::to_string(millis);
}

class $modify(MyLevelCell, LevelCell) {
    struct Fields {
        cocos2d::CCSprite* m_thumbnailSprite = nullptr;
    };

    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (!m_level) return;

        int levelID = m_level->m_levelID.value();
        auto cachePath = Mod::get()->getSaveDir() / fmt::format("thumb_{}.png", levelID);

        web::FetchRequest()
            .fetch(getThumbnailUrl(levelID))
            .into(cachePath)
            .then([this, cachePath](auto) {
                auto cacheStr = cachePath.string();
                cocos2d::CCTexture2D* texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(cacheStr.c_str(), "");
                if (texture) {
                    if (m_fields->m_thumbnailSprite) {
                        m_fields->m_thumbnailSprite->removeFromParent();
                    }
                    m_fields->m_thumbnailSprite = cocos2d::CCSprite::createWithTexture(texture);
                    m_fields->m_thumbnailSprite->setPosition({ m_width - 60.f, m_height / 2.f });
                    m_fields->m_thumbnailSprite->setScale(0.4f);
                    this->addChild(m_fields->m_thumbnailSprite);
                }
            })
            .expect([](std::string const&) {});
    }
};

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        cocos2d::CCSprite* m_bgSprite = nullptr;
    };

    bool init(GJGameLevel* level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        int levelID = level->m_levelID.value();
        auto cachePath = Mod::get()->getSaveDir() / fmt::format("thumb_{}.png", levelID);

        web::FetchRequest()
            .fetch(getThumbnailUrl(levelID))
            .into(cachePath)
            .then([this, cachePath, texture](auto) {
                auto cacheStr = cachePath.string();
                cocos2d::CCTexture2D* texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(cacheStr.c_str(), "");
                if (texture) {
                    if (m_fields->m_bgSprite) {
                        m_fields->m_bgSprite->removeFromParent();
                    }
                    m_fields->m_bgSprite = cocos2d::CCSprite::createWithTexture(texture);
                    m_fields->m_bgSprite->setPosition({ 284.5f, 160.f });
                    m_fields->m_bgSprite->setScale(1.2f);
                    this->addChild(m_fields->m_bgSprite, -1);
                    
                    this->scheduleUpdate();
                }
            })
            .expect([](std::string const&) {});

        return true;
    }

    void update(float dt) {
        LevelInfoLayer::update(dt);

        if (!m_fields->m_bgSprite) return;

        auto mousePos = cocos2d::CCDirector::sharedDirector()->getOpenGLView()->getMousePosition();
        cocos2d::CCPoint cocosMousePos = cocos2d::CCPoint(mousePos.x, mousePos.y);
        auto realPos = this->convertToNodeSpace(cocosMousePos);

        float targetX = 284.5f - (realPos.x - 284.5f) * 0.05f;
        float targetY = 160.f - (realPos.y - 160.f) * 0.05f;

        float currentX = m_fields->m_bgSprite->getPositionX();
        float currentY = m_fields->m_bgSprite->getPositionY();

        m_fields->m_bgSprite->setPositionX(currentX + (targetX - currentX) * 0.1f);
        m_fields->m_bgSprite->setPositionY(currentY + (targetY - currentY) * 0.1f);
    }
};
