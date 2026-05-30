#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCell.hpp>
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
        auto cachePath = Mod::get()->getSavedLoopDirectory() / fmt::format("thumb_{}.png", levelID);

        web::AsyncWebRequest()
            .fetch(getThumbnailUrl(levelID))
            .into(cachePath)
            .then([this, cachePath](auto) {
                if (auto texture = CCTextureCache::sharedTextureCache()->addImage(cachePath.string().c_str())) {
                    if (m_fields->m_thumbnailSprite) {
                        m_fields->m_thumbnailSprite->removeFromParent();
                    }
                    m_fields->m_thumbnailSprite = CCSprite::createWithTexture(texture);
                    m_fields->m_thumbnailSprite->setPosition({ m_width - 60.f, m_height / 2.f });
                    m_fields->m_thumbnailSprite->setScale(0.4f);
                    this->addChild(m_fields->m_thumbnailSprite);
                }
            })
            .expect([](std::string const&) {});
    }
};
