#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>

using namespace geode::prelude;

std::string getThumbnailUrl(int levelID) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    
    return "https://raw.githubusercontent.com/maksimdodep/fenixgdps-thumbnails/main/thumbnails/" 
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

        web::downloadFile(getThumbnailUrl(levelID), cachePath, [this, cachePath](bool success) {
            if (!success) return;
            auto cacheStr = cachePath.string();
            if (auto texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(cacheStr.c_str(), "")) {
                if (m_fields->m_thumbnailSprite) {
                    m_fields->m_thumbnailSprite->removeFromParent();
                }
                m_fields->m_thumbnailSprite = CCSprite::createWithTexture(texture);
                m_fields->m_thumbnailSprite->setPosition({ m_width - 60.f, m_height / 2.f });
                m_fields->m_thumbnailSprite->setScale(0.4f);
                this->addChild(m_fields->m_thumbnailSprite);
            }
        });
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

        web::downloadFile(getThumbnailUrl(levelID), cachePath, [this, cachePath](auto progress) {
            if (progress.isFinished() && !progress.isCancelled()) {
                auto cacheStr = cachePath.string();
                if (auto texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(cacheStr.c_str(), "")) {
                    if (m_fields->m_bgSprite) {
                        m_fields->m_bgSprite->removeFromParent();
                    }
                    m_fields->m_bgSprite = CCSprite::createWithTexture(texture);
                    
                    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
                    m_fields->m_bgSprite->setPosition({ winSize.width / 2.f, winSize.height / 2.f });
                    m_fields->m_bgSprite->setScale(1.2f);
                    this->addChild(m_fields->m_bgSprite, -1);
                }
            }
        });

        return true;
    }
};
